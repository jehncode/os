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

#define BUFFER 2048
#define ARGBUFF 512 

#define STKMAX 512

/* ****************************************************************************
 * definitions for strings/supported commands
 * ***************************************************************************/
#define HOME "HOME"
#define EXIT "exit"
#define CD "cd"
#define STATUS "status"

/* ****************************************************************************
 * struct/enum definitions
 * ***************************************************************************/
enum _bool {
  _true,
  _false
};

/* ****************************************************************************
 * global variables
 * ***************************************************************************/
enum _bool fgOnly = _false;
static struct sigaction stopsig = {{0}};
static struct sigaction termsig = {{0}};

/* ****************************************************************************
 * function declarations
 * ***************************************************************************/
// signal control
void catchSignal();
void catch_stopsig();
void catch_termsig();
void catch_childsig();
void _catchtermsig(int signo);
void _catchstopsig(int signo);
void _catchchildsig(int signo);
void resetsigaction();

// smallsh prog
void _runshell(char** args, int n, char* infile, char* outfile);
char* getcmd();
char** parseInput(int* n, char* input, char* infile, char* outfile);
void _fork(char** args, int n, enum _bool bkgd, int* childExitStatus,
    char* infile, char* outfile);

// status cmd
void showStatus(int childExitInteger);
void exitstatus(int childExitInteger);
void signalstatus(int childExitInteger);

// file directory
void _chfile(char** args, int n);

// file redirection
int fileDirection(char* infile, char* outfile);

// memory control
void freeargs(char** args, int n);

// debug
void printargs(char** args);

/* ****************************************************************************
 * Description:
 * initializes signal handlers for stop, term, and child
 * * ***************************************************************************/
void catchSignal() {
  catch_stopsig();
  catch_termsig();
  resetsigaction();
}

// handler for stop (ctrl+z)
void catch_stopsig() {
  // taken from lecture notes --initialize values for sigaction
  stopsig.sa_handler = _catchstopsig;
  sigfillset(&stopsig.sa_mask);
  stopsig.sa_flags = 0;
}

// handler for term (ctrl+c)
void catch_termsig() {
  // taken from lecture notes --initialize values for sigaction
  termsig.sa_handler = SIG_IGN;
  sigfillset(&termsig.sa_mask);
  termsig.sa_flags = 0;
}

// catch stop signal CTRL+Z (foreground only)
void _catchstopsig(int signo) {
  if (fgOnly == _false) {  // enter foreground-only mode
    printf("\nEntering foreground-only mode (& is now ignored)\n");
    fgOnly = _true;
  } else {                  // exit foreground-only mode
    printf("\nExiting foreground-only mode\n");
    fgOnly = _false;
  }
  fflush(stdout);
}

// catch CTRL+C
void _catchtermsig(int signo) {
  printf("terminated by signal %d\n", signo);
}

// reset signal actions for stop, term, and child
void resetsigaction() {
  sigaction(SIGTSTP, &stopsig, NULL);
  sigaction(SIGINT, &termsig, NULL);
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
  // clear input/output command line buffers
  fflush(stdout);
  fflush(stdin);

  // get input
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
 * @param nArgs
 * @param input
 * ***************************************************************************/
char** parseInput(int* nArgs, char* input, char* infile, char* outfile) {
  // reset file names
  strcpy(infile, "\0");
  strcpy(outfile, "\0");

  // get segments for command arguments
  int n = 0;    // idx for segments in input;
  char** args = malloc(sizeof(char*) * ARGBUFF);
  char* token = strtok(input, " \n");
  while(token != NULL) {

    // check if # for comment
    if (token[0] == '#') {
      break;
    }

    // check for file redirection
    if (strcmp(token, "<") == 0) {
      token = strtok(NULL, " ");
      strcpy(infile, token);

      // check for file redirection
    } else if (strcmp(token, ">") == 0) {
      token = strtok(NULL, " ");
      strcpy(outfile, token);

      // duplicate argument to args list
    } else {
      // save token as segment
      char* curr = malloc(sizeof(char) * BUFFER);
      sprintf(curr, "%s", token);

      // check for variable expansion--pid
      char* needleptr = strstr(curr, "$$");
      if (needleptr != NULL) {
        // printf("needleptr: %s\n", needleptr);
        needleptr[0] = '\0';
        // replace with pid
        sprintf(curr, "%s%d", curr, getpid());
      }

      // save segment of argument for return
      args[n++] = curr;
    }
    // update token
    token = strtok(NULL, " \n");
  }

  *nArgs = n;
  fflush(stdout);
  fflush(stdin);
  // printargs(args);
  return args;
}

/* ****************************************************************************
 * Description:
 * check if there is a file direction in command, sets direction if occurs
 * and returns 1 if redirection occurs, 0 otherwise
 * @param infile
 * @param outfile
 * ***************************************************************************/
int fileDirection(char* infile, char* outfile) {
  int res = 0;
  // handle file redirection 
  // input file
  if (strcmp(infile, "") != 0) {
    // attempt to open file
    int file = open(infile, O_RDONLY);
    // check if file is opened/dup made
    if (file == -1 || dup2(file, 0) == -1) {
      perror("error: unable to access file");
      exit(1);
    }
    // update return value
    res = 1;
    // close file
    fcntl(file, F_SETFD, FD_CLOEXEC);
  }

  // output file
  if (strcmp(outfile, "") != 0) {
    // attempt to open file
    int file = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    // check if file is opened/dup made
    if (file == -1 || dup2(file, 1) == -1) {
      perror("error: unable to access file");
      exit(1);
    } 
    // update return value
    res = 1;
    // close file
    fcntl(file, F_SETFD, FD_CLOEXEC);
  }

  return res;
}

/* ****************************************************************************
 * Description:
 * creates fork for all other commands
 * @param args
 * @param n
 * @param bkgd
 * @param childExitStatus
 * @param infile
 * @param outfile
 * ***************************************************************************/
void _fork(char** args, int n, enum _bool bkgd, int* childExitStatus,
    char* infile, char* outfile) {
  // create a fork for process
  pid_t pid = fork();
  switch(pid) {
    case -1:    // check if error creating child
      perror("error: unable to create fork\n");
      exit(1); // exit 1 if error
      break;

    case 0:     // curr is child process
      // handle child process process
      termsig.sa_handler = SIG_DFL;
      sigaction(SIGINT, &termsig, NULL);

      // handle file redirects
      fileDirection(infile, outfile);

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
      if (bkgd == _true && fgOnly == _false) {          // on background
        waitpid(pid, childExitStatus, WNOHANG);
        // print background pid
        printf("background pid is %d\n", pid);
        fflush(stdout);
      } else {  // on foreground
        // wait for process to finish
        waitpid(pid, childExitStatus, 0);
        // check if signal terminated process
        signalstatus(*childExitStatus);
      }
      // run through background processes
      while ((pid = waitpid(-1, childExitStatus, WNOHANG)) > 0) {
        printf("background process %d is done: ", pid);
        showStatus(*childExitStatus);
        fflush(stdout);
      }
  }
}

/* ****************************************************************************
 * Description:
 * runs shell program and executes user's commands
 * @param args
 * @param n
 * @param infile
 * @param outfile
 * ***************************************************************************/
void _runshell(char** args, int n, char* infile, char* outfile) {
  // check for invalid arguments
  if (args == NULL || n < 1) {
    return;
  }

  enum _bool bkgd = _false; // track if background process
  static int status = 0;    // status code

  // check if background process and if it can be background process
  if (strcmp(args[n - 1], "&") == 0 && fgOnly == _false) {
    bkgd = _true;   // if so, assign it to background
    free(args[n - 1]);  // remove last argument: "&" for remainder of this run 
    args[--n] = NULL;
  }

  // check if executed is a supported commands
  char* cmd = args[0];

  // exit command
  if (strcmp(cmd, EXIT) == 0) {
    // exit status 0
    exit(0);
  }

  // cd command
  if (strcmp(cmd, CD) == 0) {
    // change directory
    _chdir(args, n);  // chdir command
    return;
  } 

  // status command
  if (strcmp(cmd, STATUS) == 0) {
    showStatus(status);
    return;
  } 

  // all other commands induces fork()
  _fork(args, n, bkgd, &status, infile, outfile);
}

/* ****************************************************************************
 * Description: frees args 
 * @param args
 * @param n
 * ***************************************************************************/
void freeargs(char** args, int n) {
  // free each string
  int i = 0;
  for (; i < n; i++) {
    free(args[i]);
    args[i] = NULL;
  }
  // free the pointer to the array of strings
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
    // reset signal handler
    // resetsigaction();

    // Get input from the user
    input = getcmd();
    if (input[0] == '\0') continue;

    // parse command
    int nArgs = 0;
    char* infile = malloc(sizeof(char) * BUFFER);
    memset(infile, sizeof(infile), '\0');
    char* outfile = malloc(sizeof(char) * BUFFER);
    memset(outfile, sizeof(outfile), '\0' );
    char** args = parseInput(&nArgs, input, infile, outfile);

    // execute shell
    _runshell(args, nArgs, infile, outfile);

    // Free the memory allocated or else memory leak
    free(infile);
    free(outfile);
    free(input);
    infile = NULL;
    outfile = NULL;
    input = NULL;
    freeargs(args, ARGBUFF);
  }

  return 0;
}
