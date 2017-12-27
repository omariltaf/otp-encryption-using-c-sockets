//Omar Iltaf

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>


void GenRandString(char* buffer, char* lengthString) {
  // Converts length of string to integer
  int lengthInt = atoi(lengthString);
  // Character set used in this assignment
  char charSet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
  // Sets the seed
  srand(time(NULL));
  // Generates the random string from the character set
  for (int i = 0; i < lengthInt; i++) {
    buffer[i] = charSet[rand() % (sizeof(charSet) - 1)];
  }
  // Adds null terminator to end of buffer
  buffer[lengthInt] = '\0';
}

int main(int argc, char *argv[]) {
  // Sets and clears buffer
  char buffer[100000];
  memset(buffer, '\0', sizeof(buffer));

  // Calls method to generate random string and place in buffer
  GenRandString(buffer, argv[1]);

  // Prints out buffer with generated random string to stdout
  fprintf(stdout, "%s\n", buffer);
  return 0;
}
