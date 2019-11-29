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
  if (argc < 3) { printf("USAGE:  %s plaintext key\n", argv[0]); exit(0); }

  char buffer[BUFFER];
  char keybuff[BUFFER];
  // clear memory for usage
  memset(buffer, '\0', sizeof(buffer)); // clear buffer
  memset(keybuff, '\0', sizeof(keybuff)); // clear buffer

  // get plain text from file
  FILE* textfile = fopen(argv[1], "r");
  if (textfile < 0) error("error: unable to open text file");
  // get input from plaintext file
  while (fgets(buffer, sizeof(buffer) - 1, textfile));
  fclose(textfile);  // close file
  buffer[strcspn(buffer, "\n")] = '\0'; // remove trailing \n that fgets adds
  int n = strlen(buffer); 
  
  // get key from file
  FILE* keyfile = fopen(argv[2], "r");
  if (keyfile < 0) error("error: unable to open key file");
  // get input from plaintext file
  while (fgets(keybuff, sizeof(keybuff) - 1, keyfile));
  fclose(keyfile);  // close file
  keybuff[strcspn(keybuff, "\n")] = '\0'; // remove trailing \n that fgets adds
  int k = strlen(keybuff);
  
  encrypt(buffer, n, keybuff, k);
  /*
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
  FILE* textfile = fopen(argv[1], "r");
  if (textfile < 0) error("error: unable to open text file");
  // get input from plaintext file
  while (fgets(buffer, sizeof(buffer) - 1, textfile));
  fclose(textfile);  // close file
  buffer[strcspn(buffer, "\n")] = '\0'; // remove trailing \n that fgets adds
  printf("plaintext:\n%s\n", buffer); // for debugging
  
  // get key from file
  FILE* keyfile = fopen(argv[2], "r");
  if (keyfile < 0) error("error: unable to open key file");
  // get input from plaintext file
  while (fgets(keybuff, sizeof(keybuff) - 1, keyfile));
  fclose(keyfile);  // close file
  keybuff[strcspn(keybuff, "\n")] = '\0'; // remove trailing \n that fgets adds
  printf("key:\t%s\n", buffer); // for debugging

  // send message to server
  charsWritten = send(socketFD, buffer, strlen(buffer), 0);
  if (charsWritten < 0) error("CLIENT error: unable to write to socket");
  if (charsWritten < strlen(buffer)) 
    printf("CLIENT warning: not all data written to socket\n");

  // get return message from server
  memset(buffer, '\0', sizeof(buffer));
  // read data from the socket, leaving \0 at end
  charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0);
  if (charsRead < 0) error("CLIENT ERROR: unable to read from socket");
  printf("CLIENT: received from server: \"%s\"\n", buffer);

  // close the socket
  close(socketFD);
*/
  return 0;
}
