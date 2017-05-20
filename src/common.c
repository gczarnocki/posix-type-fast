#include "common.h"

/* files */

void safe_fflush(FILE* stream) {
	if(TEMP_FAILURE_RETRY(fflush(stream)) == EOF) {
		EXIT("fflush");
	}
}

void safe_close(int fd) {
	if(TEMP_FAILURE_RETRY(close(fd)) < 0) {
		EXIT("close");
	}
}

void set_nonblock(int fd) {
	int new_flags = fcntl(fd, F_GETFL) | O_NONBLOCK;
	if(fcntl(fd, F_SETFL, new_flags) == -1) {
		EXIT("fcntl");
	}
}

/* I/O */

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

ssize_t bulk_read_nb(int fd, char *buf, size_t count) {
	int c;
	size_t len = 0;
	
	do {
		c = read(fd, buf, count);
		if(c < 0) {
			if(EINTR == errno) continue;
			if(EAGAIN == errno) return len;
			return c;
		}
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
		if(c < 0)
			return c;
		if(0 == c)
			return len;
		buf += c;
		len += c;
		count -= c;
    } while(count > 0);

    return len;
}

/* signals */

void set_handler(void (*f)(int), int sigNo) {
	struct sigaction act;
	memset(&act, 0, sizeof(struct sigaction));
	act.sa_handler = f;

	if (-1 == sigaction(sigNo, &act, NULL)) {
	    EXIT("sigaction");
	}
}

/* sockets */

int is_valid_port(uint16_t port) {
	return port > 0 && port <= MAX_PORT;
}

int make_socket(int domain, int type) {
	int sock;
	sock = socket(domain, type, 0);
	if(sock < 0) {
		EXIT("socket");
	}
	
	return sock;
}

int bind_inet_socket(uint16_t port, int type) {
	struct sockaddr_in addr;
	int socket_fd, t = 1;
	socket_fd = make_socket(PF_INET, type);
	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if(setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &t, sizeof(t)))
		EXIT("setsockopt");
	if(bind(socket_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
		EXIT("bind");
	if(type == SOCK_STREAM) {
		if(listen(socket_fd, BACKLOG) < 0)
			EXIT("listen");
	}
	
	return socket_fd;
}

int accept_client(int socket_fd) {
	int client_fd;
	if((client_fd = TEMP_FAILURE_RETRY(
			accept(socket_fd, NULL, NULL))) < 0) {
	    if(EAGAIN == errno || EWOULDBLOCK == errno) {
			return -1;
		}
	    EXIT("accept");
	}

	return client_fd;
}

ssize_t socket_read(int fd, char* buf, size_t count) {
	int c;
	// size_t len;
	
	while(1) {
		c = read(fd, buf, count);
		
		if(c < 0 && errno != EINTR && errno != EPIPE) EXIT("read");
		if(c <= 0 && errno == EPIPE) c = -1;
		if(c < 0 && errno == EINTR) continue;
		break;
	}
	
	return c;
}

ssize_t socket_write(int fd, char* buf, size_t count) {
	int c;
	size_t len = 0;
	
	do {
		c = write(fd, buf, count);
		
		if(c < 0 && errno != EPIPE && errno != EINTR)
			EXIT("write");
		if(errno == EPIPE) {
			len = -1;
			break;
		}
		if(c < 0 && errno == EINTR)
			continue;
		
		buf += c;
		len += c;
		count -= c;
	} while(count > 0);
	
	return len;
}

/* threads */

void create_thread(pthread_t *thread, const pthread_attr_t *attr, 
		void* (*handler)(void*), void *arg) {
	if(pthread_create(thread, attr, handler, arg) < 0) {
		EXIT("pthread_create");
	}
}

void create_detached_thread(pthread_t *thread, 
		void* (*handler)(void*), void *arg) {
	pthread_attr_t attr;
			
	if(pthread_attr_init(&attr) != 0) {
		EXIT("pthread_attr_init");
	}
	
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	
	if(pthread_create(thread, &attr, handler, arg) < 0) {
		EXIT("pthread_create");
	}
	
	pthread_detach(*thread);
	pthread_attr_destroy(&attr);
}

void mutex_lock(pthread_mutex_t *mutex) {
	if(pthread_mutex_lock(mutex) != 0) {
		EXIT("pthread_mutex_lock");
	}
}

void mutex_unlock(pthread_mutex_t *mutex) {
	if(pthread_mutex_unlock(mutex) != 0) {
		EXIT("pthread_mutex_lock");
	}
}

void mutex_init(pthread_mutex_t *mutex, 
		const pthread_mutexattr_t *attr) {
	if(pthread_mutex_init(mutex, attr) != 0) {
		EXIT("pthread_mutex_init");
	}
}

void cond_init(pthread_cond_t *cond, 
		const pthread_condattr_t *attr) {
	if(pthread_cond_init(cond, attr) != 0) {
		EXIT("pthread_cond_init");
	}
}

void cond_signal(pthread_cond_t* cond) {
	if(pthread_cond_signal(cond) != 0) {
		EXIT("pthread_cond_signal");
	}
}
