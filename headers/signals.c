#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>

#include "signals.h"

#define ERR(source) (fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
                     perror(source),kill(0,SIGKILL),\
					 exit(EXIT_FAILURE))

int sethandler(void (*f)(int), int sigNo) {
	struct sigaction act;
	memset(&act, 0, sizeof(struct sigaction));
	act.sa_handler = f;

	if (-1 == sigaction(sigNo, &act, NULL))
	    return -1;
	return 0;
}

void sigchld_handler(int sig) {
	pid_t pid;

	for(;;) {
		pid = waitpid(0, NULL, WNOHANG);
		if(0 == pid) return;
		if(0 >= pid) {
			if(ECHILD == errno) return;
			ERR("waitpid:");
		}
	}
}
