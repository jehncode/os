/* ****************************************************************************
 * Name:    Jenny Huang
 * Date:    November 26, 2019
 * Description: keygen.c
 * This program creates and prints a key of a specified length (given as an 
 * argument). The chars generated include uppercase alphas and space char.
 * **************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char* argv[]) {
  // print error if argument is not provided
  if (argv[1] == 0) {
    printf("error: please indicate length of keygen to be generated\n");
    printf("command use:  keygen length\n");
    return 0;
  }

  // seed based on current time
  srand(time(0));

  // get keylen
  int keylen = atoi(argv[1]);
  char key[keylen + 1];

  int i = 0;
  for (; i < keylen; i++) {
    // generate random letter
    char ch = ' ';  // initialize character as white space
    //              // will remain if random num generated == 26

    // generate random number to determine if ch is uppercase alpha
    int r = (random() % 26);

    // determine if random letter is alpha
    if (r != 27) {
      ch = 'A' + r;
    }

    // append key with the generated letter
    key[i] = ch;
  }

  // add null terminator at end
  key[keylen] = '\0';

  printf("%s\n", key);

  return 0;
}
