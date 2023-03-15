#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "bnstr.h"

#define FIB_DEV "/dev/fibonacci"

static void read_number(int fd, int k)
{
    uint64_t res[15];
    char str[301];
    ssize_t sz;
    lseek(fd, k, SEEK_SET);
    sz = read(fd, &res, sizeof(res));

    if (sz < 0) {
        perror("Failed to read from character device");
        return;
    }

    sz /= sizeof(uint64_t);
    bn_to_str(str, res, sz);
    printf("Reading from " FIB_DEV
           " at offset %d, returned the sequence "
           "%s.\n",
           k, str);
}

int main(int argc, char **argv)
{
    unsigned long res[100];

    int offset = 1000;

    int fd = open(FIB_DEV, O_RDWR);
    if (fd < 0) {
        perror("Failed to open character device");
        exit(1);
    }

    /* Choose implementation */
    int impl = 4;
    if (argc > 1)
        impl = atoi(argv[1]);

    write(fd, res, impl);

#if 0
    for (int i = 0; i <= offset; i++) {
        sz = write(fd, write_buf, strlen(write_buf));
        printf("Writing to " FIB_DEV ", returned the sequence %lld\n", sz);
    }
#endif

    for (int i = 0; i <= offset; i++)
        read_number(fd, i);

    for (int i = offset; i >= 0; i--)
        read_number(fd, i);

    close(fd);
    return 0;
}
