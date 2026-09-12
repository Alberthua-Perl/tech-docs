CC ?= gcc
CFLAGS ?= -O2 -g -pthread -Wall -Wextra
BINS := app/false_sharing app/atomic_bench app/syscall_batch app/zero_copy_sendfile app/matrix_mult app/malloc_bench app/cpu_burner

all: $(BINS)

app/%: app/%.c
	$(CC) $(CFLAGS) $< -o $@

app/matrix_mult: app/matrix_mult.c
	$(CC) $(CFLAGS) -DN=384 $< -o $@

clean:
	rm -f $(BINS)

.PHONY: all clean
