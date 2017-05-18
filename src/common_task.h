#ifndef _COMMON_TASK_H
#define _COMMON_TASK_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <time.h>

#include "common.h"

#define MAX_NAME 20
#define MAX_LINE 80
#define WORDS_GAME 5
#define WORDS_CNT 40
#define MAX_CLIENTS 10
#define TIMEOUT_SECS 10
#define MAX_RETRY_CNT 3

typedef enum result { EMPTY = 0, NOT_PLAYED, PLAYING, WON, LOST } result;

typedef struct client {
	char nickname[MAX_NAME];
	result results[MAX_CLIENTS];
	int connected;
	int score;
	int idle;
	int id;
	int fd;
} client;

typedef struct pair {
	client* client1;
	client* client2;
} pair;

typedef struct game {
	int *winner;
	int *indexes;
	client *player;
	client *opponent;
	pthread_t player_thread;
	pthread_t opponent_thread;
	pthread_mutex_t *mutex;
	pthread_cond_t *cond;
} game;

char words[WORDS_CNT][MAX_LINE]; 	// global array with game words;
client* clients[MAX_CLIENTS]; 		// global array with client info;

void remove_new_line(char* str);
int are_words_equal(char* pattern, char* input);

int parse_arguments(int argc, char** argv, uint16_t* port);
void find_indexes(int* indexes, int array_size, int max_range);

int if_nickname_exists(char* name);
void get_client_nickname(int client_socket, char* nickname);

void print_scoreboard(int id);
void print_detailed_scoreboard();
void get_scoreboard(char* buffer, int id);
void send_scoreboard_client(client* player);
void send_scoreboard_game(game* this_game);

void client_init(int client_socket, client* cl);
int find_index_for_client();
int connected_clients_count();
void handle_disconnected_client(game* this_game);
void init_game_for_player(game* this_game, 
		client* player, client* opponent, 
		pthread_mutex_t *mutex, pthread_cond_t *cond,
		int* winner, int* indexes);
void handle_finished_game(client* winner, client* loser);

void find_new_games();

void info(int16_t port);
void author(void);
void menu();
void* user_input(void* arg);

#endif
