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

/* ****************************************************************************
 * definitions for strings/supported commands
 * ***************************************************************************/
#define HOME "HOME"
#define EXIT "exit"
#define CD "cd"
#define STATUS "status"

/* ****************************************************************************
 * function declarations
 * ***************************************************************************/
// signal control
void catchSIGINT(int signo);
void catchSIGTSTP(int signo);
void catchSignal();

int getNumOfArgs(char* s);

// status cmd
void showStatus(int childExitInteger);
void exitstatus(int childExitInteger);
void signalstatus(int childExitInteger);

// file directory
void _chfile(char** args, int n);

// memory control
void freeargs(char** args, int n) {

/* ****************************************************************************
 * struct/enum definitions
 * ***************************************************************************/

/* ****************************************************************************
 * Description:
 * catches/ignores signals sent by CTRL+C and CTRL+V
 * ***************************************************************************/
// catch CTRL+C
void catchSIGINT(int signo) {
}

// catch CTRL+Z (foreground only)
void catchSIGTSTP(int signo) {
  printf("Exiting foreground-only mode\n");
}

void catchSignal() {
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
 * Displays exit/signaled status to shell
 * @param childExitInteger
 * ***************************************************************************/
void showStatus(int childExitInteger) {
  // process terminated normally
  exitstatus(childExitInteger);

  // process was terminated by a signal
  signalstatus(childExitInteger);
}

// prints status if exited normally
void exitstatus(int childExitInteger) {
  if (WIFEXITED(childExitInteger)) {
    // print actual exit status
    printf("exit value %d\n", WEXITSTATUS(childExitInteger));
    // get exit status
  } 
}
//prints status if termination by signal
void signalstatus(int childExitInteger) {
  if (WIFSIGNALED(childExitInteger)) {
    // print terminating signal
    printf("terminated by signal %d\n", WTERMSIG(childExitInteger));
  }
}

/* ****************************************************************************
 * Description:
 * change directory
 * @param args
 * @param input
 * ***************************************************************************/
void _chdir(char** args, int n) {
    if (n > 2) {
      // change to directory indicated by argument 
      chdir(args[1]);
    } else {
      // change to home directory
      chdir(getenv(HOME));
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
 * creates fork for all other commands, returns 1 if error occurs and -1
 * if exit not reached
 * @param bkgd
 * @param status
 * ***************************************************************************/
int _fork(bool& bkgd, int& status) {
  // create a fork
  pid_t pid = fork();

  // check if child is created, exit 1 if error
  if (pid < 0) {
    perror("error: unable to create child\n"); 
    return 1;
  }

  // curr is child process
  if (pid == 0) {
    // terminate if on foreground
    if (!bkgrd) {
      SIGINT_action.sa_handler = SIG_DFL;
      sigaction(SIGINT, &SIGINT_action, NULL);
    }

    execvp(args[0], n);
    perror("error: command not found\n");
    return 1;
  }

  // curr is parent process
  if (bkgrd) {
    // print background pid
    printf("background pid is %d\n", pid);
  } else {
     waitpid(pid, &status, 0);
     // check if signal terminated process
     signalstatus(status);

     // check for background processes
  }
  return -1;
}

/* ****************************************************************************
 * Description:
 * runs shell program and executes user's commands, returns exit code:
 * -1 if exit not reached
 * 0 if terminated using exit command
 * 1 if error occurs
 * @param args
 * @param n
 * ***************************************************************************/
int smallsh(char** args, int n) {
  int ret = -1;
  // check for invalid arguments
  if (args == NULL || n < 1) {
    return ret;
  }
  
  // variables
  bool bkgd = false;  // boolean for is background process
  static int status;          // status code

// execute commands
  // check if background
  if (strcmp(args[n - 1], "&") == 0) {
    bkgrd = TRUE;
    n--;
  }

  // exit command
  if (strcmp(input, EXIT) == 0) {
    return 0;
  }

  // cd command
  int cd_cmd = strcmp(args[0], CD) == 0;
  if (cd_cmd) {
    _chdir(args, n);
  } 

  // status command
  int status_cmd = strcmp(args[0], STATUS) == 0;
  if (status_cmd) {
    showStatus(status);
  } 
  
  // return if supported commands entered
  if (cd_cmd || status_cmd) { return ret; }

  // all other commands induces fork()
  return _fork(bkgd, status);
}


/* ****************************************************************************
 * Description: frees args 
 * @param args
 * @param n
 * ***************************************************************************/
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
  catchSignal();

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
    int exitCode = smallsh(args, nArgs);

    // Free the memory allocated by getline() or else memory leak
    free(input);
    input = NULL;
    freeargs(args, nArgs);

    // exit if smallsh induced an exit
    if (exitCode > -1) {
      exit(exitCode);
    }
  }

  // kill processes
  //
  return 0;
}
