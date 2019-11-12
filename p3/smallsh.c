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
#include <ctype.h>
#include <sys/wait.h>
#include <fcntl.h>

#define BUFFER 256
#define ARGBUFF 32 

/* ****************************************************************************
 * struct/enum definitions
 * ***************************************************************************/
enum _bool {
  _true,
  _false
};

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
struct sigaction catch_sigIGN();
struct sigaction catch_sigSTGTSTP();
void catchSignal();

// smallsh prog
void _runshell(char** args, int n);
char* getcmd();
char** parseInput(int* n, char* input);
void _fork(char** args, int n, enum _bool bkgd, int* status);

// status cmd
void showStatus(int childExitInteger);
void exitstatus(int childExitInteger);
void signalstatus(int childExitInteger);

// file directory
void _chfile(char** args, int n);

// memory control
void freeargs(char** args, int n);

// debug
void printargs(char** args);

/* ****************************************************************************
 * Description:
 * catches/ignores signals sent by CTRL+C and CTRL+V
 * ***************************************************************************/
void catchSignal() {
  catch_sigIGN();
  catch_sigSTGTSTP();
}

// catcher for CTRL+Z
struct sigaction catch_sigSTGTSTP() {
  static struct sigaction SIGSTOP_action = {{0}};
  SIGSTOP_action.sa_handler = catchSIGTSTP;
  sigfillset(&SIGSTOP_action.sa_mask);
  SIGSTOP_action.sa_flags = SA_RESTART;
  sigaction(SIGTSTP, &SIGSTOP_action, NULL);
  return SIGSTOP_action;
}

// catcher for CTRL+C
struct sigaction catch_sigIGN() {
  static struct sigaction SIGINT_action = {{0}};
  SIGINT_action.sa_handler = SIG_IGN;
  sigfillset(&SIGINT_action.sa_mask);
  SIGINT_action.sa_flags = SA_RESTART;
  sigaction(SIGINT, &SIGINT_action, NULL);
  return SIGINT_action;
}

// catch CTRL+C
void catchSIGINT(int signo) {
}

// catch CTRL+Z (foreground only)
void catchSIGTSTP(int signo) {
  printf("\nExiting foreground-only mode\n");
}


/* ****************************************************************************
 * Description:
 * Displays exit/signaled status to shell
 * @param childExitInteger
 * ***************************************************************************/
void showStatus(int childExitInteger) {
  // process terminated normally
  exitstatus(childExitInteger);
  // process terminated by a signal
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
  int ch;
  if (n > 1) {
    // change to directory indicated by argument 
    ch = chdir(args[1]);
  } else {
    // change to home directory
    ch = chdir(getenv(HOME));
  }
  // error changing directory
  if (ch == -1) {
    perror("error: unable to access directory");
  }
}

/* ****************************************************************************
 * Description: [reference: lecture notes]
 * repeatedly gets input from user until valid and stores it
 * @param input
 * ***************************************************************************/
char* getcmd() {
  char* input = NULL;
  size_t buffer = 0; // Holds how large the allocated buffer is
  int chars = -5;   // number of chars entered
  while(1) {
    printf(": ");

    // Get a line from the user
    chars = getline(&input, &buffer, stdin); // result of reading line

    // check if line was read and if leading w/ # (comment)
    if (chars == -1 || input[0] == '#') {
      clearerr(stdin);
    } else
      // Exit the loop - we've got input
      break; 
  }
  // Remove the trailing \n that getline adds
  input[strcspn(input, "\n")] = '\0';

  return input;
}

/* ****************************************************************************
 * Description:
 * Parses string considering ' ' and returns segments input
 * and stores number of segments
 * @param input
 * ***************************************************************************/
char** parseInput(int* nArgs, char* input) {
  // get segments
  int n = 0;    // idx for segments in input;
  char** args = malloc(sizeof(char*) * ARGBUFF);
  char* token = strtok(input, " \n");
  while(token != NULL) {
    // duplicate argument to args list
    char* curr = malloc(sizeof(char) * BUFFER);
    sprintf(curr, "%s", token);
    args[n++] = curr;

    // update token
    token = strtok(NULL, " \n");
  }

  *nArgs = n;
  // printargs(args);
  return args;
}

char* fileDirection(char** args, int n, char* dir) {
  int i = 0;
  for (; i < n; i++) {
    if ((strcmp(args[i], "<") == 0 || strcmp(args[i], ">") == 0) && 
        (i + 1) < n) {
      strcpy(dir, args[i]);
      return args[i+1];
    }
  }

  return NULL;
}

/* ****************************************************************************
 * Description:
 * handles file redirection in foreground vs background
 * redirects to indicated file; 
 * note: '<' indicates input (read)
 *     : '>' indicates output (write)
 * returns 1 if error occurs
 * returns -1 if exit not reached
 * @param filename
 * @param dir
 * @param bkgd
 * ***************************************************************************/
void fileDirInBack() {
  int file = -5;
  // attempt to read file, attempt to duplicate, return error if occurs
  if ((file = open("/dev/null", O_RDONLY)) == -1 || 
      dup2(file, STDIN_FILENO) == -1 || dup2(file, STDOUT_FILENO) == -1) {
    perror("error: unable to redirect");
    exit(1);   // indicates exit 1
  }
}

void fileDirInFore(char* filename, char* dir) {
  int file;
  if (strcmp(dir, "<") == 0) {    // file input
    // attempt to open read file
    file = open(filename, O_RDONLY);
    if (file == -1) {
      printf("cannot open %s for input\n", filename);
      exit(1);
    }
    // unable to create duplicate
    if (dup2(file, 0) == -1 ) {
      perror("error: unable to redirect");
      exit(1);
    }
  } else if (strcmp(dir, ">") == 0) { // file output
    // attempt to open write file
    file = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    // unable to open
    if (file == -1) {
      printf("cannot open %s for output\n", filename);
      exit(1);
    }
    // unable to create duplicate
    if (dup2(file, 1) == -1) {
      perror("error: unable to redirect"); 
      exit(1);
    }
  }
  // close file
  if (file != -1) fcntl(file, F_SETFD, FD_CLOEXEC);
}

/* ****************************************************************************
 * Description:
 * creates fork for all other commands, returns exit code-- -1 if error occurs 
 * and -1 if exit not reached
 * @param args
 * @param n
 * @param bkgd
 * @param status
 * ***************************************************************************/
void _fork(char** args, int n, enum _bool bkgd, int* childExitStatus) {
  // signal catcher
  struct sigaction _action = catch_sigIGN();

  // create a fork
  pid_t pid = fork();
  switch(pid) {
    case -1:    // check if error creating child
      perror("error: unable to create child\n");
      exit(1); // exit 1 if error
      break;

    case 0:     // curr is child process
      // stop as foreground process
      if (bkgd == _false) {
        _action.sa_handler = SIG_DFL;
        sigaction(SIGINT, &_action, NULL);
      }

      // get filename if process redirects directory
      char dir[2] ;   // ">" or "<"
      char* filename = fileDirection(args, n, dir);
      if (filename != NULL) {
        // handle redirection
        if (bkgd == _false) {       // redirection in foreground
          fileDirInFore(filename, dir);
        } else if (bkgd == _true) { // redirection in background
          fileDirInBack();
        }
        // remove remaining args prior to running execvp
        int i = 1; for(; i < n; i++) {
          free(args[i]);
          args[i] = NULL;
        }
      }

      // attempt to execute command, print error if occurs
      // printargs(args);
      int exec = execvp(args[0], args);
      if (exec == -1) {
        perror("error: invalid command");
        fflush(stdout);
        exit(1);
      }
      break;

    default:    // curr is parent process
      if (bkgd == _true) {          // on background
        waitpid(pid, childExitStatus, WNOHANG);
        // print background pid
        printf("background pid is %d\n", (int) pid);
        fflush(stdout);
      } else if (bkgd == _false) {  // on foreground
        // wait for process to finish
        waitpid(pid, childExitStatus, 0);
        // check if signal terminated process
        signalstatus(*childExitStatus);
      }
  // background processes
  while ((pid = waitpid(-1, childExitStatus, WNOHANG)) > 0) {
    printf("background process %d is done\n", (int) pid);
    showStatus(*childExitStatus);
  }
      break;
  }
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
void _runshell(char** args, int n) {
  // check for invalid arguments
  if (args == NULL || n < 1) {
    return;
  }

  enum _bool bkgd = _false; // track if background process
  static int status = 0;    // status code

  // check if background
  if (strcmp(args[n - 1], "&") == 0) {
    bkgd = _true;
    free(args[n - 1]);
    args[--n] = NULL;
  }

  // execute supported commands
  char* cmd = args[0];

  // exit command
  if (strcmp(cmd, EXIT) == 0) {
    exit(0);
  }

  // cd command
  if (strcmp(cmd, CD) == 0) {
    _chdir(args, n);
    return;
  } 

  // status command
  if (strcmp(cmd, STATUS) == 0) {
    showStatus(status);
    return;
  } 

  // all other commands induces fork()
  _fork(args, n, bkgd, &status);
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
  free(args);
  args = NULL;
}

/* ****************************************************************************
 * Description: 
 * used to print argument strings, used to debug
 * @param args
 * ***************************************************************************/
void printargs(char** args) {
  int i = 0;
  while (args[i] != NULL) {
    printf("arg[%d]: %s\n", i, args[i]);
    i++;
  }
}

/* ****************************************************************************
 * main program
 * ***************************************************************************/
int main() {
  catchSignal();    // handle signals

  // buffer allocated by getline() that holds our entered string + \n + \0
  char* input = NULL; 

  // get command & run shell
  while(1) {   // from lecture notes
    // Get input from the user
    input = getcmd();
    if (input[0] == '\0') continue;

    // parse command
    int nArgs = 0;
    char** args = parseInput(&nArgs, input);

    // execute shell
    // int exitCode = -1;
    _runshell(args, nArgs);

    // Free the memory allocated by getline() or else memory leak
    free(input);
    input = NULL;
    freeargs(args, ARGBUFF);
  }

  return 0;
}
