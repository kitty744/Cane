# Device Drivers

The Device Driver system provides a unified interface for hardware communication and device management in CaneOS.

## Overview

CaneOS uses a modular driver architecture that separates hardware-specific code from the kernel core. Drivers register with the device manager and provide standardized interfaces for common operations like initialization, I/O, and power management.

## Quick Start

```c
#include "cane/driver.h"

// Simple driver registration
int my_driver_init(void) {
    struct device_driver *driver = create_driver("my_device", DRIVER_TYPE_CHAR);
    if (!driver) return -ENOMEM;

    driver->probe = my_driver_probe;
    driver->remove = my_driver_remove;
    driver->read = my_driver_read;
    driver->write = my_driver_write;

    return register_driver(driver);
}

// Device-specific operations
int my_driver_probe(struct device *dev) {
    // Initialize hardware
    init_hardware_registers(dev);

    // Allocate resources
    dev->private_data = kmalloc(sizeof(struct my_device_data));

    printf("My device detected at %p\n", dev);
    return 0;
}
```

## Driver Architecture

### Driver Registration

```c
struct device_driver {
    const char *name;
    enum driver_type type;
    enum device_class class;

    // Driver lifecycle
    int (*probe)(struct device *dev);
    int (*remove)(struct device *dev);
    int (*suspend)(struct device *dev);
    int (*resume)(struct device *dev);

    // I/O operations
    ssize_t (*read)(struct device *dev, void *buf, size_t count, loff_t pos);
    ssize_t (*write)(struct device *dev, const void *buf, size_t count, loff_t pos);
    int (*ioctl)(struct device *dev, unsigned int cmd, unsigned long arg);

    // Interrupt handling
    irqreturn_t (*irq_handler)(int irq, void *dev_id);

    // Driver-specific data
    void *private_data;
    struct list_head list;
};

int register_driver(struct device_driver *driver);
void unregister_driver(struct device_driver *driver);
```

### Device Management

```c
struct device {
    const char *name;
    enum device_type type;
    enum device_class class;

    // Hardware resources
    struct resource *resources;
    int irq;
    void __iomem *mmio_base;

    // Driver association
    struct device_driver *driver;
    void *private_data;

    // Device state
    enum device_state state;
    atomic_t refcount;

    // Linked lists
    struct list_head list;
    struct list_head driver_list;
};

int register_device(struct device *dev);
void unregister_device(struct device *dev);
struct device *find_device(const char *name);
```

## Common Driver Types

### Character Drivers

Character drivers provide byte-stream access to devices:

```c
struct char_device {
    struct device base;
    struct cdev cdev;
    dev_t dev_number;

    // Device-specific operations
    const struct file_operations *fops;
};

// Example: Simple character driver
static const struct file_operations my_char_fops = {
    .owner = THIS_MODULE,
    .read = my_char_read,
    .write = my_char_write,
    .open = my_char_open,
    .release = my_char_release,
    .ioctl = my_char_ioctl,
};

int my_char_driver_init(void) {
    int ret;
    dev_t dev_num;

    // Allocate device number
    ret = alloc_chrdev_region(&dev_num, 0, 1, "my_char");
    if (ret < 0) return ret;

    // Initialize character device
    cdev_init(&my_char_dev.cdev, &my_char_fops);
    my_char_dev.cdev.owner = THIS_MODULE;

    // Add character device
    ret = cdev_add(&my_char_dev.cdev, dev_num, 1);
    if (ret < 0) {
        unregister_chrdev_region(dev_num, 1);
        return ret;
    }

    my_char_dev.dev_number = dev_num;
    return 0;
}
```

### Block Drivers

Block drivers provide block-oriented access to storage devices:

```c
struct block_device {
    struct device base;
    struct gendisk *disk;
    struct request_queue *queue;

    // Device properties
    sector_t capacity;
    int block_size;
    bool removable;

    // Operations
    const struct block_device_operations *ops;
};

// Example: Simple block driver
static const struct block_device_operations my_block_ops = {
    .owner = THIS_MODULE,
    .open = my_block_open,
    .release = my_block_release,
    .ioctl = my_block_ioctl,
    .getgeo = my_block_getgeo,
};

int my_block_driver_init(void) {
    struct request_queue *queue;
    int ret;

    // Allocate request queue
    queue = blk_init_queue(my_block_request, &my_lock);
    if (!queue) return -ENOMEM;

    // Allocate disk
    my_block_dev.disk = alloc_disk(1);
    if (!my_block_dev.disk) {
        blk_cleanup_queue(queue);
        return -ENOMEM;
    }

    // Set up disk properties
    my_block_dev.disk->queue = queue;
    my_block_dev.disk->major = MY_BLOCK_MAJOR;
    my_block_dev.disk->first_minor = 0;
    my_block_dev.disk->fops = &my_block_ops;
    my_block_dev.disk->private_data = &my_block_dev;

    sprintf(my_block_dev.disk->disk_name, "myblock");
    set_capacity(my_block_dev.disk, MY_BLOCK_SIZE / 512);

    // Add disk
    add_disk(my_block_dev.disk);

    return 0;
}
```

### Network Drivers

Network drivers provide network interface functionality:

```c
struct net_device {
    struct device base;
    struct net_device *netdev;

    // Network properties
    unsigned char mac_addr[6];
    int mtu;
    unsigned int flags;

    // Statistics
    struct net_device_stats stats;

    // Operations
    const struct net_device_ops *netdev_ops;
};

// Example: Simple network driver
static const struct net_device_ops my_net_ops = {
    .ndo_open = my_net_open,
    .ndo_stop = my_net_stop,
    .ndo_start_xmit = my_net_xmit,
    .ndo_get_stats = my_net_get_stats,
    .ndo_set_mac_address = my_net_set_mac,
};

int my_net_driver_init(void) {
    struct net_device *netdev;
    int ret;

    // Allocate network device
    netdev = alloc_etherdev(sizeof(struct my_net_priv));
    if (!netdev) return -ENOMEM;

    // Set up network device
    netdev->netdev_ops = &my_net_ops;
    netdev->mtu = 1500;
    netdev->flags = IFF_BROADCAST | IFF_MULTICAST;

    // Set MAC address
    eth_hw_addr_random(netdev);

    // Register network device
    ret = register_netdev(netdev);
    if (ret) {
        free_netdev(netdev);
        return ret;
    }

    my_net_dev.netdev = netdev;
    return 0;
}
```

## Hardware Resource Management

### Memory-Mapped I/O

```c
// Map device registers
void __iomem *map_device_registers(phys_addr_t phys_addr, size_t size) {
    void __iomem *mmio;

    mmio = ioremap(phys_addr, size);
    if (!mmio) {
        printk(KERN_ERR "Failed to map device registers\n");
        return NULL;
    }

    return mmio;
}

// Access mapped registers
uint32_t read_device_register(void __iomem *base, unsigned int offset) {
    return ioread32(base + offset);
}

void write_device_register(void __iomem *base, unsigned int offset, uint32_t value) {
    iowrite32(value, base + offset);
}

// Cleanup
void unmap_device_registers(void __iomem *mmio, size_t size) {
    iounmap(mmio);
}
```

### Interrupt Handling

```c
// Register interrupt handler
int register_device_irq(int irq, irq_handler_t handler, void *dev_id) {
    int ret;

    ret = request_irq(irq, handler, IRQF_SHARED, "my_device", dev_id);
    if (ret) {
        printk(KERN_ERR "Failed to register IRQ %d\n", irq);
        return ret;
    }

    return 0;
}

// Interrupt handler example
irqreturn_t my_device_irq_handler(int irq, void *dev_id) {
    struct my_device *dev = dev_id;
    uint32_t status;

    // Read interrupt status
    status = read_device_register(dev->mmio_base, REG_STATUS);
    if (!(status & IRQ_PENDING)) {
        return IRQ_NONE;  // Not our interrupt
    }

    // Handle interrupt
    if (status & RX_READY) {
        handle_rx_interrupt(dev);
    }

    if (status & TX_COMPLETE) {
        handle_tx_complete(dev);
    }

    // Clear interrupt
    write_device_register(dev->mmio_base, REG_STATUS, status);

    return IRQ_HANDLED;
}

// Cleanup
void unregister_device_irq(int irq, void *dev_id) {
    free_irq(irq, dev_id);
}
```

### DMA Operations

```c
// Allocate DMA buffer
void *alloc_dma_buffer(size_t size, dma_addr_t *dma_handle) {
    void *buffer;

    buffer = dma_alloc_coherent(NULL, size, dma_handle, GFP_KERNEL);
    if (!buffer) {
        printk(KERN_ERR "Failed to allocate DMA buffer\n");
        return NULL;
    }

    return buffer;
}

// Setup DMA transfer
int setup_dma_transfer(struct my_device *dev, void *buffer, size_t size) {
    dma_addr_t dma_addr;

    // Get physical address for DMA
    dma_addr = dma_map_single(&dev->base.dev, buffer, size, DMA_TO_DEVICE);
    if (dma_mapping_error(&dev->base.dev, dma_addr)) {
        return -EIO;
    }

    // Program DMA controller
    write_device_register(dev->mmio_base, REG_DMA_ADDR, dma_addr);
    write_device_register(dev->mmio_base, REG_DMA_SIZE, size);
    write_device_register(dev->mmio_base, REG_DMA_CTRL, DMA_START);

    return 0;
}

// Cleanup DMA
void cleanup_dma_transfer(struct my_device *dev, void *buffer, size_t size) {
    dma_unmap_single(&dev->base.dev, dma_addr, size, DMA_TO_DEVICE);
}
```

## Device Discovery and Enumeration

### PCI Device Discovery

```c
// PCI driver structure
static struct pci_driver my_pci_driver = {
    .name = "my_pci_device",
    .id_table = my_pci_ids,
    .probe = my_pci_probe,
    .remove = my_pci_remove,
};

// PCI device IDs
static const struct pci_device_id my_pci_ids[] = {
    { PCI_DEVICE(0x1234, 0x5678) },
    { 0, }
};
MODULE_DEVICE_TABLE(pci, my_pci_ids);

// PCI probe function
static int my_pci_probe(struct pci_dev *pdev, const struct pci_device_id *id) {
    struct my_device *dev;
    int ret;

    // Allocate device structure
    dev = kzalloc(sizeof(struct my_device), GFP_KERNEL);
    if (!dev) return -ENOMEM;

    // Enable PCI device
    ret = pci_enable_device(pdev);
    if (ret) goto err_free;

    // Request PCI regions
    ret = pci_request_regions(pdev, "my_pci_device");
    if (ret) goto err_disable;

    // Map BAR 0 (memory-mapped registers)
    dev->mmio_base = pci_iomap(pdev, 0, 0);
    if (!dev->mmio_base) {
        ret = -EIO;
        goto err_release;
    }

    // Set up DMA mask
    ret = dma_set_mask_and_coherent(&pdev->dev, DMA_BIT_MASK(32));
    if (ret) goto err_unmap;

    // Get IRQ
    dev->irq = pdev->irq;
    ret = request_irq(dev->irq, my_irq_handler, IRQF_SHARED,
                     "my_pci_device", dev);
    if (ret) goto err_unmap;

    // Store device reference
    pci_set_drvdata(pdev, dev);
    dev->pdev = pdev;

    // Initialize device
    ret = init_my_device(dev);
    if (ret) goto err_irq;

    printk(KERN_INFO "My PCI device initialized\n");
    return 0;

err_irq:
    free_irq(dev->irq, dev);
err_unmap:
    pci_iounmap(pdev, dev->mmio_base);
err_release:
    pci_release_regions(pdev);
err_disable:
    pci_disable_device(pdev);
err_free:
    kfree(dev);
    return ret;
}
```

### Platform Device Discovery

```c
// Platform driver structure
static struct platform_driver my_platform_driver = {
    .driver = {
        .name = "my_platform_device",
        .of_match_table = my_of_match,
    },
    .probe = my_platform_probe,
    .remove = my_platform_remove,
};

// Device tree match table
static const struct of_device_id my_of_match[] = {
    { .compatible = "vendor,my-device", },
    { }
};
MODULE_DEVICE_TABLE(of, my_of_match);

// Platform probe function
static int my_platform_probe(struct platform_device *pdev) {
    struct resource *res;
    struct my_device *dev;
    int ret;

    dev = devm_kzalloc(&pdev->dev, sizeof(*dev), GFP_KERNEL);
    if (!dev) return -ENOMEM;

    // Get memory resource
    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if (!res) return -ENODEV;

    dev->mmio_base = devm_ioremap_resource(&pdev->dev, res);
    if (IS_ERR(dev->mmio_base))
        return PTR_ERR(dev->mmio_base);

    // Get IRQ
    dev->irq = platform_get_irq(pdev, 0);
    if (dev->irq < 0) return dev->irq;

    ret = devm_request_irq(&pdev->dev, dev->irq, my_irq_handler,
                          IRQF_SHARED, "my_platform_device", dev);
    if (ret) return ret;

    // Store device reference
    platform_set_drvdata(pdev, dev);

    return init_my_device(dev);
}
```

## Driver Development Best Practices

### Error Handling

```c
int robust_device_init(struct my_device *dev) {
    int ret;

    // Initialize hardware step by step
    ret = init_clocks(dev);
    if (ret) {
        dev_err(&dev->base.dev, "Clock initialization failed: %d\n", ret);
        return ret;
    }

    ret = reset_device(dev);
    if (ret) {
        dev_err(&dev->base.dev, "Device reset failed: %d\n", ret);
        goto err_clocks;
    }

    ret = setup_interrupts(dev);
    if (ret) {
        dev_err(&dev->base.dev, "Interrupt setup failed: %d\n", ret);
        goto err_reset;
    }

    ret = configure_device(dev);
    if (ret) {
        dev_err(&dev->base.dev, "Device configuration failed: %d\n", ret);
        goto err_interrupts;
    }

    return 0;

err_interrupts:
    cleanup_interrupts(dev);
err_reset:
    unreset_device(dev);
err_clocks:
    cleanup_clocks(dev);
    return ret;
}
```

### Power Management

```c
// Suspend function
int my_device_suspend(struct device *dev) {
    struct my_device *my_dev = dev_get_drvdata(dev);

    // Stop ongoing operations
    stop_device_operations(my_dev);

    // Save device state
    save_device_context(my_dev);

    // Put device in low power state
    set_power_state(my_dev, POWER_STATE_SUSPEND);

    return 0;
}

// Resume function
int my_device_resume(struct device *dev) {
    struct my_device *my_dev = dev_get_drvdata(dev);

    // Restore power
    set_power_state(my_dev, POWER_STATE_ON);

    // Restore device state
    restore_device_context(my_dev);

    // Resume operations
    start_device_operations(my_dev);

    return 0;
}

// Power management operations
static const struct dev_pm_ops my_pm_ops = {
    .suspend = my_device_suspend,
    .resume = my_device_resume,
    .runtime_suspend = my_device_runtime_suspend,
    .runtime_resume = my_device_runtime_resume,
};
```

## Integration Example

```c
#include "cane/driver.h"

// Complete driver initialization
static int __init my_driver_init(void) {
    int ret;

    // Register character driver
    ret = register_chrdev_region(MY_DEV_MAJOR, 1, "my_device");
    if (ret) {
        printk(KERN_ERR "Failed to register device major\n");
        return ret;
    }

    // Register platform driver
    ret = platform_driver_register(&my_platform_driver);
    if (ret) {
        printk(KERN_ERR "Failed to register platform driver\n");
        goto err_unregister_chrdev;
    }

    // Create sysfs entries
    ret = create_sysfs_entries();
    if (ret) {
        printk(KERN_ERR "Failed to create sysfs entries\n");
        goto err_unregister_platform;
    }

    printk(KERN_INFO "My driver loaded successfully\n");
    return 0;

err_unregister_platform:
    platform_driver_unregister(&my_platform_driver);
err_unregister_chrdev:
    unregister_chrdev_region(MY_DEV_MAJOR, 1);
    return ret;
}

static void __exit my_driver_exit(void) {
    remove_sysfs_entries();
    platform_driver_unregister(&my_platform_driver);
    unregister_chrdev_region(MY_DEV_MAJOR, 1);

    printk(KERN_INFO "My driver unloaded\n");
}

module_init(my_driver_init);
module_exit(my_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("CaneOS Team");
MODULE_DESCRIPTION("Example device driver for CaneOS");
```

This driver system provides a comprehensive foundation for hardware device support with proper resource management, error handling, and integration with the broader kernel infrastructure.
