#include "common_task.h"

void info(int16_t port) {
	printf("------- Witaj w grze 'Wyścig szczurów'. --------\n");
	printf("- Przepisuj słowa szybciej, niż inni i wygraj! -\n");
	printf("------- Aby grać, użyj telnetu. Port: %d -----\n", port);
}

void author(void) {
	printf("Stworzone przez: Grzegorz Czarnocki\n");
	printf("Politechnika Warszawska, wydz. MiNI\n");
	printf("--- Warszawa | maj 2017 r. (c) ----\n");
}

void remove_new_line(char* str) {
	int i;
	for(i = strlen(str) - 1; i >= 0; i--) {
		if(str[i] == '\r' || str[i] == '\n')
			str[i] = '\0';
		else
			break;
	}
}

int are_words_equal(char* pattern, char* input) {
	int len_pattern = strlen(pattern);
	int len_input = strlen(input);
	int i = 0;

	if(len_input != len_pattern)
		return 0;
	else {
		for(; i < len_input; i++) {			
			if(pattern[i] != input[i]) {
				return 0;
			}
		}
	}

	return 1;
}

int parse_arguments(int argc, char** argv, uint16_t* port) {
	if(argc != 3) {
		return 0;
	} else {
		*port = atoi(argv[1]);
	
		if(!is_valid_port(*port)) {
			return 0;
		}
	}
	
	return 1;
}

void find_indexes(int* indexes, int array_size, int max_range) {
	srand(time(NULL));
	
	int i = 0;
	int data[max_range];
	
	for(i = 0; i < max_range; i++) {
		data[i] = i;
	}
	
	for(i = 0; i < max_range; i++) {
		int temp = data[i];
		int rnd = rand() % max_range;
	
		data[i] = data[rnd];
		data[rnd] = temp;
	}
	
	for(i = 0; i < array_size; i++) {
		indexes[i] = data[i];
	}
}

int if_nickname_exists(char* name) {
	int i = 0;
	
	for(; i < MAX_CLIENTS; i++) {
		if(clients[i] != NULL && 
		strcmp(clients[i]->nickname, name) == 0) {
			return 1;
		}
	}
	
	return 0;
}

void get_client_nickname(int client_socket, char* nickname) {
	char data[MAX_LINE];
	int do_work = 1;
	size_t size;
	
	while(do_work) {
		memset(data, 0, sizeof(data));
		strncpy(data, "Podaj nazwę:\n", MAX_LINE);
		socket_write(client_socket, data, sizeof(data));
		memset(data, 0, sizeof(data));
		size = socket_read(client_socket, data, sizeof(data));
		
		if(size > 0) {
			remove_new_line(data);
		}
		
		if(!if_nickname_exists(data)) {
		// wychodzimy z pętli, jeśli nazwa nie jest używana;
			do_work = 0;
			strncpy(nickname, data, MAX_LINE);
		}
	}
}

void print_scoreboard() {
	int scoreboard_size = MAX_LINE * MAX_CLIENTS * sizeof(char);
	char* scoreboard = (char*)malloc(scoreboard_size);
	get_scoreboard(scoreboard);
	printf("%s", scoreboard);
	free(scoreboard);
}

void get_scoreboard(char* buffer) {
	int i = 0;
	char tmp[MAX_LINE];
	
	int cnt = connected_clients_count();
	
	if(cnt > 0) {
		strncpy(tmp, "### Ranking ###\n", MAX_LINE);
		strcat(buffer, tmp);
	
		for(; i < MAX_CLIENTS; i++) {
			if(clients[i] != NULL && clients[i]->connected == 1) {
				snprintf(tmp, MAX_LINE, "[%d: %s] %d pts\n",
					clients[i]->id,
					clients[i]->nickname,
					clients[i]->score);
					
				strcat(buffer, tmp);
			}
		}
	} else {
		printf("Brak podłączonych klientów.\n");
	}
}

void send_scoreboard_to_clients(game* this_game) {	
	int scoreboard_size = MAX_LINE * MAX_CLIENTS * sizeof(char);
	char* scoreboard = (char*)malloc(scoreboard_size);
	
	get_scoreboard(scoreboard);
	socket_write(this_game->player->fd, scoreboard, scoreboard_size);
	socket_write(this_game->opponent->fd, scoreboard, scoreboard_size);
	free(scoreboard);
}

int connected_clients_count() {
	int i = 0, cnt = 0;
	
	for(; i < MAX_CLIENTS; i++) {
		if(clients[i] != NULL &&
		clients[i]->connected == 1) {
			cnt++;
		}
	}
	
	return cnt;
}
