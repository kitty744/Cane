#ifndef SHELL_H
#define SHELL_H

#include <stdint.h>

void shell_init(void);
void shell_input(signed char c);
void process_command(char *cmd);
void shell_task_main(void);

#endif
