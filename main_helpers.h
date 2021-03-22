#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>

// Define the constants
#define MAX_LENGTH 2048
#define MAX_ARGUMENTS 512

// The prompt that will be displayed to the user
char line_prompt[] = ": ";
char *user_input = NULL;
size_t n = 0;

// Set maximum size of the command line input
char cl_input[MAX_LENGTH + 1] = {0};
// Set array for the arguments inputted
char* arg_input[MAX_ARGUMENTS + 2] = {0};

// Set default I/O files to NULL
char* input_path = NULL;
char* output_path = NULL;
char null_path[] = "/dev/null";

// Set the basic things needed for inputs
int status = 0;

// Use these to check if there are files expected
int is_input_file = 0;
int is_output_file = 0;


/******************************
* prompt_user: Simple function to send prompt to terminal
******************************/
char* prompt_user(void){
    // Prompt the user for input
    printf("%s",line_prompt);
    fflush(stdout);

    // Ge the user's input
    getline(&user_input, &n, stdin);
    strcpy(cl_input, user_input);

    return cl_input;
}


/******************************
* free_args: This will free the memory of the arguments inputted
 * it will also clear the command
******************************/
void free_arg_input(void){
    memset(cl_input,0,sizeof(cl_input));
    for(int i=0; i<514 && arg_input[i] != NULL; i++){
        free(arg_input[i]);
        arg_input[i] = NULL;
    }
}

/******************************
* free_pathname: This will free the memory of the input_path variable
 * if it is not /dev/null
******************************/
void free_pathname(void){
    if(input_path){
        // Check if it is /dev/null
        if(strcmp(null_path, input_path) != 0){
            free(input_path);
        }
        // Otherwise, make NULL
        input_path = NULL;
    }
    // Do the same for the output
    if(output_path){
        if(strcmp(null_path, output_path) != 0){
            free(output_path);
        }
        output_path = NULL;
    }
}

/******************************
* check_child: This will loop through the child processes
 * and print the PID and if it exited normally or by signal
******************************/
// Variables needed for this function, but also globally in main
// Set the Child PID
int c_id = 0;
// Set the Child Status
int c_stat = 0;

void check_child(void){
    while((c_id = waitpid(-1, &c_stat, WNOHANG)) > 0){
        // If it terminated normally
        if(WIFEXITED(c_stat)){
            printf("background pid %d is done: exit value %d\n", c_id, WEXITSTATUS(c_stat));
            fflush(stdout);
        // If it was terminated due to a signal
        }else if(WIFSIGNALED(c_stat)){
            printf("background pid %d is done: terminated by signal %d\n", c_id, WTERMSIG(c_stat));
            fflush(stdout);
        }
    }
}


/******************************
* noEntry: This function will check if there was anything entered
 * or if it was just an empty line.
******************************/
int no_entry(char* input){
    // Cycle through the inputs to check for spaces, new lines, and tabs
    while(input != NULL){
        // If there isn't a blank line return 0
        if(*input != ' ' && *input != '\n' && *input != '\t'){
            return 0;
        }
        // Move to the next input
        input ++;
    }
    // Otherwise return 1 if there is a blank
    return 1;
}

// Set up variables to run process in either
// the foreground or background
int run_foreground = 0;
int run_background = 1;

/******************************
* switch_ground: This function will switch a process from running
 * in either the foreground or background.
******************************/
void switch_ground(int process){
    // Set stdin and stdout for foreground
    if(run_foreground == 1){
        putc('\n', stdin);
        putc('\n', stdout);
    }
    // Switch background
    if(run_background == 0){
        run_background = 1;
    }else{
        run_background = 0;
    }
}


/******************************
* create_output: This function will create the output file
 * if there is one designated.
******************************/
void create_output(void){
    int fl_desc = 0;

    // If file path is /dev/null
    if (strcmp(output_path, null_path) != 0) {
        fl_desc = open(output_path, O_WRONLY | O_TRUNC | O_CREAT, 0666);
    } else {
        // Otherwise, just open with write only
        fl_desc = open(output_path, O_WRONLY);
    }

    // If the file descriptor failed
    if (fl_desc == -1) {
        perror("Stdout failed to open output");
        fflush(stdout);
        exit(1);
    }

    // Use dup2 to duplicate to stdout
    if (dup2(fl_desc, STDOUT_FILENO) == -1) {
        // If dup2 fails, print error
        perror("Stdout failed to redirect output");
        fflush(stdout);
        exit(1);
    }
    close(fl_desc);
}

/******************************
* create_input: This function will create the input file
 * if there is one designated.
******************************/
void create_input(void){
    int fl_desc = open(input_path, O_RDONLY);

    // If opening failed
    if (fl_desc == -1) {
        printf("cannot open %s for input: ", input_path);
        fflush(stdout);
        perror("");
        fflush(stdout);
        exit(1);
    }

    // Duplicate the file with dup2 to stdin
    if (dup2(fl_desc, STDIN_FILENO) == -1) {
        perror("Stdin failed to redirect input");
        fflush(stdout);
        exit(1);
    }
    //close the file descriptor
    close(fl_desc);
}
