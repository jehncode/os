/* ****************************************************************************
 * Name:    Jenny Huang
 * Date:    November 26, 2019
 * Description: opt_enc.c
 * This program connects to otp_enc_d and asks it to perform a one-time pad
 * style encryption.
 * This program is ran as follows:
 *    otp_enc plaintext key port
 * where 
 *    plaintext is the name of the file in the current directory that contains
 *        the plaintext to be encrypted
 *    key contains the encryption key used to encrypt the text
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
int validText(char*, int);
void error(const char*);

/* ****************************************************************************
 * Description:
 * returns 1 if text contains contains valid characters only
 * valid characters include:
 *    space
 *    uppercase alphas
 * returns 0 otherwise
 * @param text
 * @param n
 * ***************************************************************************/
int validText(char* text, int n) {
  int i = 0;
  for (; i < n; i++) {
    char ch = text[i];
    // if not space nor uppercase alpha
    if (ch != ' ' && isupper(ch) == 0) {
      printf("char: \'%c\' is invalid", ch);
      return 0;   // result: invalid
    }
  }
  return 1;   // result: valid
}

/* ****************************************************************************
 * Description:
 * ***************************************************************************/
void readfromfile(char* buffer, int n, char* filename) {
  memset(buffer, '\0', n);
  
  FILE* fi = fopen(filename, "r");
  if (fi < 0) error("error: unable to open text file");
  // get input from plaintext file
  // printf("sizeofbuffer: %d\n", sizeof(buffer));
  while (fgets(buffer, n - 1, fi)) {
  };
  fclose(fi);  // close file
  buffer[strcspn(buffer, "\n")] = '\0'; // remove trailing \n that fgets adds
  // printf("read from %s :\n%s\n", filename, buffer); // for debugging
}

/* ****************************************************************************
 * Description:
 * print error message and exit
 * ***************************************************************************/
int sendMessage(char* buffer, int socketFD) {
  int charsWritten = send(socketFD, buffer, strlen(buffer), 0);
  if (charsWritten < 0) error("CLIENT error: unable to write to socket");
  if (charsWritten < strlen(buffer)) 
    printf("CLIENT warning: not all data written to socket\n");

  return charsWritten;
}

/* ****************************************************************************
 * Description:
 * ***************************************************************************/
int recvMessage(char* buffer, int n, int socketFD) {
  // get message from server
  memset(buffer, '\0', n);
  // read data from the socket, leaving \0 at end
  int charsRead = recv(socketFD, buffer, n - 1, 0);
  if (charsRead < 0) error("CLIENT ERROR: unable to read from socket");
  // printf("CLIENT: received from server: \"%s\"\n", buffer);

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
  // print error if invalid number of arguments is provided
  if (argc < 4) {
    printf("USAGE:  %s plaintext key port\n", argv[0]);
    exit(0);
  }
  
  // initialize variables
  int socketFD, port, charsWritten, charsRead;
  struct sockaddr_in serverAddress;
  struct hostent* serverHostInfo;
  char buffer[BUFFER];
  char keybuff[BUFFER];
  // clear memory for usage
  memset(buffer, '\0', sizeof(buffer)); // clear buffer
  memset(keybuff, '\0', sizeof(keybuff)); // clear buffer
  memset((char*) &serverAddress, '\0', sizeof(serverAddress)); // clear stuct 
  
  // set up socket
  serverAddress.sin_family = AF_INET; // create network-capable socket
  port = atoi(argv[3]); // get port number from argument
  serverAddress.sin_port = htons(port); // store port number
  serverHostInfo = gethostbyname(HOST); // convert machine name to special form
  if (serverHostInfo == NULL) error("CLIENT error: no such host");
  memcpy((char*) &serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, 
    serverHostInfo->h_length);  // copy the address

  // set up the socket
  socketFD = socket(AF_INET, SOCK_STREAM, 0); // capabilities of full size socket
  if (socketFD < 0) error("CLIENT error: unable to open socket");

  // connect to server
  struct sockaddr* addr = (struct sockaddr*)&serverAddress;
  if (connect(socketFD, addr, sizeof(*addr)) < 0)
    error("CLIENT error: unable to connect");

  // get plain text from file
  readfromfile(buffer, BUFFER, argv[1]);
  // send plaintext message to server
  sendMessage(buffer, socketFD);

  // get key from file
  readfromfile(buffer, BUFFER, argv[2]);
  // send key to server
  sendMessage(buffer, socketFD);

  // receive encoded text
  recvMessage(buffer, sizeof(buffer), socketFD);
  printf("CLIENT received ciphertext: \"%s\"\n", buffer);

  // close the socket
  close(socketFD);

  return 0;
}
