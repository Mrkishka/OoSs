#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define NTHREADS 8
#define NINCR 1000000

long counter = 0;

void *worker(void *arg)
{
    for (int i = 0; i < NINCR; ++i)
    {
        counter++;
    }
    return NULL;
}

int main(void)
{
    pthread_t th[NTHREADS];
    for (int i = 0; i < NTHREADS; ++i)
    {
        if (pthread_create(&th[i], NULL, worker, NULL) != 0)
        {
            perror("pthread_create");
            return 1;
        }
    }
    for (int i = 0; i < NTHREADS; ++i)
        pthread_join(th[i], NULL);
    printf("Expected counter = %ld, actual counter = %ld\n", (long)NTHREADS * NINCR, counter);
    return 0;
}
