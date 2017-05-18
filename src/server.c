#include "common.h"
#include "common_task.h"

volatile sig_atomic_t do_work = 1;

void sigint_handler(int sig);
void usage(char* name);
void build_array(char* path);
void* user_input(void* arg);
void find_new_games();
void* single_game(void* arg);
void* handle_pair(void* arg);
void* client_handler(void* arg);
void do_server(int server_socket, int16_t port);

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

void* single_game(void* arg) {
	game* this_game = (game*)arg;
	int* indexes = this_game->indexes;
	char buffer[MAX_LINE];
	
	int i = 0, timed_out = 1;
	int retry_cnt = 0, disconnected = 0;
	
	int fd = this_game->player->fd;
	int id = this_game->player->id;
	
	struct timespec timeout;
	timeout.tv_sec = TIMEOUT_SECS;
	timeout.tv_nsec = 0;
	
	fd_set base_wfds, wfds;
	FD_ZERO(&base_wfds);
	FD_SET(fd, &base_wfds);
	
	snprintf(buffer, MAX_LINE, "# Obecnie grasz z: [%s].\n",
		this_game->opponent->nickname);
	socket_write(fd, buffer, MAX_LINE);
	
	while(i < WORDS_GAME) {	
		wfds = base_wfds;
		timed_out = 1;
		
		memset(buffer, 0, sizeof(buffer));
		snprintf(
			buffer, MAX_LINE, "# [%d/%d] %s: ", 
			i + 1, WORDS_GAME, words[indexes[i]]);
		socket_write(fd, buffer, MAX_LINE);
		
		if(pselect(fd + 1, NULL, &wfds, NULL, &timeout, NULL) > 0) {
			timed_out = 0;
			
			if(FD_ISSET(fd, &wfds)) {
				memset(buffer, 0, sizeof(buffer));
				int read = socket_read(fd, buffer, MAX_LINE);

				if(read == -1) {
					// client disconnected;
					disconnected = 1;
				} else {
					remove_new_line(buffer);
					if(are_words_equal(words[indexes[i]], buffer)) {
						i++;
					} 
				}
			}
		}
		
		if(timed_out) {
			strncpy(buffer, "\n# Hej, jesteś tam?\n", MAX_LINE);
			socket_write(fd, buffer, MAX_LINE);
			
			retry_cnt++;
			printf("# Ponowienie połączenia [%d/%d]: [%s]\n", 
				retry_cnt, MAX_RETRY_CNT,
				this_game->player->nickname);
				
			if(retry_cnt == MAX_RETRY_CNT) {
				disconnected = 1;
			}
		}
				
		if(disconnected) {
			handle_disconnected_client(this_game);
			find_new_games();
			return arg;
		}
	}
	
	mutex_lock(this_game->mutex);
	cond_signal(this_game->cond);
	*(this_game->winner) = id;
	mutex_unlock(this_game->mutex);

	return arg;
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

void* client_handler(void* arg) {
	int client_socket = *((int*)arg);
	client* cl = (client*)malloc(sizeof(client));
	client_init(client_socket, cl);
	
	printf("# [Id: %d | %s] Klient połączył się.\n", 
		cl->id, cl->nickname);
	
	find_new_games();
	
	pthread_exit((void*)arg);
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
	create_thread(&user_input_thread, NULL, user_input, (void*)&port);
	
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
	do_server(server_socket, port);
	
	safe_close(server_socket);
	
	fprintf(stderr, "# Serwer zakończył działanie.\n");

	return EXIT_SUCCESS;
}
