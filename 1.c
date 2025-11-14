#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    pid_t pid = fork();
    if (pid < 0)
    {
        perror("fork");
        return 1;
    }
    else if (pid == 0)
    {
        printf("Child: pid=%d ppid=%d\n", getpid(), getppid());
        exit(42);
    }
    else
    {
        printf("Parent: pid=%d ppid=%d child_pid=%d\n", getpid(), getppid(), pid);
        int status;
        pid_t w = waitpid(pid, &status, 0);
        if (w == -1)
        {
            perror("waitpid");
            return 1;
        }
        if (WIFEXITED(status))
        {
            printf("Child %d exited with code %d\n", pid, WEXITSTATUS(status));
        }
        else if (WIFSIGNALED(status))
        {
            printf("Child %d killed by signal %d\n", pid, WTERMSIG(status));
        }
        else
        {
            printf("Child %d ended with status 0x%x\n", pid, status);
        }
    }
    return 0;
}
