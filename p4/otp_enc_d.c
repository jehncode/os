/* ****************************************************************************
 * Name:    Jenny Huang
 * Date:    November 26, 2019
 * Description: opt_enc.c
 * This program is run in the background as a daemon. An error is printed if
 * it can't be run due to a network error, such as the ports being unavailable
 * This program performs the actual encoding for OTP.
 * This program listens to a particular port/socket and accepts when a 
 * connection is requested. This program supports up to five concurrent 
 * socket connections running at the same time.
 * This program is ran as follows:
 *    otp_enc_d port &
 * where 
 *    port is the port that the program attemps to connect otp_enc_d on
 * **************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define BUFFER 2048
#define HOST "localhost"

/* ****************************************************************************
 * function declarations
 * ***************************************************************************/
void encrypt(char*, char*);
int chtoval(char);
char valtoch(int);

void error(const char*);

/* ****************************************************************************
 * Description:
 * encrypts text considering OTP
 * @param text
 * @param key
 * ***************************************************************************/
void encrypt(char* text, char* key) {
  int n = strlen(text); // length of plaintext
  int k = strlen(key);  // length of key

  // printf("key:\n%s\n", key);  printf("plain text:\n%s\n", text);
  int i = 0;
  for (; i < n; i++) {
    int val = chtoval(text[i]) + chtoval(key[i]);
    text[i] = valtoch(val);
  }
  // printf("cipher text:\n%s\n", text);
}

// helper function to return value of character for encryption
int chtoval(char ch) {
  // if it's a space, the value is 26
  if (ch == ' ') return 26;
  // otherwise the value based on the character
  return (int) (ch - 'A') % 26;
}

// helper function to return char of value
char valtoch(int val) {
  // if the value is 26, the character is a space
  if (val == 26) return ' ';
  // otherwise it is based on the value
  return (char) (val % 26 + 'A');
}


/* ****************************************************************************
 * Description:
 * send message from server
 * returns return value of send() if error is not encountered
 * @param buffer
 * @param establishedConnectionFD
 * ***************************************************************************/
int sendMessage(char* buffer, int establishedConnectionFD) {
  // place message in socket
  int charsWritten = send(establishedConnectionFD, buffer, strlen(buffer), 0);
  // print error if unable to write to socket
  if (charsWritten < 0) error("SERVER error: unable to write to socket");
  // print error if not all data was written to socket
  if (charsWritten < strlen(buffer))
    error("SERVER error: not all data written to socket\n");
  return charsWritten;
}

/* ****************************************************************************
 * Description:
 * get message from client 
 * returns return value of recv() if error is not encountered
 * @param buffer
 * @param n
 * @param establishedConnectionFD
 * ***************************************************************************/
int recvMessage(char* buffer, int n, int establishedConnectionFD) {
  // clear memory of buffer
  memset(buffer, '\0', n);

  // Read the client's message from the socket
  int charsRead = recv(establishedConnectionFD, buffer, n, 0); 
  // print error if unable to read from socket
  if (charsRead < 0) error("SERVER error: reading from socket");
  // printf("SERVER: I received this from the client: \"%s\"\n", buffer);
  return charsRead;
}

/* ****************************************************************************
 * Description:
 * print error message and exit
 * ***************************************************************************/
void error(const char* msg) { perror(msg); exit(0); }

/* ****************************************************************************
 * Description:
 * main program
 * ***************************************************************************/
int main(int argc, char* argv[]) {
  if (argc < 2) {
    fprintf(stderr, "USAGE: %s port\n", argv[0]);
    exit(1);
  }

  int listenSocketFD, establishedConnectionFD, portNumber, charsRead;
  socklen_t sizeOfClientInfo;
  char buffer[BUFFER];
  char key[BUFFER];
  struct sockaddr_in serverAddress, clientAddress;

  // clear memory
  memset(buffer, '\0', sizeof(buffer));
  memset(key, '\0', sizeof(key));
  memset((char *)&serverAddress, '\0', sizeof(serverAddress)); // Clear address

  // set up address struct for process
  portNumber = atoi(argv[1]); // Get the port number, convert to an integer from a string
  serverAddress.sin_family = AF_INET; // Create a network-capable socket
  serverAddress.sin_port = htons(portNumber); // Store the port number
  serverAddress.sin_addr.s_addr = INADDR_ANY; // Any address is allowed for connection to this process

  // set up socket
  listenSocketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
  if (listenSocketFD < 0) error("error: unable to open socket");

  // enable socket to begin listening
  // Connect socket to port
  if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, 
        sizeof(serverAddress)) < 0) { error("error: unable to bind"); }

  // Flip the socket on - it can now receive up to 5 connections
  listen(listenSocketFD, 5);

  int status = -5;
  while (1) {
    // accept connection, blocking if one isn't available until one connects
    sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address 
    establishedConnectionFD = accept(listenSocketFD, 
        (struct sockaddr*)&clientAddress, &sizeOfClientInfo);  // accept
    if (establishedConnectionFD < 0) error("error: unable to accept");

    // fork request
    pid_t pid = fork();
    switch (pid) {
      case -1:  // error 
        error("error: unable to create fork");
        break;
      case 0:  // successful: get plaintext/key from client, send ciphertext 
        // Read the client's message from the socket

        // read plaintext
        recvMessage(buffer, sizeof(buffer), establishedConnectionFD);
        // printf("SERVER received plaintext: %s\n", buffer);
        // read key
        recvMessage(key, sizeof(key), establishedConnectionFD);
        // printf("SERVER received key: %s\n", key);

        // encrypt plaintext
        encrypt(buffer, key);
        // printf("SERVER ciphertext: %s\n", buffer);

        // write ciphertext to socket
        sendMessage(buffer, establishedConnectionFD);

        close(establishedConnectionFD); // Close the existing socket which is connected to the client
        break;
      default:  // parent process
        close(establishedConnectionFD);
        while (pid > 0) {
          pid = waitpid(-1, &status, WNOHANG);
        }
        break;
    }
  }
  close(listenSocketFD); // Close the listening socket

  return 0;
}
