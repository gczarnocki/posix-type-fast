#ifndef _COMMON_H
#define _COMMON_H

ssize_t bulk_read(int fd, char *buf, size_t count);
ssize_t bulk_write(int fd, char *buf, size_t count);
int sethandler(void (*f)(int), int sigNo);
int make_socket(int domain, int type);
int bind_tcp_socket(uint16_t port);
int add_new_client(int sfd);

#endif
