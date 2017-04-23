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

#include "common.h"
#include "common_task.h"

#define ERR(source) (perror(source),\
					fprintf(stderr, "%s: %d\n", __FILE__, __LINE__),\
					exit(EXIT_FAILURE))

#define MAX_LINE 80

volatile sig_atomic_t do_work = 0;

void sigint_handler(int sig) {
	do_work = 0;
}

void read_file(char* path, char** words, int words_cnt) {
	char* line = (char*)malloc((MAX_LINE + 1) * sizeof(char));
	char data[MAX_LINE + 1];

	FILE *fp;
	int i = 0;
	
	fp = fopen(path, "r");
	if(fp == NULL)
		ERR("fopen");
		
	// while((size = getline(&line, &len, fp)) != -1) {
	while(fgets(data, sizeof(data), fp) != NULL && i < words_cnt) {
		sscanf(data, "%s", line);
		size_t len = strlen(line);
		
		if(len > 0 && line[len - 1] == '\n')
			line[len - 1] = '\0';
		
		strncpy(words[i], line, MAX_LINE);
		i++;
	}
	
	fclose(fp);
	if(line) free(line);
}

char** build_array(char* path, int words_cnt) {
	int i;
	
	char** words = (char**)malloc(sizeof(char*) * words_cnt);
	if(!words)
		ERR("malloc");
		
	for(i = 0; i < words_cnt; i++) {
		words[i] = malloc((MAX_LINE + 1) * sizeof(char));

		if(!words[i]) {
			free(words);
			return NULL;
		}
	}
	
	read_file(path, words, words_cnt);
	
	return words;
}

void get_random_words(char** words, int words_cnt, int n) {
	if(n > words_cnt)
		return;
		
	// char* output = (char*)malloc(n * MAX_LINE * sizeof(char));
	srand(time(NULL));
	
	int i = 0, index;
	int curr_max = words_cnt - 1;
	
	while(i < n) {
		index = rand() % curr_max;
		
		char* tmp = words[index];
		words[curr_max] = words[index];
		words[index] = tmp;
		
		curr_max--;
		i++;
	}
	
	for(i = words_cnt; i > curr_max; i--) {
		printf("%s\n", words[i]);
	}
}

void print_array(char** words, int words_cnt) {
	int i = 0;
	
	for(; i < words_cnt; i++)
		printf("[%d] %s\n", i, words[i]);
}

void usage(char* name) {
	fprintf(stderr, "USAGE: %s port file_name lines_cnt\n", name);
}

void do_server(int fdt, char** words, int words_cnt) {

}

int main(int argc, char** argv) {
	int fdt;
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
	
	fdt = bind_tcp_socket(atoi(argv[1]));
	
	new_flags = fcntl(fdt, F_GETFL) | O_NONBLOCK;
	fcntl(fdt, F_SETFL, new_flags);
	
	filename = argv[2];
	words_cnt = atoi(argv[3]);
	words = build_array(filename, words_cnt);
	
	info();
	do_server(fdt, words, words_cnt);
	
	if(TEMP_FAILURE_RETRY(close(fdt)) < 0) ERR("close");
	fprintf(stderr, "Server has terminated.\n");
	
	return EXIT_SUCCESS;
}
