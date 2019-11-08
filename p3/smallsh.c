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

#define BUFFER 256
#define ARGSBUFFER 2048

// definitions for strings/supported commands
#define HOME "HOME"
#define EXIT "exit"
#define CD "cd"
#define STATUS "status"

// function declarations
void showStatus(int childExitInteger);
int getNumOfArgs(char* s);

// struct/enums
enum BOOL { 
  TRUE,
  FALSE
};

/* ****************************************************************************
 * Description:
 * catches/ignores signals sent by CTRL+C and CTRL+V
 * ***************************************************************************/
void catchSIGINT(int signo) {
}

void catchSIGTSTP(int signo) {
}

void catchTermination() {
  struct sigaction SIGINT_action = {{0}};
  struct sigaction SIGSTOP_action = {{0}};

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

/* ****************************************************************************
 * Description:
 * Displays exit status to shell
 * @param childExitInteger
 * ***************************************************************************/
void showStatus(int childExitInteger) {
  // process terminated normally
  if (WIFEXITED(childExitInteger)) {
    // print actual exit status
    printf("exit value %d\n", WEXITSTATUS(childExitInteger));
    // get exit status
  } 

  // process was terminated by a signal
  if (WIFSIGNALED(childExitInteger)) {
    // print terminating signal
    printf("terminated by signal %d\n", WTERMSIG(childExitInteger));
  }
}

/* ****************************************************************************
 * Description:
 * Parses string considering ' ' and returns number of segments of input
 * @param args
 * @param input
 * ***************************************************************************/
int parseInput(char** args, char* input) {
  int n = 0;    // number of segments in input;
  char* token = strtok(input, " \n");
  while(token != NULL) {
    // duplicate argument to args list
    args[n++] = strdup(token);    
    // update token
    token = strtok(NULL, " \n");
  }
  return n;
}

/* ****************************************************************************
 * Description:
 * runs shell program and executes user's commands, returns 0 if terminated
 * using exit command
 * @param args
 * @param n
 * ***************************************************************************/
int smallsh(char** args, int n) {
  int ret = -1;
  // check if there are any arguments
  if (args == NULL || n < 1) {
    return ret;
  }
  
  // variables
  enum BOOL isbkgrd = FALSE;  // boolean for is background process
  static int status;          // status code

// execute commands
  // check if background
  if (strcmp(args[n - 1], "&") == 0) {
    isbkgrd++;
    n--;
  }

  // exit command
  if (strcmp(input, EXIT) == 0) {
    return ++ret;
  }

  // cd command
  if (strcmp(args[0], CD) == 0) {
    if (n > 2) {
      // change to directory indicated by argument 
      chdir(args[1]);
    } else {
      // change to home directory
      chdir(getenv(HOME));
    }
    return ret;
  } 

  // status command
  if (strcmp(input, STATUS) == 0) {
    showStatus(status);
    return ret;
  } 

  // all other commands--fork()

}

void freeargs(char** args, int n) {
  int i = 0;
  for (; i < n; i++) {
    free(args[i]);
    args[i] = NULL;
  }
}

/* ****************************************************************************
 * main program
 * ***************************************************************************/
int main() {
  // catch signals 
  catchTermination();

  int process = -1; // process
  int fin = -1;     // file input
  int fout = -1;    // file output
  int stat = 0;     // background status

  int chars = -5;   // How many chars we entered

  // buffer allocated by getline() that holds our entered string + \n + \0
  char* input = NULL; 
  size_t buffer = 0; // Holds how large the allocated buffer is

  int exit = 0;

  // get command & run shell
  while(1) {   // from lecture notes
    // Get input from the user until valid not blank
    while(1) {
      printf(": ");

      // Get a line from the user
      chars = getline(&input, &buffer, stdin);
      if (chars == -1 || input[0] == '#')
        clearerr(stdin);
      else
        // Exit the loop - we've got input
        break; 
    }

    // Remove the trailing \n that getline adds
    input[strcspn(input, "\n")] = '\0';

    // parse command
    char** args = NULL;
    int nArgs = parseInput(args, input);

    // run shell
    int exit = smallsh(args, nArgs);

    // Free the memory allocated by getline() or else memory leak
    free(input);
    input = NULL;
    freeargs(args, nArgs);

    if (exit == 0) {
      exit(0);
    }
  }

  // kill processes
  //
  return 0;
}
