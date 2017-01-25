#ifndef SHELL_H
#define SHELL_H

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#define MAX_LINE		 80
#define MAX_COMMANDS	 10
#define INTERACTIVE_MODE 0
#define BATCH_MODE       1
#define MAX_PATHS        1000
#define HISTORY_FILE     "history.txt"

char history[MAX_COMMANDS][MAX_LINE];
char display_history [MAX_COMMANDS][MAX_LINE];
char commands[300][MAX_LINE];
char * paths[MAX_PATHS];
int command_count = 0, file_commands = 0, file_command_index=0;


void add_to_history(char inputBuffer[]) ;
void write_history_to_file() ;
void read_history_from_file();
void read_commands_from_file(char * fileName);
int find_paths();
char * concat(char * s1, char * s2) ;
char * get_valid_path(int paths_size, char * args[]) ;
void parse_command(char inputBuffer[],char *args[],int length,int *background);
int get_next_command(char inputBuffer[],char *args[],int *background,int readMode);
void signal_handler(int sig) ;


#endif // SHELL_H

