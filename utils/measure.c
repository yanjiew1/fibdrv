#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#define FIB_DEV "/dev/fibonacci"

int main()
{
    char buf[] = "testing";
    int offset = 100; /* TODO: try test something bigger than the limit */

    int fd = open(FIB_DEV, O_RDWR);
    if (fd < 0) {
        perror("Failed to open character device");
        exit(1);
    }

    for (int i = 0; i <= offset; i++) {
        long long kt;
        lseek(fd, i, SEEK_SET);
        read(fd, buf, 0);
        kt = write(fd, buf, 1) - write(fd, buf, 0);
        printf("%d: %lld\n", i, kt);
    }

    close(fd);
    return 0;
}
