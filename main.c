/******************************
* Author: Max Grier
* Course: CS344 - 400 - W2021
* Date Modified: 2-8-2021
* Due Date: 2-8-2021
* Description:  This program will create a shell in C.  It will work just like a shell
 *              you use regularly with a few other features.  It has 3 built in commands:
 *              exit, cd, and status.
* Source code credit: Parts of this code are either from, or based on,
*                     the code in the modules.  Generally this includes
 *                     code related to sigaction.
******************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "main_helpers.h"
#include "main_sigHandler.h"

/******************************
* main: This has the majority of the functionality of the program.
 *  It also has calls to functions from main_helpers.h and main_sigHandler.h
******************************/
int main(void){
    // Set the process id to pid
    pid_t pid = getpid();

    // Set up sig action for CTRL C and CTRL Z
    sig_action();

    // Keep the starting background signal
    int starting_background = run_background;

    // Continuously gather the command line inputs
    while(1){
        // See if we need to switch background/foreground
        if(starting_background != run_background){
            if (run_background == 1){
                printf("Exiting foreground only mode\n");
                fflush(stdout);
            } else{
                printf("Entering foreground only mode (& is now ignored)\n");
                fflush(stdout);
            }
        }
        // Reset the starting background
        starting_background = run_background;

        // Clear the previously entered arguments from memory
        free_arg_input();

        // Clear any path memory if the name is not /dev/null
        free_pathname();

        // Prompt the user for input
        prompt_user();

        // Print the child PID and how it exited
        check_child();

        // If the line is blank of commented, skip
        if(no_entry(cl_input) || cl_input[0] == '#'){
            continue;
        // Otherwise use the input and any commands
        }else{
            // Initialize spawn_pid
            pid_t spawn_pid = 0;

            // Use this to tell if & is used and if it should run in is_background
            int is_background = 0;

            // Token to parse out the input
            char* token = NULL;
            // Use delimiters space, tab, and new line to parse
            token = strtok(cl_input, " \t\n");

            int num_args = 0;

            // Parse the arguments out
            while(token != NULL){
                // Cycle through all arguments
                if(is_background == 1){
                    // If they used & to signal it should run in background
                    is_background = 0;
                    arg_input[num_args] = calloc(2, sizeof(char));
                    sprintf(arg_input[num_args], "%c", '&');
                    // Increment the number of arguments
                    num_args ++;
                }

                // Check if there is an input or output file
                if(strcmp(token, "<") == 0 || (strcmp(token, ">") == 0)){
                    if(strcmp(token, "<") == 0){
                        is_input_file = 1;
                    }else if((strcmp(token, ">") == 0)){
                        is_output_file = 1;
                    }
                }

                // Test for $$ and for &
                else if(strcmp(token, "$$") == 0 || (strcmp(token, "&") == 0)){
                    // Expand process ID for smallsh itself
                    if((strcmp(token, "$$") == 0)){
                        arg_input[num_args] = calloc(10, sizeof(char));
                        sprintf(arg_input[num_args], "%d", pid);
                        num_args++;
                    }else if((strcmp(token, "&") == 0)){
                        // If they wanted it to be run in is_background
                        is_background = 1;
                    }
                }
                // Check if there is an input or output file
                else if(is_input_file || is_output_file){
                    if(is_input_file){
                        // Set input file, if there is one
                        is_input_file = 0;
                        // Allocate memory to the path
                        input_path = calloc(MAX_LENGTH + 1, sizeof(char));
                        strcpy(input_path, token);
                    }else if(is_output_file){
                        // Set up output file, if there is one
                        is_output_file = 0;
                        // Allocate memory to the path
                        output_path = calloc(MAX_LENGTH + 1, sizeof(char));
                        strcpy(output_path, token);
                    }
                }else{
                    //For non built in commands
                    // Make memory for the arguments
                    arg_input[num_args] = calloc(MAX_LENGTH + 1, sizeof(char));
                    // Copy argument to the argument string
                    strcpy(arg_input[num_args], token);
                    // Increment the arguments
                    num_args++;
                }
                // Break out the string by the delimiters
                token = strtok(NULL, " \t\n");

            }
            // End of the while loop going through arguments
            // If there aren't any arguments, continue
            if(num_args == 0){
                continue;
            }
            // To not run in background
            if(run_background == 0){
                is_background = 0;
            }
            // If & was last, then the process will run in the background
            if(is_background == 1){
                // If no input path is selected
                if(input_path == NULL){
                    input_path = null_path;
                }
                // If no output path is selected
                if(output_path == NULL){
                    output_path = null_path;
                }
            }
            // If the input command is exit, then exit the program
            if(strcmp(arg_input[0], "exit") == 0){
                exit(0);
            }
            // If the command was status, run the status function
            else if(strcmp(arg_input[0], "status") == 0){
                status_cmd();
                continue;
            }
            // Change the directory
            else if(strcmp(arg_input[0], "cd") == 0){
                // If nothing after cd, change to home directory
                if(num_args == 1){
                    chdir(getenv("HOME"));
                }
                // If there is a directory specified, change to it
                else{
                    status = chdir(arg_input[1]);
                    // If the directory change errored out, print error
                    if(status == -1){
                        printf("%s: No such file or directory\n", arg_input[1]);
                        fflush(stdout);
                    }
                }
            }
            else {
                // Start to fork the children
                spawn_pid = fork();

                if (spawn_pid == -1) {
                    // The fork failed, print the error message and break
                    perror("fork() attempt failed");
                    fflush(stdout);
                    status = 1;
                    break;
                }
                if (spawn_pid == 0) {
                    // If the fork was successful
                    if (is_background == 0) {
                        // handle SIGINT if it is a foreground process
                        check_signit();
                    }
                    // Check if there is an error with sigststp
                    check_sigtstp();

                    // Open the file if one is specified
                    if (input_path != NULL) {
                        create_input();
                    }
                    // If there is an output file
                    if (output_path != NULL) {
                        create_output();
                    }

                    // If execvp fails, print an error saying it didn't work
                    if (execvp(arg_input[0], arg_input) == -1) {
                        printf("%s: ", arg_input[0]);
                        fflush(stdout);
                        perror("");
                        fflush(stdout);
                        exit(1);
                    }
                    exit(1);
                } else {
                    {
                        if (is_background == 0) {
                            check_status(spawn_pid);
                        } else {
                            // Otherwise, print the background PID
                            printf("background pid is %d\n", spawn_pid);
                        }
                    }
                }
            }
            }

        }
    return 0;
}
