#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define SHM_SIZE 256

#define SEM_PARENT 0
#define SEM_CHILD  1

void sem_op(int semid, int semnum, int op) {
    struct sembuf sb = { semnum, op, 0 };
    if (semop(semid, &sb, 1) < 0) {
        perror("semop");
        exit(1);
    }
}

int main() {
    key_t key = ftok("chat.c", 1);

    int shmid = shmget(key, SHM_SIZE, IPC_CREAT | 0666);
    if (shmid < 0) { perror("shmget"); exit(1); }

    char *shm = (char*)shmat(shmid, NULL, 0);
    if (shm == (char*)-1) { perror("shmat"); exit(1); }

    int semid = semget(key, 2, IPC_CREAT | 0666);
    if (semid < 0) { perror("semget"); exit(1); }

    semctl(semid, SEM_PARENT, SETVAL, 1);
    semctl(semid, SEM_CHILD,  SETVAL, 0);

    pid_t pid = fork();

    // Дитя
    if (pid == 0) {
        while (1) {
            sem_op(semid, SEM_CHILD, -1);

            printf("Родитель: %s\n", shm);
            fflush(stdout);

            printf("Дочерний > ");
            fgets(shm, SHM_SIZE, stdin);

            sem_op(semid, SEM_PARENT, +1);
        }
        exit(0);
    }

    // Родител
    while (1) {
        sem_op(semid, SEM_PARENT, -1);

        printf("Родитель > ");
        fgets(shm, SHM_SIZE, stdin);

        sem_op(semid, SEM_CHILD, +1);

        sem_op(semid, SEM_PARENT, -1);
        printf("Дочерний: %s\n", shm);
        sem_op(semid, SEM_PARENT, +1);
    }

    return 0;
}
