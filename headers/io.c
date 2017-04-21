#define _GNU_SOURCE

#include <unistd.h>
#include <errno.h>

ssize_t bulk_read(int fd, char *buf, size_t count){
    int c;
    size_t len = 0;

    do {
            c = TEMP_FAILURE_RETRY(read(fd, buf, count));
            if(c < 0) return c;
            if(0 == c) return len;
            buf += c;
            len += c;
            count -= c;
    } while(count > 0);

    return len;
}

ssize_t bulk_write(int fd, char *buf, size_t count){
    int c;
    size_t len = 0;

    do {
            c = TEMP_FAILURE_RETRY(write(fd, buf, count));
            if(c < 0) return c;
            if(0 == c) return len;
            buf += c;
            len += c;
            count -= c;
    } while(count > 0);

    return len;
}
