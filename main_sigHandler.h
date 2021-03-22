#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>


/******************************
* Set up the sigaction structs
******************************/

// Initialize SIGINT_action struct to be empty
struct sigaction SIGINT_action = {0};
// Ignore signal set up
struct sigaction IGNORE_action = {
        .sa_handler = SIG_IGN
};
// Set up stop handler for CTRL Z
struct sigaction STOP_action = {
        .sa_handler = switch_ground
};

/******************************
* check_signit: This will check if there was an error with sigint action
******************************/
void check_signit(void){
    if (sigaction(SIGINT, &SIGINT_action, NULL) != 0) {
        // Print error if it fails
        perror("sigaction error");
        fflush(stdout);
        exit(1);
    }
}

/******************************
* check_sigtstp: This will check if there was an error with the ignore action
******************************/
void check_sigtstp(void){
    if (sigaction(SIGTSTP, &IGNORE_action, NULL) != 0) {
        // Print error if it fails
        perror("sigtstp error");
        fflush(stdout);
        exit(1);
    }
}

/******************************
* status_cmd: This will be the built in function for the status command
******************************/
void status_cmd(void){
    // If the process exited normally
    if(WIFEXITED(status)){
        printf("Exit value %d\n", WEXITSTATUS(status));
        fflush(stdout);
    }
    // If the process was terminated
    if(WIFSIGNALED(status)){
        printf("terminated by signal %d\n", WTERMSIG(status));
        fflush(stdout);
    }
}

/******************************
* check_status: This will see if the process was terminated of
 * if it ended normally.
******************************/
void check_status(int spawn_pid){
    // Return PID
    int return_pid;
    // As long as the return_pid is equal to spawn_pid
    do {
        return_pid = waitpid(spawn_pid, &status, 0);
        // Print the terminating signal if there is one, then break
        if (WIFSIGNALED(status)) {
            printf("\nterminated by signal %d\n", WTERMSIG(status));
            break;
        }
        // If the child ended normally, break
        if (WIFEXITED(status)) {
            break;
        }
    } while (return_pid == spawn_pid);
}

/******************************
* sig_action:  Set up the controls to ignore CTRL C and CTRL Z
******************************/
void sig_action(void){
    // Set up to ignore the SIGINT or Control + C
    if(sigaction(SIGINT, &IGNORE_action, &SIGINT_action) != 0){
        perror("sigaction sigint error");
        fflush(stdout);
        exit(1);
    }
    // Set up to ignore SIGTSTP or Control + Z
    if(sigaction(SIGTSTP, &STOP_action, NULL) != 0){
        perror("sigaction sigtstp error");
        fflush(stdout);
        exit(1);
    }
}