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
#include "otp.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define HOST "localhost"
// connection validity
#define ACCEPT "accepted"
#define REJECT "rejected"

/* ****************************************************************************
 * function declarations
 * ***************************************************************************/
void validateText(char*, char*);

// file
void readfromfile(char*, int, char*);
// socket/address
void addrSetup(struct sockaddr_in*, int);
void validateConnection(char* tag, int socketFD);

/* ****************************************************************************
 * Description:
 * returns 1 if text contains contains valid characters only
 * valid characters include:
 *    space
 *    uppercase alphas
 * returns 0 otherwise
 * @param text
 * ***************************************************************************/
void validateText(char* text, char* filename) {
  int i = 0;
  int n = strlen(text);
  for (; i < n; i++) {
    char ch = text[i];
    // if not space nor uppercase alpha
    if (ch != ' ' && isupper(ch) == 0) {
      // printf("char: \'%c\' is invalid", ch);
      fprintf(stderr, "error: file \'%s\' contains invalid characters", filename);
      exit(1);   // result: invalid
    }
  }
}

/* ****************************************************************************
 * Description:
 * reads text from file into buffer
 * @param buffer
 * @param n
 * @param filename
 * ***************************************************************************/
void readfromfile(char* buffer, int n, char* filename) {
  memset(buffer, '\0', n);

  FILE* fi = fopen(filename, "r");
  if (fi < 0) error("error: unable to open text file", 0);
  // printf("sizeofbuffer: %d\n", sizeof(buffer));
  // get input from plaintext file
  while (fgets(buffer, n - 1, fi)) {
  };
  fclose(fi);  // close file
  buffer[strcspn(buffer, "\n")] = '\0'; // remove trailing \n that fgets adds
  // printf("read from %s :\n%s\n", filename, buffer); // for debugging

  // validate text contains valid characters
  validateText(buffer, filename);
}

/* ****************************************************************************
 * Description:
 * set up address info given port
 * @param serverAddress
 * @param port
 * ***************************************************************************/
void addrSetup(struct sockaddr_in* serverAddress, int port) {
}

/* ****************************************************************************
 *  Description:
 *  validates connection is allowed
 *  @param tag
 *  @param socketFD
 * ***************************************************************************/
void validateConnection(char* tag, int socketFD) {
  // send this program's tag to server
  sendMessage(tag, socketFD);

  // get server's response
  char buffer[BUFFER];
  memset(buffer, '\0', sizeof(buffer));
  recvMessage(buffer, sizeof(buffer), socketFD);

  // check if accepted, print error if not
  if (strcmp(buffer, ACCEPT) != 0)
    error("error: client unable to connect to server", 2);
}

/* ****************************************************************************
 * Description:
 * main program
 * ***************************************************************************/
int main(int argc, char* argv[]) {
  // print error if invalid number of arguments is provided
  if (argc < 4) {
    fprintf(stderr, "USAGE:  %s plaintext key port\n", argv[0]);
    exit(1);
  }

  // get port number
  int port = atoi(argv[3]); // get port number from argument

  // set up address info
  struct sockaddr_in serverAddress;
  // addrSetup(&serverAddress, port);
  memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // clear struct
  struct hostent* serverHostInfo;
  // set up address struct
  serverAddress.sin_family = AF_INET;  // create network-capable socket
  serverAddress.sin_port = htons(port);  // store port number
  serverHostInfo = gethostbyname(HOST); // convert machine name to special form
  // print error if occurs
  if (serverHostInfo == NULL) error("error: client unable to find host", 1);
  // copy the address
  memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr,
      serverHostInfo->h_length);

  // set up the socket w/ capabilities of full-size socket
  int socketFD = socket(AF_INET, SOCK_STREAM, 0); // create socket
  if (socketFD < 0) error("error: client unable to open socket", 1);
  // socketSetup(&socketFD);

  // connect to server
  socketConnect(socketFD, &serverAddress);
  // validate connection
  validateConnection(ENC_TAG, socketFD);

  // initialize variables
  char buffer[BUFFER];
  memset(buffer, '\0', sizeof(buffer)); // clear buffer

  // get plaintext from file
  char* textfile = argv[1];
  readfromfile(buffer, BUFFER, textfile);
  int n = strlen(buffer); // length of plaintext
  sendMessage(buffer, socketFD);  // send plaintext to server

  // get key from file
  char* keyfile = argv[2];
  readfromfile(buffer, BUFFER, keyfile);
  int k = strlen(buffer); // length of key

  // check if key is long enough, exit as reqd
  if (n > k) { 
    fprintf(stderr, "error: key \'%s\' is too short\n", keyfile); 
    exit(1); 
  }

  sendMessage(buffer, socketFD);  // send key to server

  // receive encoded text
  recvMessage(buffer, sizeof(buffer), socketFD);
  printf("%s\n", buffer);

  // close the socket
  close(socketFD);

  return 0;
}
