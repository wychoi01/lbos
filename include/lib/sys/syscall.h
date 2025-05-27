#ifndef SYSCALL_H
#define SYSCALL_H

#define SYS_PRINTF 1
#define SYS_FORK 2
#define SYS_EXIT 3
#define SYS_WAIT 4

int printf(const char* format);
int fork(void);
void exit(int status);
int wait(int* status);

#endif /* SYSCALL_H */
