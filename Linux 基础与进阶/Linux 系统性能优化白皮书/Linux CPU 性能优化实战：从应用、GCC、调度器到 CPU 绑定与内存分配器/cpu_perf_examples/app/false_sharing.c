#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define THREADS 4
#define ITERS 5000000ULL

typedef struct { uint64_t v; char pad[64 - sizeof(uint64_t)]; } padded64;
static padded64 good[THREADS];
static uint64_t bad[THREADS];

static double now_sec(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

static void *worker_good(void *arg) {
    int id = *(int *)arg;
    for (uint64_t i = 0; i < ITERS; i++) { good[id].v++; __asm__ __volatile__("" ::: "memory"); }
    return NULL;
}

static void *worker_bad(void *arg) {
    int id = *(int *)arg;
    for (uint64_t i = 0; i < ITERS; i++) { bad[id]++; __asm__ __volatile__("" ::: "memory"); }
    return NULL;
}

int main(int argc, char **argv) {
    int use_padded = argc > 1 && argv[1][0] == 'p';
    pthread_t t[THREADS];
    int ids[THREADS];
    double st = now_sec();

    for (int i = 0; i < THREADS; i++) {
        ids[i] = i;
        if (pthread_create(&t[i], NULL, use_padded ? worker_good : worker_bad, &ids[i])) {
            perror("pthread_create");
            return 1;
        }
    }
    for (int i = 0; i < THREADS; i++) pthread_join(t[i], NULL);

    double en = now_sec();
    uint64_t sum = 0;
    for (int i = 0; i < THREADS; i++) sum += use_padded ? good[i].v : bad[i];
    printf("mode=%s threads=%d iters=%llu sum=%llu elapsed=%.3f s\n",
           use_padded ? "padded" : "shared", THREADS,
           (unsigned long long)ITERS, (unsigned long long)sum, en - st);
    return 0;
}
