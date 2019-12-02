/* ****************************************************************************
 * Name:    Jenny Huang
 * Date:    November 26, 2019
 * Description: opt.h
 * This program contains common function declarations for 
 *    otp_enc,  
 *    otp_enc_d,
 *    otp_dec,
 *    otp_dec_d
 * **************************************************************************/
#ifndef OTP_H
#define OTP_H 

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define BUFFER 2048
#define HOST "localhost"
#define ENC_TAG "otp_enc"
// connection validity
#define ACCEPT "accepted"
#define REJECT "rejected"


// socket correspondence
int sendMessage(char*, int);
int recvMessage(char*, int, int);

// socket
// void addressSetup(struct sockaddr_in*, int);
void socketConnect(int, struct sockaddr_in*);

// prints
void error(const char*, int);

#endif
