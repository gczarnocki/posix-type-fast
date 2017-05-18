#include "common_task.h"

/* helpers */

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

/* words */

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

/* nickname */

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

/* scoreboard */

void print_scoreboard(int id) {
	int scoreboard_size = MAX_LINE * MAX_CLIENTS * sizeof(char);
	char* scoreboard = (char*)malloc(scoreboard_size);
	get_scoreboard(scoreboard, id);
	printf("%s", scoreboard);
	free(scoreboard);
}

void print_game_result(int client_id, int opponent_id) {
	int res = clients[client_id]->results[opponent_id];
	char* nickname = clients[opponent_id]->nickname;
	
	char res_value[MAX_LINE];
	
	switch(res) {
		case 1:
			strncpy(res_value, "Przed grą", MAX_LINE);
			break;
		case 2:
			strncpy(res_value, "W trakcie gry", MAX_LINE);
			break;
		case 3:
			strncpy(res_value, "Wygrana", MAX_LINE);
			break;
		case 4:
			strncpy(res_value, "Przegrana", MAX_LINE);	
			break;
	}
	
	printf("\t-> [%s]: %s\n", nickname, res_value);
}

void print_detailed_scoreboard() {
	int i = 0, j = 0;
	int cnt = connected_clients_count();
	
	if(cnt == 0) {
		printf("Brak podłączonych klientów.\n");
		return;
	}
	
	printf("# Ranking+ #\n");
	
	for(i = 0; i < MAX_CLIENTS; i++) {
		if(clients[i] == NULL) continue;
		
		printf("- [Id: %d] [%s] [Wynik: %d pkt.]\n", 
			clients[i]->id, 
			clients[i]->nickname,
			clients[i]->score);
		printf("Połączony: [%d] | Wolny: [%d]\n",
			clients[i]->connected,
			clients[i]->idle);
			
		for(j = 0; j < MAX_CLIENTS; j++) {
			if(clients[j] == NULL || i == j) continue;
			
			print_game_result(i, j);
		}
	}
}

void get_scoreboard(char* buffer, int id) {
	int len = strlen(buffer) + 1;
	memset(buffer, 0, len);
	char indicator[] = "[*]";
	
	int i = 0;
	char tmp[MAX_LINE];
	memset(tmp, 0, MAX_LINE);
	
	int cnt = connected_clients_count();
	
	if(cnt > 0) {
		strncpy(tmp, "# Ranking #\n", MAX_LINE);
		strcat(buffer, tmp);
	
		for(; i < MAX_CLIENTS; i++) {
			if(clients[i] != NULL && clients[i]->connected == 1) {
				snprintf(tmp, MAX_LINE, 
					"[Id: %d] [%s] [Wynik: %d pkt.] %s\n",
					clients[i]->id,
					clients[i]->nickname,
					clients[i]->score,
					i == id ? indicator : "");
					
				strcat(buffer, tmp);
			}
		}
	} else {
		printf("Brak podłączonych klientów.\n");
	}
}

void send_scoreboard_client(client* player) {
	int scoreboard_size = MAX_LINE * MAX_CLIENTS * sizeof(char);
	char* scoreboard = (char*)malloc(scoreboard_size);
	
	get_scoreboard(scoreboard, player->id);
	socket_write(player->fd, scoreboard, scoreboard_size);
	free(scoreboard);
}

void send_scoreboard_game(game* this_game) {
	send_scoreboard_client(this_game->player);
	send_scoreboard_client(this_game->opponent);
}

/* clients */

void client_init(int client_socket, client* cl) {
	int i = 0;
	int index = find_index_for_client();
	
	clients[index] = cl;

	get_client_nickname(client_socket, cl->nickname);
	cl->id = index;
	cl->fd = client_socket;
	cl->score = 0;
	cl->idle = 1;
	cl->connected = 1;
	
	for(i = 0; i < MAX_CLIENTS; i++) {
		cl->results[i] = EMPTY;
	}
	
	for(i = 0; i < MAX_CLIENTS; i++) {
		if(i == index) continue;
			
		if(clients[i] != NULL) {
			cl->results[i] = NOT_PLAYED;
			clients[i]->results[index] = NOT_PLAYED;
		}
	}
}

int find_index_for_client() {
	int i = 0;
	
	for(i = 0; i < MAX_CLIENTS; i++) {
		if(clients[i] == NULL) {
			return i;
		}
	}
	
	return -1;
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

void handle_disconnected_client(game* this_game) {
	// this_game->player disconnected;
	
	int i = 0;
	int id = this_game->player->id;
	
	char buffer[MAX_LINE];
	memset(buffer, 0, MAX_LINE);
	snprintf(buffer, MAX_LINE, 
		"# Klient [%s] odłączył się - wygrana!\n", 
		this_game->player->nickname);
	
	socket_write(this_game->opponent->fd, buffer, sizeof(buffer));
	printf("%s", buffer);
	
	clients[id] = NULL;
	
	for(i = 0; i < MAX_CLIENTS; i++) {
		if(clients[i] != NULL) {
			clients[i]->results[id] = EMPTY;
		}
	}
					
	pthread_cancel(this_game->opponent_thread);
	this_game->opponent->idle = 1;
	this_game->opponent->score++;
	
	send_scoreboard_client(this_game->opponent);
}

void init_game_for_player(game* this_game, 
		client* player, client* opponent, 
		pthread_mutex_t *mutex, pthread_cond_t *cond,
		int* winner, int* indexes) {
	this_game->indexes = indexes;
	this_game->winner = winner;
	this_game->player = player;
	this_game->opponent = opponent;
	
	this_game->mutex = mutex;
	this_game->cond = cond;
}

void handle_finished_game(client* winner, client* loser) {
	char buffer[MAX_LINE];
	memset(buffer, 0, MAX_LINE);
	
	sprintf(buffer, 
		"# [Przegrana] Wygrał gracz: [%s].\n", winner->nickname);
	socket_write(loser->fd, buffer, MAX_LINE);
	memset(buffer, 0, sizeof(buffer));
	sprintf(buffer, 
		"# [Wygrana] Wygrałeś z: [%s].\n", loser->nickname);
	socket_write(winner->fd, buffer, MAX_LINE);
		
	clients[winner->id]->results[loser->id] = WON;
	clients[loser->id]->results[winner->id] = LOST;
	winner->score++;
}

/* others */

void info(int16_t port) {
	printf("------- Witaj w grze 'Wyścig szczurów'. --------\n");
	printf("- Przepisuj słowa szybciej, niż inni i wygraj! -\n");
	printf("------- Aby grać, użyj telnetu. Port: %d -----\n", port);
}

void author(void) {
	printf("--------- Autor: Grzegorz Czarnocki ---------\n");
	printf("---- Politechnika Warszawska, wydz. MiNI ----\n");
	printf("--------- Warszawa | maj 2017 r. (c) --------\n");
}

void menu() {
	printf("# Menu:\n");
	printf("A) O autorze\n");
	printf("I) Informacje\n");
	printf("M) Menu\n");
	printf("R) Ranking\n");
	printf("S) Ranking+\n");
}

void* user_input(void* arg) {
	int stdin = STDIN_FILENO;
	char buffer[1];
	
	fd_set base_rfds, rfds;
	FD_ZERO(&base_rfds);
	FD_SET(stdin, &base_rfds);

	int16_t port = *((int16_t*)arg);
	
	menu();
	
	while(1) {
		rfds = base_rfds;
		
		if(select(stdin + 1, &rfds, NULL, NULL, NULL) > 0) {		
			if(FD_ISSET(stdin, &rfds)) {
				while(read(stdin, buffer, sizeof(buffer)) > 0) {
					if(strcmp(buffer, "A") == 0) {
						author();
					} else if(strcmp(buffer, "I") == 0) {
						info(port);
					} else if(strcmp(buffer, "M") == 0) {
						menu();
					} else if(strcmp(buffer, "R") == 0) {
						print_scoreboard(-1);
					} else if(strcmp(buffer, "S") == 0) {
						print_detailed_scoreboard();
					}
				}
			}
		}
	}
}
