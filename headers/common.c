#include <stdio.h>
#include <stdlib.h>

#include "common.h"

#define MAX_NAME 32

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
