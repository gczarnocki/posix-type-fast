#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <signal.h>
#include <time.h>
#include <fcntl.h>
#include <pthread.h>

#include "common.h"
#include "common_task.h"

#define ERR(source) (perror(source),\
					fprintf(stderr, "%s: %d\n", __FILE__, __LINE__),\
					exit(EXIT_FAILURE))
					
#define MAX_LINE 80
#define MAX_NAME 32
#define MAX_CLIENT 2

struct client_info {
	char nickname[MAX_NAME];
	int results[MAX_CLIENT];
	int score;
	int id;
};

volatile sig_atomic_t do_work = 1;
volatile sig_atomic_t client_id = -1;

void remove_new_line(char* string) {
	int i;
	for(i = strlen(string) - 1; i >= 0; i--) {
		if(string[i] == '\r' || string[i] == '\n')
			string[i] = '\0';
		else
			break;
	}
}

int get_client_id() {
	return ++client_id;
}

void sigint_handler(int sig) {
	do_work = 0;
}

void usage(char* name) {
	fprintf(stderr, "USAGE: %s port file_name lines_cnt\n", name);
}

int are_words_equal(char* pattern, char* input) {
	int len_pattern = strlen(pattern);
	int len_input = strlen(input);
	int i = 0;
	
	if(len_input != len_pattern)
		return -1;
	else {
		for(; i < len_input; i++) {
			if(pattern[i] != input[i])
				return 0;
		}
	}
	
	return 1;
}

void communicate(int cfd, struct client_info* clients, char** random_words, int n) {
	ssize_t size;
	char nick[MAX_NAME];
	char data_r[MAX_LINE];
	char data_w[MAX_LINE];
		
	memset(data_w, 0, sizeof(char) * MAX_LINE);
	memset(data_r, 0, sizeof(char) * MAX_LINE);
	strcpy(data_w, "Nickname:\n");
	
	if(bulk_write(cfd, data_w, MAX_LINE) < 0 && errno != EPIPE) ERR("write");
	if((size = read(cfd, data_r, MAX_NAME)) < 0) ERR("read"); // read nickname
	remove_new_line(data_r);
	strcpy(nick, data_r);
	printf("[%s] connected.", nick);
	fflush(stdout);
	
	int i = 0;
	
	for(; i < n; i++) {
		if(bulk_write(cfd, random_words[i], MAX_LINE) < 0 && errno != EPIPE) ERR("write");
		if(bulk_write(cfd, "\n", sizeof(char)) < 0 && errno != EPIPE) ERR("write");
		
		memset(data_r, 0, sizeof(char) * MAX_LINE);
		if((size = read(cfd, data_r, MAX_LINE)) < 0) ERR("read");
		remove_new_line(data_r);
		
		if(strcmp(random_words[i], data_r) == 0)
			printf("Success: %s\n", data_r);
	}
	
	// strncpy(clients[client_id].nickname, nickname, MAX_NAME);
	// clients[client_id].id = get_client_id();
		
	if(TEMP_FAILURE_RETRY(close(cfd)) < 0) ERR("close");
}

void do_server(int fdt, char** words, int words_cnt, struct client_info* clients) {
	int cfd;
	char** random_words;
	fd_set base_rfds, rfds;
	sigset_t mask, oldmask;
	
	FD_ZERO(&base_rfds);
	FD_SET(fdt, &base_rfds);
	
	sigemptyset(&mask);
	sigaddset(&mask, SIGINT);
	sigprocmask(SIG_BLOCK, &mask, &oldmask);
	
	while(do_work) {
		rfds = base_rfds;
		
		random_words = get_random_words(words, words_cnt, 5);
		
		if(pselect(fdt + 1, &rfds, NULL, NULL, NULL, &oldmask) > 0) {
			if(FD_ISSET(fdt, &rfds)) cfd = add_new_client(fdt);
			if(cfd >= 0) communicate(cfd, clients, random_words, 5);
		} else {
			if(EINTR == errno) continue;
			ERR("pselect");
		}
	}
	
	sigprocmask(SIG_UNBLOCK, &mask, NULL);
}

int main(int argc, char** argv) {
	int fdt, i = 0;
	int new_flags;
	
	char* filename;
	char** words;
	int words_cnt;
	
	if(argc != 4) {
		usage(argv[0]);
		return EXIT_FAILURE;
	}
	
	if(sethandler(SIG_IGN, SIGPIPE)) ERR("Setting SIGPIPE");
	if(sethandler(sigint_handler, SIGINT)) ERR("Setting SIGINT");
	
	struct client_info clients[MAX_CLIENT];
	
	fdt = bind_tcp_socket(atoi(argv[1]));
	
	new_flags = fcntl(fdt, F_GETFL) | O_NONBLOCK;
	fcntl(fdt, F_SETFL, new_flags);
	
	filename = argv[2];
	words_cnt = atoi(argv[3]);
	words = build_array(filename, words_cnt);
	
	info();
	do_server(fdt, words, words_cnt, clients);
	
	for(i = 0; i < words_cnt; i++) {
		free(words[i]);
	}
	
	free(words);
	
	if(TEMP_FAILURE_RETRY(close(fdt)) < 0) ERR("close");
	fprintf(stderr, "Server has terminated.\n");
	
	return EXIT_SUCCESS;
}
