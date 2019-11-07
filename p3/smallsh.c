/* ****************************************************************************
 * Name:    Jenny Huang
 * Date:    November 4, 2019
 * Description:  
 * Program that supports three built-in commands: exit, cd, and status
 * Input/output redirection is not supported for these built in commands
 *
 * exit: exits shell and kills any other processes or jobs that the shell has
 * has started before termination
 *
 * cd: changes the working directory of the shell. changes to directory
 * specified in the HOME environment variable when no argument is presented.
 * also supports one argument--the path of a directory to change to,
 * supports absolute and relative paths. when shell is terminated, the 
 * original shell it was launched from will be in its original working directory
 *
 * status: prints out the exit status or terminating signal of the last 
 * foreground process
 * ***************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#define BUFFER 512

void catchSIGINT(int signo) {
  char* message = "SIGINT. Use CTRL-Z to Stop.\n";
  write(STDOUT_FILENO, message, 28);
}

void catchSIGTSTP(int signo) {
  char* message = "\n";
  write(STDOUT_FILENO, message, 28);
}

void catchTermination() {
  struct sigaction SIGINT_action = {0};
  struct sigaction SIGSTOP_action = {0};

  // catch CTRL+C
  SIGINT_action.sa_handler = SIG_IGN;
  sigfillset(&SIGINT_action.sa_mask);
  SIGINT_action.sa_flags = SA_RESTART;
  sigaction(SIGINT, &SIGINT_action, NULL);

  // catch CTRL+Z
  SIGSTOP_action.sa_handler = catchSIGTSTP;
  sigfillset(&SIGSTOP_action.sa_mask);
  SIGSTOP_action.sa_flags = SA_RESTART;
  sigaction(SIGTSTP, &SIGSTOP_action, NULL);
}

int main() {
  catchTermination();

  int process = -1; // process
  int fin = -1;   // file input
  int fout = -1;  // file output
  int stat = 0;   // background status
  char input[BUFFER];
  memset(input, '\0', sizeof(input));

  char* args[BUFFER];
  int isbg = 0; // boolean for is background process

  int numCharsEntered = -5; // How many chars we entered
  int currChar = -5; // Tracks where we are when we print out every char
  size_t bufferSize = 0; // Holds how large the allocated buffer is
  char* lineEntered = NULL; // Points to a buffer allocated by getline() that holds our entered string + \n + \0

  int exit = 0;
  while(exit < 1) {
    // Get input from the user
    while(1) {
      printf("Enter in a line of text (CTRL-C to exit):");
      // Get a line from the user
      numCharsEntered = getline(&lineEntered, &bufferSize, stdin);
      if (numCharsEntered == -1)
        clearerr(stdin);
      else
        break; // Exit the loop - we've got input
    }

    printf("Allocated %zu bytes for the %d chars you entered.\n",
        bufferSize, numCharsEntered);
    printf("Here is the raw entered line: \"%s\"\n", lineEntered);
    // Remove the trailing \n that getline adds
    lineEntered[strcspn(lineEntered, "\n")] = '\0';
    printf("Here is the cleaned line: \"%s\"\n", lineEntered);
    // Free the memory allocated by getline() or else memory leak
    //
    if (strcmp(lineEntered, "exit") == 0) {
      exit++;
    }
    free(lineEntered);
    lineEntered = NULL;
  }
  return 0;
}
