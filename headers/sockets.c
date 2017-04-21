#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#define ERR(source) (perror(source),\
                	fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
                	exit(EXIT_FAILURE))

#define BACKLOG 3

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
