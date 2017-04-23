#define _GNU_SOURCE

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>


#include "common.h"

#define ERR(source) (fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
                     perror(source),kill(0,SIGKILL),\
					 exit(EXIT_FAILURE))

#define BACKLOG 3
#define MAX_NAME 32

/* IO */

ssize_t bulk_read(int fd, char *buf, size_t count) {
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

ssize_t bulk_write(int fd, char *buf, size_t count) {
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

/* signals */

int sethandler(void (*f)(int), int sigNo) {
	struct sigaction act;
	memset(&act, 0, sizeof(struct sigaction));
	act.sa_handler = f;

	if (-1 == sigaction(sigNo, &act, NULL))
	    return -1;
	return 0;
}

/* sockets */

int make_socket(int domain, int type) {
	int sock;
	sock = socket(domain, type, 0);
	if(sock < 0) ERR("socket");
	return sock;
}

int bind_tcp_socket(uint16_t port) {
	struct sockaddr_in addr;
	int socketfd, t = 1;
	socketfd = make_socket(PF_INET, SOCK_STREAM);
	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if(setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &t, sizeof(t)))
		ERR("setsockopt");
	if(bind(socketfd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
		ERR("bind");
	if(listen(socketfd, BACKLOG) < 0)
		ERR("listen");

	return socketfd;
}

int add_new_client(int sfd) {
	int nfd;
	if((nfd = TEMP_FAILURE_RETRY(accept(sfd, NULL, NULL))) < 0) {
	    if(EAGAIN == errno || EWOULDBLOCK == errno) 
			return -1;
	    ERR("accept");
	}

	return nfd;
}
