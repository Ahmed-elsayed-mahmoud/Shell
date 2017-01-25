#include "shell.h"



void add_to_history(char inputBuffer[]) 
{
    int i = 0;
    if(command_count >= MAX_COMMANDS) {
        for(i=1; i< MAX_COMMANDS; i++) {
            strcpy( history[i - 1], history[ i ] );
            strcpy( display_history[i - 1], display_history[ i ] );
        }
        command_count--;
    }
    strcpy(history[command_count ], inputBuffer);
    i = 0;
    while (inputBuffer[i] != '\n' && inputBuffer[i] != '\0') {
        display_history[command_count ][i] = inputBuffer[i];
        i++;
    }
    display_history[command_count][i] = '\0';
    ++command_count;
    return;

}

void write_history_to_file() 
{
    FILE *f = fopen(HISTORY_FILE, "w");
    if (f == NULL)
    {
        printf("Error opening file!\nFile history.txt not found!\n");
        return;
    }
    int i = 0;
    for(; i < command_count; i++) {
        fputs(display_history[i], f);
        if(i != command_count) fputs("\n", f);
    }

    fclose(f);
}

void read_history_from_file() 
{
    FILE *f = fopen(HISTORY_FILE, "r");
    if (f == NULL)
    {
        printf("Error opening file!\nFile history.txt not found!\n");
        return;
    }
    int i = 0;
    char line[300];
    while(fgets ( line, MAX_LINE+200, f) != NULL && i < MAX_COMMANDS) {
        strcpy(history[i] , line);
        strcpy(display_history[i] , line);
        if(line [0] == '\n') continue;
        int len = strlen(display_history[i]);
        if (len > 0 && display_history[i][len-1] == '\n')
            display_history[i][len-1] = 0;
        i++;
        command_count++;
    }

    fclose(f);
}

void read_commands_from_file(char * fileName) 
{
    FILE *f = fopen(fileName, "r");
    if (f == NULL)
    {
        printf("Error opening file!\nFile %s not found!\n", fileName);
        return;
    }
    int i = 0;
    char line[300];
    while(fgets ( line, MAX_LINE+100, f) != NULL) {
        if(line [0] == '\n') continue;
        strcpy(commands[i] , line);

        i++;
        file_commands++;
    }

    fclose(f);
}

int find_paths() 
{
    int size = 0;
    char * path_string = getenv("PATH");
    char * token;
    token = strtok(path_string, ":");
    while(token != NULL) {
        paths[size++] = token;
        token = strtok(NULL, ":");
    }
    return size;
}

char * concat(char * s1, char * s2) 
{
    char * s3 = (char * ) malloc(strlen(s1) + strlen(s2) + 1);
    strcpy(s3, s1);
    strcat(s3, s2);
    return s3;
}

char * get_valid_path(int paths_size, char * args[]) 
{
    int i;

    for(i = 0; i<paths_size; i++) {
        char * path;
        path = concat(paths[i], "/");
        path = concat(path, args[0]);
        if(access(path, F_OK) != -1) {
            return path;
        }
    }
    return args[0];
}

void parse_command(char inputBuffer[],char *args[],int length,int *background)
{
    int i = 0, j = 0, c = 0, start_index = -1, args_index = 0, slash = 0;
    int start_quote = -1, end_qoute = -1;
    for (i=0; i<length; i++) {
        if(inputBuffer[i] == '"' || inputBuffer[i] == '\'') {
            if(start_quote == -1)
                start_quote = i;
            else if( end_qoute < i)
                end_qoute = i;
        }

    }
    for (i=0; i<length; i++) {
        if(start_quote != -1 && i == start_quote) {
            char * substr;
            if(start_index != -1) {
                substr = malloc(i - start_index + 1);
                strncpy(substr, &inputBuffer[start_index], i - start_index);
                substr[i - start_index + 1] = 0;
            }
            char *str2 = (char * ) malloc(MAX_LINE);
            for(j = i+1, c=0; j<end_qoute; j++) {
                if(inputBuffer[j] == '"' ) continue;
                if(inputBuffer[j] == '\\' && slash %2 == 0){
                    slash --;
                    continue;
                }
                else if(inputBuffer[j] == '\\'){
                    slash ++;
                }
                str2[c++] = inputBuffer[j];
            }
            str2[c] = '\0';
            i = j;

            args[args_index] = (start_index != -1)? concat(substr,str2): str2;
            args_index++;
            start_index = -1;
            continue;
        }
        switch (inputBuffer[i]) {
        case ' ':
        case '\t' :
            if(start_index != -1) {
                args[args_index] = &inputBuffer[start_index];
                args_index++;
            }
            inputBuffer[i] = '\0';
            start_index = -1;
            break;

        case '\n':
            if (start_index != -1) {
                args[args_index] = &inputBuffer[start_index];
                args_index++;
            }
            inputBuffer[i] = '\0';
            args[args_index] = NULL;
            break;
        default :
            if (start_index == -1)
                start_index = i;
            if (inputBuffer[i] == '&') {
                *background  = 1;
                inputBuffer[i-1] = '\0';
            }
        }
    }

    // If we get &, don't enter it in the args array
    if (*background)
        args[--args_index] = NULL;

    args[args_index] = NULL;
    return;
}

int get_next_command(char inputBuffer[],char *args[],int *background,int readMode)
{
    int length, command_number;

    if(readMode == INTERACTIVE_MODE) {
        do {
            printf("shell> ");
            fflush(stdout);
            length = read(STDIN_FILENO, inputBuffer, MAX_LINE + 300);
        }
        while (inputBuffer[0] == '\n');
    }
    else if(readMode == BATCH_MODE && file_command_index < file_commands) {
        printf("shell> ");
        fflush(stdout);
        strcpy(inputBuffer, commands[file_command_index]);
        printf("%s",commands[file_command_index]);
        length = strlen(commands[file_command_index]);
        file_command_index++;
    }
    else if(readMode == BATCH_MODE && file_command_index >= file_commands) {
        length = 0;
    }

    if (length == 0) { // ^d was entered, end of user command stream 
        write_history_to_file();
        printf("\n");
        exit(0);
    }
    if(length > MAX_LINE) {
        printf("The Command is very long (over 80).\n");
        return 2;
    }

    /**
     * the <control><d> signal interrupted the read system call
     * if the process is in the read() system call, read returns -1
     * However, if this occurs, errno is set to EINTR. We can check  
     * this  value and disregard the -1 value
     */
    if ( (length < 0) && (errno != EINTR) ) {
        perror("Error!! error reading the command");
        exit(-1);
    }

    if (inputBuffer[0] == '!') {
        if (command_count == 0) {
            printf("No Commands in the History.\n");
            return 2;
        }
        else if (inputBuffer[1] == '!') {

            strcpy(inputBuffer,history[command_count - 1]);
            length = strlen(inputBuffer) + 1;
        }
        else if (isdigit(inputBuffer[1])) {
            command_number = atoi( &inputBuffer[1] ) -1;
            if(command_number < command_count && command_number >= 0) {
                strcpy(inputBuffer, history[command_number]);
                length = strlen(inputBuffer) + 1;
            }
            else {
                printf("No such command in history.\n");
                return 2;
            }
        }
    }

    // Add the command to the history
    if (strncmp(inputBuffer,"history", 7) != 0 && 
		strncmp(inputBuffer, "exit", 4) != 0)
        add_to_history(inputBuffer);

    // Parse the contents of inputBuffer
    parse_command(inputBuffer, args, length, background );

    return 1;
}

/*
 * sigchld_handler - The kernel sends a SIGCHLD to the shell whenever
 * a child job terminates (becomes a zombie), or stops because it received
 * a SIGINT signal. The handler reaps all available zombie children.
 */
void signal_handler( int sig) 
{
    pid_t childpid;
    switch(sig) {
    case SIGINT:   // Ctrl + C
        write_history_to_file();
        printf("\n");
        exit(0);
        break;

    case SIGCHLD:

        //printf("Found SIGCHLD in signal handler\n");
        while ((childpid = waitpid(-1, NULL, WNOHANG)) > 0) {
            printf("Process [%d] Done \n",(int)childpid);
        }
       int i = kill(childpid, sig);
        if (i) {
            perror("kill");
        }
        if(errno != ECHILD){
            printf("Wait pid error!!\n");   
        }
        break;
    }
}
int main(int argc, char* argv[]) 
{

    char *args[MAX_LINE/2 + 1];
    char inputBuffer[MAX_LINE];
    char * fileName ;
    int should_run = 1, background, status;
    int i, start;
    pid_t child;

    if(argc > 1) {
        fileName = argv[1];
        read_commands_from_file(fileName);
    }
    // read history at the begining
    read_history_from_file();
    // find all paths to add args later
    int paths_size = find_paths();

    /* These are the ones you will need to implement 
    struct sigaction act, act_old;
    act.sa_handler = signal_handler;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);
    if(	(sigaction(SIGINT,  &act, &act_old) == -1) ||  // Ctrl^C
            (sigaction(SIGCHLD,  &act, &act_old) == -1)) {
        perror("signal");
        return 1;
    }
    */
    if(signal(SIGCHLD, signal_handler) == SIG_ERR || signal(SIGINT, signal_handler) == SIG_ERR){
        printf("signal error");
    }
    
    while (should_run) {
        background = 0;
        if(argc > 1 && argv[1] != NULL) {
            should_run = get_next_command(inputBuffer, args, &background
										, BATCH_MODE);
        }
        else {
            should_run = get_next_command(inputBuffer, args, &background
										, INTERACTIVE_MODE);
        }
        if (strncmp(inputBuffer, "exit", 4) == 0) {
            write_history_to_file();
            return 0;
        }
        else if (strncmp(inputBuffer,"history", 7) == 0) {
            if (command_count < MAX_COMMANDS){
                start = command_count;
            }
            else{
                start = MAX_COMMANDS;
            }
            for (i = start-1; i >= 0; i--) {
                printf("%d \t %s\n", i+1, display_history[i]);
            }
            continue;
        }
        if(should_run == 2) { // error handling
            should_run = 1;
            continue;
        }

        if (should_run) {
            child = fork();
            if(child == 0) { // child
                status = execv(get_valid_path(paths_size, args), args);
                if (status != 0) {
                    perror("Error in execting command !!  ");
                    exit(-2);
                }
            }
            else if(child == -1) { // error
                perror("Error!! could not fork the process");
                /* perror is a library routine that displays a system
                   error message, according to the value of the system
                   variable "errno" which will be set during a function
                   (like fork) that was unable to successfully
                   complete its task. */
            }
            else { // parent
                if (background == 0)  // handle parent, wait for child
                    while (child != wait(NULL)) ;
            }
        }
    }
    return 0;
}