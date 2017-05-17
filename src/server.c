#include "common.h"
#include "common_task.h"

volatile sig_atomic_t do_work = 1;

void sigint_handler(int sig);
void usage(char* name);
void build_array(char* path);
void print_ranking();
void* user_input(void* arg);
int find_index_for_client();
void find_new_games();
void init_game_for_player(game* this_game, client* player, 
	client* opponent, pthread_mutex_t *mutex, pthread_cond_t *cond,
	int* winner, int* indexes);
void* single_game(void* arg);
void* handle_pair(void* arg);
void client_init(int client_socket, client* cl);
void* client_handler(void* arg);
void do_server(int server_socket);

void sigint_handler(int sig) {
	do_work = 0;
}

void usage(char* name) {
	fprintf(stderr, "USAGE: %s port file_name\n", name);
}

void build_array(char* path) {
	char data[MAX_LINE];

	FILE *fp;
	int i = 0;

	fp = fopen(path, "r");
	if(fp == NULL)
		EXIT("fopen");

	while(fgets(data, sizeof(data), fp) != NULL && i < WORDS_CNT) {
		sscanf(data, "%s", data);
		remove_new_line(data);
		strncpy(words[i], data, MAX_LINE);
		i++;
	}

	fclose(fp);
}

void* user_input(void* arg) {
	int stdin = STDIN_FILENO;
	char buffer[1];
	
	fd_set base_rfds, rfds;
	FD_ZERO(&base_rfds);
	FD_SET(stdin, &base_rfds);
	
	while(1) {
		printf("# Wpisz 'R', by zobaczyć ranking.\n");
		
		rfds = base_rfds;
		
		if(select(stdin + 1, &rfds, NULL, NULL, NULL) > 0) {		
			if(FD_ISSET(stdin, &rfds)) {
				while(read(stdin, buffer, sizeof(buffer)) > 0) {
					if(strcmp(buffer, "R") == 0) {
						print_scoreboard(-1);
					}
				}
			}
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

void find_new_games() {
	int i = 0, j = 0;
	
	for(; i < MAX_CLIENTS; i++) {
		if(clients[i] == NULL) continue;
		
		for(j = 0; j < MAX_CLIENTS; j++) {
			if(clients[j] != NULL &&
			clients[i]->connected == 1 &&
			clients[j]->connected == 1 &&
			clients[j]->results[i] == NOT_PLAYED &&
			clients[i]->idle == 1 &&
			clients[j]->idle == 1) {
				// let's start game between i and j;				
				clients[i]->idle = clients[j]->idle = 0;
				clients[i]->results[j] = PLAYING;
				clients[j]->results[i] = PLAYING;
				
				pthread_t game_thread;
				pair* new_pair = (pair*)malloc(sizeof(pair));
				new_pair->client1 = clients[i];
				new_pair->client2 = clients[j];
				
				printf("# Rozpoczęcie gry: [%s] <-> [%s].\n",
					new_pair->client1->nickname,
					new_pair->client2->nickname);
				create_thread(
					&game_thread, NULL, 
					handle_pair, new_pair);
			}
		}
	}
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

void print_to_socket(int fd, char* text) {
	int len = strlen(text) + 1;
	socket_write(fd, text, len);
}

void handle_disconnected_client(game* this_game) {
	// this_game->player disconnected;
	
	int i = 0;
	int id = this_game->player->id;
	
	char buffer[MAX_LINE];
	snprintf(buffer, MAX_LINE, 
		"# Klient [%s] odłączył się - wygrana!\n", 
		this_game->player->nickname);
	
	print_to_socket(this_game->opponent->fd, buffer);
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

void* single_game(void* arg) {
	game* this_game = (game*)arg;
	int* indexes = this_game->indexes;
	char buffer[MAX_LINE];
	int i = 0, timeouted = 1;
	
	int fd = this_game->player->fd;
	int id = this_game->player->id;
	
	struct timespec timeout;
	timeout.tv_sec = TIMEOUT_SECS;
	timeout.tv_nsec = 0;
	
	fd_set base_rfds, rfds;
	FD_ZERO(&base_rfds);
	FD_SET(fd, &base_rfds);
	
	snprintf(buffer, MAX_LINE, "# Obecnie grasz z: [%s].\n",
		this_game->opponent->nickname);
	socket_write(fd, buffer, MAX_LINE);
	
	while(i < WORDS_GAME) {	
		rfds = base_rfds;
		timeouted = 1;
		
		memset(buffer, 0, sizeof(buffer));
		snprintf(
			buffer, MAX_LINE, "# [%d/%d] %s: ", 
			i + 1, WORDS_GAME, words[indexes[i]]);
		socket_write(fd, buffer, MAX_LINE);
		
		if(pselect(fd + 1, &rfds, NULL, NULL, &timeout, NULL) > 0) {
			timeouted = 0;
			
			if(FD_ISSET(fd, &rfds)) {
				memset(buffer, 0, sizeof(buffer));
				int read = socket_read(fd, buffer, MAX_LINE);

				if(read == -1) {
					// client disconnected;
					handle_disconnected_client(this_game);
					find_new_games();
					return arg;
				}
				
				remove_new_line(buffer);
				if(are_words_equal(words[indexes[i]], buffer)) {
					i++;
				}
			}
		}
		
		if(timeouted) {
			strncpy(buffer, "\n# Hej, jesteś tam?\n", MAX_LINE);
			socket_write(fd, buffer, MAX_LINE);
		}
	}
	
	mutex_lock(this_game->mutex);
	cond_signal(this_game->cond);
	*(this_game->winner) = id;
	mutex_unlock(this_game->mutex);

	return arg;
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

void* handle_pair(void* arg) {
	int winner = -1;
	pair* this_pair = (pair*)arg;
	client* cl1 = this_pair->client1;
	client* cl2 = this_pair->client2;
	game this_game[2];
	pthread_t threads[2];
	
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	mutex_init(&mutex, NULL);
	cond_init(&cond, NULL);
	
	int* indexes = (int*)malloc(sizeof(int) * WORDS_GAME);
	find_indexes(indexes, WORDS_GAME, WORDS_CNT);
	
	init_game_for_player(&this_game[0], cl1, cl2, 
		&mutex, &cond, &winner, indexes);
	init_game_for_player(&this_game[1], cl2, cl1,
		&mutex, &cond, &winner, indexes);
	
	create_thread(&threads[0], NULL, single_game, &this_game[0]);
	create_thread(&threads[1], NULL, single_game, &this_game[1]);
	this_game[0].player_thread = threads[0];
	this_game[1].player_thread = threads[1];
	this_game[0].opponent_thread = threads[1];
	this_game[1].opponent_thread = threads[0];
	
	pthread_cond_wait(&cond, &mutex);

	if(winner == cl1->id) {
		handle_finished_game(cl1, cl2);
		pthread_cancel(threads[1]);
	} else {
		handle_finished_game(cl2, cl1);
		pthread_cancel(threads[0]);
	}

	clients[cl1->id]->idle = 1;
	clients[cl2->id]->idle = 1;
	
	free(indexes);
	
	send_scoreboard_game(this_game);
	find_new_games();
	pthread_exit((void*)arg);
}

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

void* client_handler(void* arg) {
	int client_socket = *((int*)arg);
	client* cl = (client*)malloc(sizeof(client));
	client_init(client_socket, cl);
	
	printf("# [Id: %d | %s] Klient połączył się.\n", 
		cl->id, cl->nickname);
	
	find_new_games();
	
	pthread_exit((void*)arg);
}

void do_server(int server_socket) {
	int client_socket;
	
	fd_set base_rfds, rfds;
	sigset_t mask, oldmask;
	
	FD_ZERO(&base_rfds);
	FD_SET(server_socket, &base_rfds);
	
	sigemptyset(&mask);
	sigaddset(&mask, SIGINT);
	sigprocmask(SIG_BLOCK, &mask, &oldmask);
	
	pthread_t user_input_thread;
	create_thread(&user_input_thread, NULL, user_input, NULL);
	
	while(do_work) {
		rfds = base_rfds;
		
		if(pselect(server_socket + 1, &rfds, 
				NULL, NULL, NULL, &oldmask) > 0) {		
			if(FD_ISSET(server_socket, &rfds)) {
				if((client_socket = accept_client(server_socket)) < 0) {
					if(errno == EINTR) continue;
					EXIT("accept");
				} else {
					pthread_t client_thread;
					create_thread(&client_thread, NULL, 
						client_handler, &client_socket);
				}
			}
		}
	}
	
	sigprocmask(SIG_UNBLOCK, &mask, NULL);
}

int main(int argc, char** argv) {
	int server_socket;
	uint16_t port;
	
	if(!parse_arguments(argc, argv, &port)) {
		usage(argv[0]);
		exit(EXIT_FAILURE);
	} else {
		build_array(argv[2]);
	}

	set_handler(SIG_IGN, SIGPIPE);
	set_handler(sigint_handler, SIGINT);

	server_socket = bind_inet_socket(port, SOCK_STREAM);
	set_nonblock(server_socket);
	
	info(port);
	do_server(server_socket);
	
	safe_close(server_socket);
	
	author();
	fprintf(stderr, "# Serwer zakończył działanie.\n");

	return EXIT_SUCCESS;
}
