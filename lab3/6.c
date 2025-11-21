#define _POSIX_C_SOURCE 200809L
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>

#define NTHREADS 8
#define NINCR 500000

long counter;

typedef void *(*thread_fn_t)(void *);

typedef struct
{
    int id;
    pthread_mutex_t *pm;
    pthread_spinlock_t *ps;
    pthread_rwlock_t *prw;
} arg_t;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
void *worker_mutex(void *arg)
{
    (void)arg;
    for (int i = 0; i < NINCR; ++i)
    {
        pthread_mutex_lock(&mutex);
        ++counter;
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

pthread_spinlock_t spin;
void *worker_spin(void *arg)
{
    (void)arg;
    for (int i = 0; i < NINCR; ++i)
    {
        pthread_spin_lock(&spin);
        ++counter;
        pthread_spin_unlock(&spin);
    }
    return NULL;
}

pthread_rwlock_t rwlock = PTHREAD_RWLOCK_INITIALIZER;
void *worker_rw(void *arg)
{
    (void)arg;
    for (int i = 0; i < NINCR; ++i)
    {
        pthread_rwlock_wrlock(&rwlock);
        ++counter;
        pthread_rwlock_unlock(&rwlock);
    }
    return NULL;
}

static double now_sec(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

void run_test(const char *name, thread_fn_t fn)
{
    pthread_t th[NTHREADS];
    counter = 0;
    double t0 = now_sec();
    for (int i = 0; i < NTHREADS; ++i)
    {
        if (pthread_create(&th[i], NULL, fn, NULL) != 0)
        {
            perror("pthread_create");
            exit(1);
        }
    }
    for (int i = 0; i < NTHREADS; ++i)
        pthread_join(th[i], NULL);
    double t1 = now_sec();
    printf("%s: counter=%ld expected=%ld time=%.4f s\n",
           name, counter, (long)NTHREADS * NINCR, t1 - t0);
}

int main(void)
{
    if (pthread_spin_init(&spin, PTHREAD_PROCESS_PRIVATE) != 0)
    {
        perror("pthread_spin_init");
        return 1;
    }
    printf("Running tests with NTHREADS=%d NINCR=%d\n", NTHREADS, NINCR);

    run_test("pthread_mutex", worker_mutex);
    run_test("pthread_spin_lock", worker_spin);
    run_test("pthread_rwlock (write-lock)", worker_rw);

    pthread_spin_destroy(&spin);
    return 0;
}
