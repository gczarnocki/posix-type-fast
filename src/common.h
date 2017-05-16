#ifndef _COMMON_H
#define _COMMON_H

#define _GNU_SOURCE

#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define MAX_PORT 65535
#define BACKLOG 3

#ifndef TEMP_FAILURE_RETRY
#define TEMP_FAILURE_RETRY(expression) 			\
(												\
	({ long int __result; 						\
	do __result = (long int) (expression); 		\
	while (__result == -1L && errno == EINTR); 	\
	__result; }))
#endif

#ifndef ERR
#define ERR(source) 												\
do {																\
	fprintf(stderr, "[%s] %s @ %d\n", "ERROR", __FILE__, __LINE__); \
    perror(source); kill(0, SIGKILL); 								\
	exit(EXIT_FAILURE);												\
} while(0)
#endif

#ifndef LOG_ERROR
#define LOG_ERROR(message) 											\
do { 																\
	fprintf(stderr, "[%s] %s @ %d: ", "ERROR", __FILE__, __LINE__); \
	perror(message); 												\
} while(0)
#endif

#ifndef EXIT
#define EXIT(source)	\
do { 					\
	LOG_ERROR(source); 	\
	kill(0, SIGKILL); 	\
	exit(EXIT_FAILURE); \
} while(0)
#endif

/* Files */
void safe_fflush(FILE* stream);
void safe_close(int fd);
void set_nonblock(int fd);

/* I/O */
ssize_t bulk_read(int fd, char *buf, size_t count);
ssize_t bulk_read_nb(int fd, char *buf, size_t count);
ssize_t bulk_write(int fd, char *buf, size_t count);

/* Signals */
void set_handler(void (*f)(int), int sigNo);

/* Sockets */
int is_valid_port(uint16_t port);
int make_socket(int domain, int type);
int bind_inet_socket(uint16_t port, int type);
int accept_client(int sfd);

ssize_t socket_read(int fd, char* buf, size_t count);
ssize_t socket_write(int fd, char* buf, size_t count);

/* Threads */

void create_thread(pthread_t *thread, const pthread_attr_t *attr, void* (*handler)(void*), void *arg);
void create_detached_thread(pthread_t *thread, void* (*handler)(void*), void *arg);

void mutex_lock(pthread_mutex_t *mutex);
void mutex_unlock(pthread_mutex_t *mutex);
void mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr);

void cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr);
void cond_signal(pthread_cond_t* cond);

#endif
