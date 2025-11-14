#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define NTHREADS 5

void *thread_func(void *arg)
{
    int id = *(int *)arg;
    printf("Thread %d: pthread_self=%lu\n", id, (unsigned long)pthread_self());
    return NULL;
}

int main(void)
{
    pthread_t th[NTHREADS];
    int ids[NTHREADS];
    for (int i = 0; i < NTHREADS; ++i)
    {
        ids[i] = i + 1;
        if (pthread_create(&th[i], NULL, thread_func, &ids[i]) != 0)
        {
            perror("pthread_create");
            return 1;
        }
    }
    for (int i = 0; i < NTHREADS; ++i)
    {
        pthread_join(th[i], NULL);
    }
    return 0;
}
