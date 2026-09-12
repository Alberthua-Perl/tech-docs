#include <stdio.h>
#include <stdlib.h>
#include <time.h>
int main(int argc, char **argv) {
    int seconds = argc > 1 ? atoi(argv[1]) : 5;
    time_t start = time(NULL);
    volatile unsigned long long x = 0;
    while (time(NULL) - start < seconds) x++;
    printf("cpu_burner done x=%llu\n", x);
    return 0;
}
