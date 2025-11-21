#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <sys/types.h>

#define MSGSZ 256

struct msgbuf
{
    long mtype;
    char mtext[MSGSZ];
};

int main()
{
    key_t key = ftok("chat_msgqueue.c", 1);
    int msgid = msgget(key, IPC_CREAT | 0666);
    if (msgid < 0)
    {
        perror("msgget");
        exit(1);
    }

    pid_t pid = fork();

    if (pid < 0)
    {
        perror("fork");
        exit(1);
    }

    // ---------------------------------------------------
    // Чилд — рид mtype=1 и сенд mtype=2
    // ---------------------------------------------------
    if (pid == 0)
    {
        struct msgbuf message;

        while (1)
        {
            // рид фром парент
            if (msgrcv(msgid, &message, MSGSZ, 1, 0) < 0)
            {
                perror("child msgrcv");
                exit(1);
            }

            printf("[Родитель → Дочерний]: %s\n", message.mtext);

            // инпут ансвер
            printf("Дочерний ввод: ");
            fflush(stdout);
            fgets(message.mtext, MSGSZ, stdin);

            message.mtype = 2; // сенд ту парент

            if (msgsnd(msgid, &message, strlen(message.mtext) + 1, 0) < 0)
            {
                perror("child msgsnd");
                exit(1);
            }
        }
    }

    // ---------------------------------------------------
    // Парент — Рид mtype=2 и сенд mtype=1
    // ---------------------------------------------------
    struct msgbuf message;

    while (1)
    {
        // инпут фром парент
        printf("Родитель ввод: ");
        fflush(stdout);
        fgets(message.mtext, MSGSZ, stdin);

        message.mtype = 1; // сенд ту чилд

        if (msgsnd(msgid, &message, strlen(message.mtext) + 1, 0) < 0)
        {
            perror("parent msgsnd");
            exit(1);
        }

        // рид
        if (msgrcv(msgid, &message, MSGSZ, 2, 0) < 0)
        {
            perror("parent msgrcv");
            exit(1);
        }

        printf("[Дочерний → Родитель]: %s\n", message.mtext);
    }

    return 0;
}
