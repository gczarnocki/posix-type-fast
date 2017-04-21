#ifndef _IO_H
#define _IO_H

ssize_t bulk_read(int fd, char* buf, size_t count);
ssize_t bulk_write(int fd, char* buf, size_t count);

#endif
