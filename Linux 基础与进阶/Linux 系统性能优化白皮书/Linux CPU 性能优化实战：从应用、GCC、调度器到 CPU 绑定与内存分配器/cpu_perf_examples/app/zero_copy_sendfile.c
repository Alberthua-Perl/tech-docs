#define _GNU_SOURCE
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#define SRC "/tmp/cpuopt_zero_copy_src.bin"
#define SRC_SIZE (64U * 1024U * 1024U)
#define CHUNK (1024U * 1024U)

static double now_sec(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

static int ensure_src(void) {
    int fd = open(SRC, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) { perror("open src"); return -1; }
    char *buf = malloc(CHUNK);
    if (!buf) { perror("malloc"); return -1; }
    memset(buf, 'a', CHUNK);
    for (size_t off = 0; off < SRC_SIZE; off += CHUNK) {
        if (write(fd, buf, CHUNK) != (ssize_t)CHUNK) { perror("write src"); return -1; }
    }
    free(buf);
    close(fd);
    return 0;
}

static int copy_read_write(void) {
    int in = open(SRC, O_RDONLY);
    int out = open("/dev/null", O_WRONLY);
    if (in < 0 || out < 0) { perror("open"); return -1; }
    char *buf = malloc(CHUNK);
    if (!buf) { perror("malloc"); return -1; }
    double st = now_sec();
    ssize_t n;
    while ((n = read(in, buf, CHUNK)) > 0) {
        if (write(out, buf, (size_t)n) != n) { perror("write"); return -1; }
    }
    double en = now_sec();
    printf("read_write copy: %.3f s\n", en - st);
    free(buf); close(in); close(out);
    return n < 0 ? -1 : 0;
}

static int copy_sendfile(void) {
    int in = open(SRC, O_RDONLY);
    int out = open("/dev/null", O_WRONLY);
    if (in < 0 || out < 0) { perror("open"); return -1; }
    double st = now_sec();
    off_t off = 0;
    while (off < (off_t)SRC_SIZE) {
        ssize_t n = sendfile(out, in, &off, CHUNK);
        if (n < 0) { perror("sendfile"); return -1; }
        if (n == 0) break;
    }
    double en = now_sec();
    printf("sendfile   copy: %.3f s\n", en - st);
    close(in); close(out);
    return 0;
}

int main(void) {
    if (ensure_src()) return 1;
    if (copy_read_write()) return 1;
    if (copy_sendfile()) return 1;
    unlink(SRC);
    return 0;
}
