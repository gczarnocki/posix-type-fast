#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "common.h"

int main(int argc, char** argv) {
	char* nickname;

	nickname = get_nickname();
	printf("Your nickname is: %s\n", nickname);

	return EXIT_SUCCESS;
}
