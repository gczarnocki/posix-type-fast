#include "common.h"
#include "common_task.h"

volatile sig_atomic_t do_work = 1;

void sigint_handler(int sig);
void usage(char* name);
void build_array(char* path);
void find_new_games();
void client_game_init(client_game* this_client_game, client* player, 
	client* opponent, pthread_mutex_t *mutex, pthread_cond_t *cond,
	int* winner, int* indexes);
void client_games_init(client_game* games, 
	pair* this_pair, pthread_t* threads, pthread_mutex_t* mutex, 
	pthread_cond_t* cond, int* winner, int* indexes);
void* handle_pair(void* arg);
void* single_client_game(void* arg);
void* client_handler(void* arg);
void do_server(int server_socket, int16_t port);

void sigint_handler(int sig) {
	do_work = 0;
}

void usage(char* name) {
	fprintf(stderr, "USAGE: %s port file_name\n", name);
}

void build_array(char* path) {
	// method used to build an array with words from text file
	
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

void find_new_games() {
	// method used to find a new game between two idle clients.
	// client  must be  connected and idle and there  must be a
	// "NOT_PLAYED" enum result to start a game between them.

	int i = 0, j = 0;
	
	for(; i < MAX_CLIENTS; i++) {
		if(clients[i] == NULL) continue;
		
		for(j = 0; j < MAX_CLIENTS; j++) {
			if(clients[j] != NULL &&
			clients[i]->connected && clients[j]->connected &&
			clients[i]->idle && clients[j]->idle &&
			clients[j]->results[i] == NOT_PLAYED) {				
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
				
				thread_create(&game_thread, NULL, handle_pair, new_pair);
			}
		}
	}
}

void client_game_init(client_game* this_client_game, 
		client* player, client* opponent, 
		pthread_mutex_t *mutex, pthread_cond_t *cond,
		int* winner, int* indexes) {
	this_client_game->indexes = indexes;
	this_client_game->winner = winner;
	this_client_game->player = player;
	this_client_game->opponent = opponent;
	
	this_client_game->mutex = mutex;
	this_client_game->cond = cond;
}

void client_games_init(client_game* games, pair* this_pair, 
		pthread_t* threads, pthread_mutex_t* mutex, pthread_cond_t* cond, 
		int* winner, int* indexes) {
	// method used to initialize structures for a game between clients.
	// every client in a game operates on a  different thread, has info
	// about its opponent, indexes  of words, and tries to set its' wi-
	// nner (int*) value to its' id,  protected by mutex and cond. var.
			
	client_game_init(&games[0], this_pair->client1, this_pair->client2, 
		mutex, cond, winner, indexes);
	client_game_init(&games[1], this_pair->client2, this_pair->client1, 
		mutex, cond, winner, indexes);
		
	thread_create(&threads[0], NULL, single_client_game, &games[0]);
	thread_create(&threads[1], NULL, single_client_game, &games[1]);
	games[0].player_thread = threads[0];
	games[1].player_thread = threads[1];
	games[0].opponent_thread = threads[1];
	games[1].opponent_thread = threads[0];
}

void* handle_pair(void* arg) {
	// thread function used to handle a game between two clients
	// in client_game_init, 2 threads are  created for 2 clients.
	// winner value  stands for winner's  identifier, players try
	// to set this value to their id's, prot'd by mutex and c. v.
	// after game end, we can start looking for new games again.
	
	int winner = -1;
	pair* this_pair = (pair*)arg;
	client* cl1 = this_pair->client1;
	client* cl2 = this_pair->client2;
	client_game client_games[2];
	pthread_t threads[2];
	
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	mutex_init(&mutex, NULL);
	cond_init(&cond, NULL);
	
	int* indexes = (int*)malloc(sizeof(int) * WORDS_GAME);
	find_indexes(indexes, WORDS_GAME, WORDS_CNT);
	
	client_games_init(client_games, this_pair, threads, 
		&mutex, &cond, &winner, indexes);
	
	pthread_cond_wait(&cond, &mutex);

	if(winner == cl1->id) {
		handle_finished_client_game(cl1, cl2);
		pthread_cancel(threads[1]);
	} else {
		handle_finished_client_game(cl2, cl1);
		pthread_cancel(threads[0]);
	}

	clients[cl1->id]->idle = 1;
	clients[cl2->id]->idle = 1;
	
	free(indexes);

	send_scoreboard_client_game(&client_games[0]);
	find_new_games();
	return thread_exit((void*)arg);
}

void* single_client_game(void* arg) {
	// create single game for player and play with opponent
	
	client_game* this_client_game = (client_game*)arg;
	int* indexes = this_client_game->indexes;
	char buffer[MAX_LINE];
	
	int i = 0, disconnected = 0;
	int fd = this_client_game->player->fd;
	char* op_nickname = this_client_game->opponent->nickname;
	
	fd_set base_rfds, rfds;
	FD_ZERO(&base_rfds);
	FD_SET(fd, &base_rfds);
	
	snprintf(buffer, MAX_LINE, "# Obecnie grasz z: [%s].\n", op_nickname);
	socket_write(fd, buffer, MAX_LINE);
	
	while(i < WORDS_GAME) {	
		rfds = base_rfds;
		
		send_word_to_client(fd, words[indexes[i]], i);
		
		if(pselect(fd + 1, &rfds, NULL, NULL, NULL, NULL) > 0) {
			if(FD_ISSET(fd, &rfds)) {
				int ret = recv_word_from_client(fd, words[indexes[i]]);
				
				if(ret > 0) {
					i++; // word was correctly written by client;
				} else if(ret == -1) {
					disconnected = 1;
				}
			}
		}
				
		if(disconnected) {
			handle_disconnected_client(this_client_game);
			find_new_games();
			return thread_exit((void*)arg);
		}
	}
	
	handle_winner(this_client_game);
	return thread_exit((void*)arg);
}

void* client_handler(void* arg) {
	// handle client by finding a game to play for him.
	
	int client_socket = *((int*)arg);
	client* cl = (client*)malloc(sizeof(client));
	client_init(client_socket, cl);
	
	printf("# [Id: %d | %s] Klient połączył się.\n", 
		cl->id, cl->nickname);
	
	find_new_games();
	return thread_exit((void*)arg);
}

void do_server(int server_socket, int16_t port) {
	int client_socket;
	
	fd_set base_rfds, rfds;
	sigset_t mask, oldmask;
	
	FD_ZERO(&base_rfds);
	FD_SET(server_socket, &base_rfds);
	
	sigemptyset(&mask);
	sigaddset(&mask, SIGINT);
	sigprocmask(SIG_BLOCK, &mask, &oldmask);
	
	pthread_t user_input_thread;
	thread_create(&user_input_thread, NULL, user_input, (void*)&port);
	
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
					thread_create(&client_thread, NULL, 
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
	do_server(server_socket, port);
	
	safe_close(server_socket);
	fprintf(stderr, "# Serwer zakończył działanie.\n");
	return EXIT_SUCCESS;
}
