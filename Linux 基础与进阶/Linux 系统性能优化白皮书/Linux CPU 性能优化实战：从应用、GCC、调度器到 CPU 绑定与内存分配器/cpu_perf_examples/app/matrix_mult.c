#include <stdio.h>
#include <time.h>
#ifndef N
#define N 384
#endif
static double A[N][N], B[N][N], C[N][N];

static double now_sec(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

int main(void) {
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++) {
            A[i][j] = (i + 1) * 0.5 + j * 0.25;
            B[i][j] = (j + 1) * 0.75 - i * 0.125;
        }
    double st = now_sec();
    for (int i = 0; i < N; i++)
        for (int k = 0; k < N; k++) {
            double a = A[i][k];
            for (int j = 0; j < N; j++)
                C[i][j] += a * B[k][j];
        }
    double en = now_sec();
    volatile double sink = C[N/2][N/2];
    (void)sink;
    printf("N=%d elapsed=%.3f s checksum=%f\n", N, en - st, C[N/2][N/2]);
    return 0;
}
