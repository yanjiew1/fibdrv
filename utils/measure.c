#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define FIB_DEV "/dev/fibonacci"

long get_nanosecond()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000000000 + ts.tv_nsec;
}

int main(int argc, char *argv[])
{
    char buf[300];
    int offset = 1000;
    int impl = 4;

    int fd = open(FIB_DEV, O_RDWR);
    if (fd < 0) {
        perror("Failed to open character device");
        exit(1);
    }

    if (argc > 1)
        impl = atoi(argv[1]);

    if (argc > 2)
        offset = atoi(argv[2]);

    /* Change implementation */
    write(fd, buf, impl);

    /* Discard the first round measurement */
    for (int i = 0; i <= offset; i++) {
        lseek(fd, i, SEEK_SET);
        read(fd, buf, 300);
        get_nanosecond();
    }

    for (int i = 0; i <= offset; i++) {
        long start, ktime, utime;

        /* Perform the operation without measurement to avoid cache misses */
        get_nanosecond();
        lseek(fd, i, SEEK_SET);
        read(fd, buf, 300);

        lseek(fd, i, SEEK_SET);
        start = get_nanosecond();
        read(fd, buf, 300);
        utime = get_nanosecond() - start;
        ktime = write(fd, buf, 1) - write(fd, buf, 0);
        printf("%d %ld %ld %ld\n", i, ktime, utime, utime - ktime);
    }

    close(fd);
    return 0;
}
