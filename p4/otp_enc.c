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
#define PROGTAG "otp_enc"
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

// socket correspondence
int sendMessage(char*, int);
int recvMessage(char*, int, int);

// socket
void addressSetup(struct sockaddr_in*, int);
void socketSetup(int*);
void socketConnect(int, struct sockaddr_in*);
void validateConnection(int);

// prints
void error(const char*, int);

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
 * send message from client
 * returns return value of send() if error is not encountered
 * @param buffer
 * @param socketFD
 * ***************************************************************************/
int sendMessage(char* buffer, int socketFD) {
  // place message in socket
  int charsWritten = send(socketFD, buffer, strlen(buffer), 0);
  // print error if unable to write to socket
  if (charsWritten < 0) error("CLIENT error: unable to write to socket", 0);
  // print error if not all data was written to socket
  if (charsWritten < strlen(buffer)) 
    error("CLIENT error: not all data written to socket", 0);
  return charsWritten;
}

/* ****************************************************************************
 * Description:
 * get message from server
 * returns return value of recv() if error is not encountered
 * @param buffer
 * @param n
 * @param socketFD
 * ***************************************************************************/
int recvMessage(char* buffer, int n, int socketFD) {
  // get message from server
  memset(buffer, '\0', n);
  // read data from the socket, leaving \0 at end
  int charsRead = recv(socketFD, buffer, n - 1, 0);
  if (charsRead < 0) error("CLIENT error: unable to read from socket", 0);
  // printf("CLIENT: received from server: \"%s\"\n", buffer);

  return charsRead;
}

/* ****************************************************************************
 * Description:
 * set up address info given port number
 * ***************************************************************************/
void addrSetup(struct sockaddr_in* serverAddress, int port) {
  memset((char*) &serverAddress, '\0', sizeof(serverAddress)); // clear struct
  struct hostent* serverHostInfo;

  // set up address struct 
  serverAddress->sin_family = AF_INET; // create network-capable socket
  serverAddress->sin_port = htons(port); // store port number
  serverHostInfo = gethostbyname(HOST); // convert machine name to special form
  if (serverHostInfo == NULL) error("CLIENT error: no such host", 0);
  memcpy((char*) &serverAddress->sin_addr.s_addr, (char*)serverHostInfo->h_addr, 
    serverHostInfo->h_length);  // copy the address
}

/* ****************************************************************************
 * Description:
 * set up socket, exit with error if occurs
 * @param socketFD
 * ***************************************************************************/
void socketSetup(int* socketFD) {
  *socketFD = socket(AF_INET, SOCK_STREAM, 0); // capabilities of full size socket
  // print error if socket not created
  if (*socketFD < 0) error("CLIENT error: unable to open socket", 0);
}

/* ****************************************************************************
 * Description:
 * attempts to connect to socket, exit with error if occurs
 * @param socketFD
 * @param serverAdress
 * ***************************************************************************/
void socketConnect(int socketFD, struct sockaddr_in* serverAddress) {
  struct sockaddr* addr = (struct sockaddr*)serverAddress;
  int stat = connect(socketFD, addr, sizeof(*addr));
  // print error if connection isnt made
  if (stat < 0) { error("CLIENT error: unable to connect", 0); }
  // ensure connection is allowed
  validateConnection(socketFD);
}

/* ****************************************************************************
 * Description:
 * validates connection is allowed
 * @param socketFD
 * ***************************************************************************/
void validateConnection(int socketFD) {
  // send this program's tag to server
  sendMessage(PROGTAG, socketFD);

  // get server's response
  char buffer[BUFFER];
  memset(buffer, '\0', sizeof(buffer));
  recvMessage(buffer, sizeof(buffer), socketFD); 

  // check if accepted, print error if not
  if (strcmp(buffer, ACCEPT) != 0) 
    error("CLIENT error: connection not allowed", 2);
  
}

/* ****************************************************************************
 * Description:
 * print error message and exit
 * ***************************************************************************/
void error(const char* msg, int stat) { perror(msg); exit(stat); }

/* ****************************************************************************
 * Description:
 * main program
 * ***************************************************************************/
int main(int argc, char* argv[]) {
  // print error if invalid number of arguments is provided
  if (argc < 4) {
    fprintf(stderr, "USAGE:  %s plaintext key port\n", argv[0]);
    exit(0);
  }
  
  // get port
  int port = atoi(argv[3]); // get port number from argument
  
  // set up address info
  struct sockaddr_in serverAddress;
  addrSetup(&serverAddress, port);

  // set up the socket
  int socketFD; 
  socketSetup(&socketFD);
  // connect to server
  socketConnect(socketFD, &serverAddress);

  // initialize variables
  char buffer[BUFFER];
  memset(buffer, '\0', sizeof(buffer)); // clear buffer
  // char keybuff[BUFFER];
  // memset(keybuff, '\0', sizeof(keybuff)); // clear buffer

  // get plaintext from file
  char* textfile = argv[1];
  readfromfile(buffer, BUFFER, textfile);

  int n = strlen(buffer); // length of plaintext
  // send plaintext message to server
  sendMessage(buffer, socketFD);

  // get key from file
  char* keyfile = argv[2];
  readfromfile(buffer, BUFFER, keyfile);

  int k = strlen(buffer); // length of key
  
  // check if key is long enough, exit as reqd
  if (n > k) { 
    fprintf(stderr, "error: key \'%s\' is too short\n", keyfile); 
    exit(1); 
  }

  // send key to server
  sendMessage(buffer, socketFD);

  // receive encoded text
  recvMessage(buffer, sizeof(buffer), socketFD);
  printf("%s\n", buffer);

  // close the socket
  close(socketFD);

  return 0;
}
