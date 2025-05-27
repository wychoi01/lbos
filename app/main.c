#include <lib/sys/syscall.h>

int main() {
    printf("Starting main process");

    int pid = fork();
    int flag = 0;

    if (pid < 0) {
        printf("Fork failed");
    } else if (pid == 0) {
        printf("Hello from child process");
        exit(42);
    } else {
        printf("Hello from parent process");

        int status;
        int child_pid = wait(&status);

        if (child_pid > 0) {
            printf("Child process exited");
        } else {
            printf("Wait failed");
        }
    }

    printf("Main process exiting");
    while(1) {}

    return 0;
}
