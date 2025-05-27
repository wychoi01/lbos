#include <lib/sys/syscall.h>

static int syscall(int num, int arg1, int arg2, int arg3, int arg4, int arg5) {
    int ret;
    __asm__ volatile(
        "movl %1, %%eax\n"    /* syscall number */
        "movl %2, %%ebx\n"    /* arg1 */
        "movl %3, %%ecx\n"    /* arg2 */
        "movl %4, %%edx\n"    /* arg3 */
        "movl %5, %%esi\n"    /* arg4 */
        "movl %6, %%edi\n"    /* arg5 */
        "int $0x80\n"         /* trigger interrupt */
        "movl %%eax, %0"      /* store return value */
        : "=m"(ret)           /* output: store in memory */
        : "m"(num),           /* input: syscall number */
          "m"(arg1),          /* input: arg1 */
          "m"(arg2),          /* input: arg2 */
          "m"(arg3),          /* input: arg3 */
          "m"(arg4),          /* input: arg4 */
          "m"(arg5)           /* input: arg5 */
        : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
    );
    return ret;
}

int printf(const char* format) {
    return syscall(SYS_PRINTF, (int)format, 0, 0, 0, 0);
}

int fork(void) {
    return syscall(SYS_FORK, 0, 0, 0, 0, 0);
}

void exit(int status) {
    syscall(SYS_EXIT, status, 0, 0, 0, 0);
}

int wait(int* status) {
    return syscall(SYS_WAIT, (int)status, 0, 0, 0, 0);
}
