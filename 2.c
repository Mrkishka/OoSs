#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

void spawn_and_wait_children(int count, const char *label_prefix)
{
    pid_t p;
    for (int i = 1; i <= count; ++i)
    {
        p = fork();
        if (p < 0)
        {
            perror("fork");
            exit(1);
        }
        if (p == 0)
        {
            printf("%s%d: pid=%d ppid=%d\n", label_prefix, i, getpid(), getppid());
            return;
        }
    }
    int status;
    pid_t w;
    for (int i = 0; i < count; ++i)
    {
        w = wait(&status);
        if (w > 0)
        {
            if (WIFEXITED(status))
                printf("Parent of %s waited: child %d exited %d\n", label_prefix, w, WEXITSTATUS(status));
            else
                printf("Parent of %s waited: child %d status 0x%x\n", label_prefix, w, status);
        }
    }
}

int main(void)
{
    pid_t p1 = fork();
    if (p1 < 0)
    {
        perror("fork");
        return 1;
    }
    if (p1 == 0)
    {
        printf("process1: pid=%d ppid=%d\n", getpid(), getppid());
        spawn_and_wait_children(2, "process3/4_");
        exit(0);
    }

    pid_t p2 = fork();
    if (p2 < 0)
    {
        perror("fork");
        return 1;
    }
    if (p2 == 0)
    {
        printf("process2: pid=%d ppid=%d\n", getpid(), getppid());
        pid_t p5 = fork();
        if (p5 < 0)
        {
            perror("fork");
            exit(1);
        }
        if (p5 == 0)
        {
            printf("process5: pid=%d ppid=%d\n", getpid(), getppid());
            exit(0);
        }
        else
        {
            int st;
            waitpid(p5, &st, 0);
            printf("process2: child process5 %d finished\n", p5);
            exit(0);
        }
    }

    printf("root: pid=%d ppid=%d (spawned p1=%d p2=%d)\n", getpid(), getppid(), p1, p2);
    int status;
    waitpid(p1, &status, 0);
    printf("root: process1 %d finished\n", p1);
    waitpid(p2, &status, 0);
    printf("root: process2 %d finished\n", p2);

    return 0;
}
