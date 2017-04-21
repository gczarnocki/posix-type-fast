#ifndef _SIGNALS_H
#define _SIGNALS_H

int sethandler(void (*f)(int), int sig);
void sigchld_handler(int sig);

#endif
