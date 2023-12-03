#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int main() {
    int fd, nread;
    char buff[16];
    fd = open("file.txt", O_RDONLY);
    if (fd == -1) {
        perror("open");
        exit(1);
    }

    nread = read(fd, buff, 16);
    printf("%s\n", buff);
}