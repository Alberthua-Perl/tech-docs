#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define BYTES (16U * 1024U * 1024U)
#define CHUNK 8192U

static double now_sec(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

int main(int argc, char **argv) {
    int batch = argc > 1 && argv[1][0] == 'b';
    int fd = open("/dev/null", O_WRONLY);
    if (fd < 0) { perror("open"); return 1; }

    char *buf = malloc(BYTES);
    if (!buf) { perror("malloc"); return 1; }
    memset(buf, 'x', BYTES);

    double st = now_sec();
    if (batch) {
        for (size_t off = 0; off < BYTES; off += CHUNK) {
            if (write(fd, buf + off, CHUNK) != (ssize_t)CHUNK) { perror("write"); return 1; }
        }
    } else {
        for (size_t off = 0; off < BYTES; off++) {
            if (write(fd, buf + off, 1) != 1) { perror("write"); return 1; }
        }
    }
    double en = now_sec();

    printf("mode=%s bytes=%u syscalls≈%u elapsed=%.3f s\n",
           batch ? "batch_8k" : "one_byte", BYTES,
           batch ? (unsigned)(BYTES / CHUNK) : (unsigned)BYTES, en - st);
    free(buf);
    close(fd);
    return 0;
}
