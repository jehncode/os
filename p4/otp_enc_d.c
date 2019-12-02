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
#include "otp.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

/* ****************************************************************************
 * function declarations
 * ***************************************************************************/
// encryption
void encrypt(char*, char*);
int chtoval(char);
char valtoch(int);

void addrSetup(struct sockaddr_in* address, int port);
void authenticateConnection(char* tag, int socketFD);
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

// function to return value of character for encryption
int chtoval(char ch) {
  // if it's a space, the value is 26
  if (ch == ' ') return 26;
  // otherwise the value based on the character
  return (int) (ch - 'A') % 26;
}

// function to return char of value
char valtoch(int val) {
  // if the value is 26, the character is a space
  if (val == 26) return ' ';
  // otherwise it is based on the value
  return (char) (val % 26 + 'A');
}

/* ****************************************************************************
 * Description:
 * set up address info given port
 * @param address
 * @param port
 * ***************************************************************************/
void addrSetup(struct sockaddr_in* address, int port) {
}


/* ****************************************************************************
 * Description:
 * authenticates if connection can be made
 * @param tag
 * @param socketFD
 * ***************************************************************************/
void authenticateConnection(char* tag, int socketFD) {
  char buffer[BUFFER];
  memset(buffer, '\0', sizeof(buffer));

  // receive validation message from client
  recvMessage(buffer, sizeof(buffer), socketFD);

  // authenticate if buffer matches tag
  if (strcmp(buffer, tag) != 0) 
    error("error: attempt to connect terminated", 2);

  // authenticate by sending acceptance
  sendMessage(ACCEPT, socketFD);
}

/* ****************************************************************************
 * Description:
 * main program
 * ***************************************************************************/
int main(int argc, char* argv[]) {
  // print error if invalid number of arguments
  if (argc < 2) {
    fprintf(stderr, "USAGE: %s port\n", argv[0]);
    exit(1);
  }

  // get port number
  int port = atoi(argv[1]); // Get the port number from argument

  socklen_t sizeOfClientInfo;
  char buffer[BUFFER];
  memset(buffer, '\0', sizeof(buffer));
  char key[BUFFER];
  memset(key, '\0', sizeof(key));

  // set up address struct for process
  struct sockaddr_in serverAddress, clientAddress;
  memset((char *)&serverAddress, '\0', sizeof(serverAddress)); // Clear address

  serverAddress.sin_family = AF_INET; // Create a network-capable socket
  serverAddress.sin_port = htons(port); // Store the port number
  serverAddress.sin_addr.s_addr = INADDR_ANY; // Any address is allowed for connection to this process
  // addrSetup(&serverAddress, port);

  // set up socket
  int listenSocketFD, establishedConnectionFD;
  socketSetup(&listenSocketFD);

  // enable socket to begin listening
  // Connect socket to port
  if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, 
        sizeof(serverAddress)) < 0) 
    error("error: server unable to bind", 1);

  // Flip the socket on - it can now receive up to 5 connections
  if (listen(listenSocketFD, 5) < 0) 
    error("error: server unable to listen", 1);

  // wait for connection request from client
  int status = -5;
  while (1) {
    // accept connection, blocking if one isn't available until one connects
    sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address 
    establishedConnectionFD = accept(listenSocketFD, 
        (struct sockaddr*)&clientAddress, &sizeOfClientInfo);  // accept
    if (establishedConnectionFD < 0) 
      error("error: server unable to accept", 1);

    // fork request
    pid_t pid = fork();
    switch (pid) {
      case -1:  // error 
        error("error: server unable to create fork", 0);
        break;
      case 0:  // successful: get plaintext/key from client, send ciphertext 
        // authenticate client
        authenticateConnection(ENC_TAG, listenSocketFD);

        // read plaintext
        recvMessage(buffer, sizeof(buffer), establishedConnectionFD);
         printf("SERVER received plaintext: %s\n", buffer);
        // read key
        recvMessage(key, sizeof(key), establishedConnectionFD);
         printf("SERVER received key: %s\n", key);

        // encrypt plaintext
        encrypt(buffer, key);
         printf("SERVER ciphertext: %s\n", buffer);

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
