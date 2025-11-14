#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAXLINE 4096

int main(void)
{
    char *line = NULL;
    size_t len = 0;
    printf("mini-shell. Type 'exit' to quit.\n");
    while (1)
    {
        printf(">> ");
        fflush(stdout);
        ssize_t nread = getline(&line, &len, stdin);
        if (nread == -1)
        {
            printf("\n");
            break;
        }
        if (nread > 0 && line[nread - 1] == '\n')
            line[nread - 1] = '\0';
        if (strcmp(line, "exit") == 0)
            break;
        if (line[0] == '\0')
            continue;
        char *saveptr;
        int argc = 0;
        char *argv[256];
        char *token = strtok_r(line, " \t", &saveptr);
        while (token && argc < 255)
        {
            argv[argc++] = token;
            token = strtok_r(NULL, " \t", &saveptr);
        }
        argv[argc] = NULL;
        if (argc == 0)
            continue;

        pid_t pid = fork();
        if (pid < 0)
        {
            perror("fork");
            continue;
        }
        else if (pid == 0)
        {
            execvp(argv[0], argv);
            perror("execvp");
            exit(127);
        }
        else
        {
            int status;
            waitpid(pid, &status, 0);
            if (WIFEXITED(status))
            {
                printf("Child exited with %d\n", WEXITSTATUS(status));
            }
            else if (WIFSIGNALED(status))
            {
                printf("Child killed by signal %d\n", WTERMSIG(status));
            }
        }
    }
    free(line);
    printf("mini-shell exiting.\n");
    return 0;
}
