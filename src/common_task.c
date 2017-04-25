#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <time.h>

#include "common_task.h"

#define ERR(source) (fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
                     perror(source),kill(0,SIGKILL),\
					 exit(EXIT_FAILURE))

#define MAX_NAME 32
#define MAX_LINE 80

void info(void) {
	printf("- Welcome to 'Wyścig Szczurów' game!\n");
	printf("You will check how fast you can type.\n");
	printf("Compete with other players and win!!!\n");
}

void author(void) {
	printf("Created by: Grzegorz Czarnocki\n");
	printf("Warsaw University of Technology\n");
	printf("Warsaw | 2017 (c)\n");
}

char* get_nickname() {
	char data[MAX_NAME];
	char* name = malloc(MAX_NAME * sizeof(char));
	
	printf("Type your nickname: ");
	fgets(data, sizeof(data), stdin);
	sscanf(data, "%s", name);

	return name;
}

void read_file(char* path, char** words, int words_cnt) {
	char* line = (char*)malloc((MAX_LINE + 1) * sizeof(char));
	char data[MAX_LINE + 1];

	FILE *fp;
	int i = 0;
	
	fp = fopen(path, "r");
	if(fp == NULL)
		ERR("fopen");

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

char** get_random_words(char** words, int words_cnt, int n) {
	if(n > words_cnt || n <= 0)
		return NULL;
		
	char** output;
	int i, j = 0, index;
	int curr_max = words_cnt - 1;
	
	output = (char**)malloc(n * sizeof(char*));
	for(i = 0; i < n; i++)
		output[i] = (char*)malloc(MAX_LINE * sizeof(char));
	
	srand(time(NULL));
	
	while(j < n) {
		index = rand() % curr_max;
		output[j] = words[index];

		char* tmp = words[index];
		words[index] = words[curr_max];
		words[curr_max] = tmp;
		
		curr_max--;
		j++;
	}
	
	return output;
}
