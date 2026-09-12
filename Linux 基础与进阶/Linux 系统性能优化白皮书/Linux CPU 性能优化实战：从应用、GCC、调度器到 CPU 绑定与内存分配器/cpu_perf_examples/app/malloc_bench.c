#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define THREADS 4
#define ITERS 2000000U

static double now_sec(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

static uint64_t xs(uint64_t *s) {
    *s ^= *s << 13; *s ^= *s >> 7; *s ^= *s << 17; return *s;
}

static void *worker(void *arg) {
    uint64_t s = 0x9e3779b97f4a7c15ULL ^ (uintptr_t)arg;
    for (uint32_t i = 0; i < ITERS; i++) {
        size_t sz = 32 + (size_t)(xs(&s) % 224); /* 32..255 bytes */
        char *p = malloc(sz);
        if (!p) { perror("malloc"); exit(1); }
        memset(p, (int)(sz & 0xff), sz);
        free(p);
    }
    return NULL;
}

int main(void) {
    pthread_t t[THREADS];
    double st = now_sec();
    for (int i = 0; i < THREADS; i++) {
        if (pthread_create(&t[i], NULL, worker, (void *)(uintptr_t)(i + 1))) {
            perror("pthread_create"); return 1;
        }
    }
    for (int i = 0; i < THREADS; i++) pthread_join(t[i], NULL);
    double en = now_sec();
    printf("threads=%d iters_per_thread=%u elapsed=%.3f s\n", THREADS, ITERS, en - st);
    return 0;
}
