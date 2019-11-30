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
void encrypt(char*, int, char*, int);
int chtoval(char);
char valtoch(int);

void error(const char*);

/* ****************************************************************************
 * Description:
 * encrypts text considering OTP
 * @param text
 * @param n
 * @param key
 * @param k
 * ***************************************************************************/
void encrypt(char* text, int n, char* key, int k) {
  printf("key:\n%s\n", key);
  printf("plain text:\n%s\n", text);
  int i = 0;
  for (; i < n; i++) {
    int val = chtoval(text[i]) + chtoval(key[i % k]);
    text[i] = valtoch(val);
  }

  printf("cipher text:\n%s\n", text);
}

// helper function to return value of character for encryption
int chtoval(char ch) {
  if (ch == ' ') return 26;
  return (int) (ch - 'A') % 26;
}

// helper function to return char of value
char valtoch(int val) {
  if (val == 26) return ' ';
  return (char) (val % 26 + 'A');
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
  struct sockaddr_in serverAddress, clientAddress;

  // set up address struct for process
  memset((char *)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
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

  // while (1) {
    // accept connection, blocking if one isn't available until one connects
    sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address 
    establishedConnectionFD = accept(listenSocketFD, 
        (struct sockaddr*)&clientAddress, &sizeOfClientInfo);  // accept
    if (establishedConnectionFD < 0) error("error: unable to accept");

    // fork request
    //
    /*
       pid_t pid = fork();
       switch (pid) {
       case -1:  // error 
       error("error: unable to create fork");
       break;
       case 0:  // successful: get message from client and display it
       memset(buffer, '\0', sizeof(buffer));
    // Read the client's message from the socket
    charsRead = recv(establishedConnectionFD, buffer, BUFFER, 0); 

    if (charsRead < 0) error("ERROR reading from socket");
    printf("SERVER: I received this from the client: \"%s\"\n", buffer);
    // send success message to client
    charsRead = send(establishedConnectionFD, "I am the server, and I got your message", 39, 0); // Send success back
    if (charsRead < 0) error("error: writing to socket");
    close(establishedConnectionFD); // Close the existing socket which is connected to the client
    memset(buffer, '\0', sizeof(buffer));
    charsRead = recv(establishedConnectionFD, buffer, BUFFER, 0);

    close(listenSocketFD); // Close the listening socket
    break;
    default:
    break;
    }
    */

    // read plain text
    memset(buffer, '\0', sizeof(buffer));
    // Read the client's message from the socket
    charsRead = recv(establishedConnectionFD, buffer, BUFFER, 0); 

    if (charsRead < 0) error("SERVER error: reading from socket");
    printf("SERVER: I received this from the client: \"%s\"\n", buffer);
    // send success message to client
    charsRead = send(establishedConnectionFD, "I am the server, and I got your message", 39, 0); // Send success back
    if (charsRead < 0) error("SERVER error: writing to socket");


    // read key
    memset(buffer, '\0', sizeof(buffer));
    charsRead = recv(establishedConnectionFD, buffer, BUFFER, 0);
    if (charsRead < 0) error("SERVER error: unable to read from socket");
    printf("SERVER: I received this from the client: \"%s\"\n", buffer);
    // send success message to client
    charsRead = send(establishedConnectionFD, "SERVER: I got your message", 39, 0); // Send success back
    if (charsRead < 0) error("SERVER error: unable to write to socket");

    close(establishedConnectionFD); // Close the existing socket which is connected to the client
    close(listenSocketFD); // Close the listening socket
  // }


  return 0;
}
