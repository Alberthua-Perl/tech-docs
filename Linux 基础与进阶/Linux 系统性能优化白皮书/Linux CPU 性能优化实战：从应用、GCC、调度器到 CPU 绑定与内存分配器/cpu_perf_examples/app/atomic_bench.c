#include <pthread.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define THREADS 4
#define ITERS 10000000ULL
static _Atomic uint64_t total = 0;

static double now_sec(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

static void *worker_global(void *unused) {
    (void)unused;
    for (uint64_t i = 0; i < ITERS; i++)
        atomic_fetch_add_explicit(&total, 1, memory_order_relaxed);
    return NULL;
}

static void *worker_local(void *unused) {
    (void)unused;
    uint64_t local = 0;
    for (uint64_t i = 0; i < ITERS; i++) local++;
    atomic_fetch_add_explicit(&total, local, memory_order_relaxed);
    return NULL;
}

int main(int argc, char **argv) {
    int local_first = argc > 1 && argv[1][0] == 'l';
    pthread_t t[THREADS];
    double st = now_sec();
    for (int i = 0; i < THREADS; i++) {
        if (pthread_create(&t[i], NULL, local_first ? worker_local : worker_global, NULL)) {
            perror("pthread_create");
            return 1;
        }
    }
    for (int i = 0; i < THREADS; i++) pthread_join(t[i], NULL);
    double en = now_sec();
    printf("mode=%s threads=%d iters_per_thread=%llu total=%llu elapsed=%.3f s\n",
           local_first ? "local_accumulate" : "global_atomic", THREADS,
           (unsigned long long)ITERS, (unsigned long long)total, en - st);
    return 0;
}
