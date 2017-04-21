#ifndef _SOCKETS_H
#define _SOCKETS_H

int make_socket(int domain, int type);
int bind_tcp_socket(uint16_t port)
int add_new_client(int sfd);

#endif
