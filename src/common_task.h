#ifndef _COMMON_TASK_H
#define _COMMON_TASK_H

void info(void);
void author(void);
char* get_nickname();
void read_file(char* path, char** words, int words_cnt);
char** build_array(char* path, int words_cnt);
char** get_random_words(char** words, int words_cnt, int n);

#endif
