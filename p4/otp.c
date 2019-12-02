/* ****************************************************************************
 * Name:    Jenny Huang
 * Date:    November 26, 2019
 * Description: opt.c
 * This program contains common function definitions for 
 *    otp_enc,  
 *    otp_enc_d,
 *    otp_dec,
 *    otp_dec_d
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

#define BUFFER 2048
#define PROGTAG "otp_enc"
#define HOST "localhost"
// connection validity
#define ACCEPT "accepted"
#define REJECT "rejected"

/* ****************************************************************************
 * Description:
 * send message through socket, using send() 
 * returns return value of send() if error is not encountered
 * @param buffer
 * @param socketFD
 * ***************************************************************************/
int sendMessage(char* buffer, int socketFD) {
  // place message in socket
  int charsWritten = send(socketFD, buffer, strlen(buffer), 0);
  // print error if unable to write to socket
  if (charsWritten < 0) error("error: unable to write to socket", 0);
  // print error if not all data was written to socket
  if (charsWritten < strlen(buffer)) 
    error("error: not all data written to socket", 1);
  return charsWritten;
}

/* ****************************************************************************
 * Description:
 * get message from socket, using recv()
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
  if (charsRead < 0) error("error: unable to read from socket", 0);
  // printf(": received in socket: \"%s\"\n", buffer);  // debug
  return charsRead;
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
  if (stat < 0) { error("error: unable to connect", 1); }
}

/* ****************************************************************************
 * Description:
 * print error message and exit
 * ***************************************************************************/
void error(const char* msg, int stat) { perror(msg); exit(stat); }
