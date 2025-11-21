// gcc -std=c11 -Wall -O2 chat_ipc.c -o chat_ipc
//  ./chat_ipc

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#define BUF_SIZE 1024

int main(void)
{
    int p2c[2];
    int c2p[2];

    if (pipe(p2c) == -1)
    {
        perror("pipe p2c");
        exit(EXIT_FAILURE);
    }
    if (pipe(c2p) == -1)
    {
        perror("pipe c2p");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    if (pid == -1)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0)
    {
        // Чилдрёныш
        close(p2c[1]);
        close(c2p[0]);

        int read_fd = p2c[0];
        int write_fd = c2p[1];
        char buf[BUF_SIZE];
        fd_set rfds;
        printf("[child] started. Type messages to send to parent. Type 'exit' to quit.\n");

        while (1)
        {
            FD_ZERO(&rfds);
            FD_SET(STDIN_FILENO, &rfds);
            FD_SET(read_fd, &rfds);
            int maxfd = (STDIN_FILENO > read_fd) ? STDIN_FILENO : read_fd;

            int sel = select(maxfd + 1, &rfds, NULL, NULL, NULL);
            if (sel == -1)
            {
                if (errno == EINTR)
                    continue;
                perror("select child");
                break;
            }

            if (FD_ISSET(STDIN_FILENO, &rfds))
            {
                if (!fgets(buf, sizeof(buf), stdin))
                {
                    close(write_fd);
                    break;
                }
                size_t len = strlen(buf);
                if (len > 0 && buf[len - 1] == '\n')
                    buf[len - 1] = '\0';

                if (write(write_fd, buf, strlen(buf)) == -1)
                {
                    perror("child write");
                    break;
                }
                if (write(write_fd, "\n", 1) == -1)
                {
                }

                if (strcmp(buf, "exit") == 0)
                {
                    close(write_fd);
                    break;
                }
            }

            if (FD_ISSET(read_fd, &rfds))
            {
                ssize_t r = read(read_fd, buf, sizeof(buf) - 1);
                if (r == -1)
                {
                    perror("child read");
                    break;
                }
                else if (r == 0)
                {
                    fprintf(stderr, "[child] parent closed connection. Exiting.\n");
                    break;
                }
                else
                {
                    buf[r] = '\0';
                    printf("[parent] %s", buf);
                    if (buf[r - 1] != '\n')
                        printf("\n");
                    fflush(stdout);
                }
            }
        }

        close(read_fd);
        _exit(EXIT_SUCCESS);
    }
    else
    { // Родител
        close(p2c[0]);
        close(c2p[1]);

        int write_fd = p2c[1];
        int read_fd = c2p[0];

        char buf[BUF_SIZE];
        fd_set rfds;
        printf("[parent] started. Type messages to send to child. Type 'exit' to quit.\n");

        while (1)
        {
            FD_ZERO(&rfds);
            FD_SET(STDIN_FILENO, &rfds);
            FD_SET(read_fd, &rfds);
            int maxfd = (STDIN_FILENO > read_fd) ? STDIN_FILENO : read_fd;

            int sel = select(maxfd + 1, &rfds, NULL, NULL, NULL);
            if (sel == -1)
            {
                if (errno == EINTR)
                    continue;
                perror("select parent");
                break;
            }

            if (FD_ISSET(STDIN_FILENO, &rfds))
            {
                if (!fgets(buf, sizeof(buf), stdin))
                {
                    close(write_fd);
                    break;
                }
                size_t len = strlen(buf);
                if (len > 0 && buf[len - 1] == '\n')
                    buf[len - 1] = '\0';

                if (write(write_fd, buf, strlen(buf)) == -1)
                {
                    perror("parent write");
                    break;
                }
                if (write(write_fd, "\n", 1) == -1)
                { /* ignore */
                }

                if (strcmp(buf, "exit") == 0)
                {
                    close(write_fd);
                    break;
                }
            }

            if (FD_ISSET(read_fd, &rfds))
            {
                ssize_t r = read(read_fd, buf, sizeof(buf) - 1);
                if (r == -1)
                {
                    perror("parent read");
                    break;
                }
                else if (r == 0)
                {
                    fprintf(stderr, "[parent] child closed connection. Exiting.\n");
                    break;
                }
                else
                {
                    buf[r] = '\0';
                    printf("[child] %s", buf);
                    if (buf[r - 1] != '\n')
                        printf("\n");
                    fflush(stdout);
                }
            }
        }

        int status;
        waitpid(pid, &status, 0);
        close(read_fd);
        printf("[parent] exiting.\n");
        exit(EXIT_SUCCESS);
    }
}
