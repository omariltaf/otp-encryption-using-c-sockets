// Omar Iltaf

#include <ctype.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#define h_addr h_addr_list[0]

int main(int argc, char *argv[]) {
  // Server variables
  int socketFD, portNumber, charsWritten, charsRead, charsReadSoFar;
  struct sockaddr_in serverAddress;
  struct hostent *serverHostInfo;
  // Buffers set to large enough capacity
  char buffer[200000];
  char plaintextBuff[100000];
  char keyBuff[100000];
  // Character set used is this assignment
  char charSet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
  // Clearing out the buffers with null terminators
  memset(buffer, '\0', sizeof(buffer));
  memset(plaintextBuff, '\0', sizeof(plaintextBuff));
  memset(keyBuff, '\0', sizeof(keyBuff));

  // Checks usage & args
  if (argc < 4) {
    fprintf(stderr, "USAGE: %s hostname port\n", argv[0]);
    exit(1);
  }

  /////////////////////////////////////////////////////////////////////////////

  // Opening and storing contents of plaintext and key files
  FILE* fpPlaintext = fopen(argv[1], "r");
  FILE* fpKey = fopen(argv[2], "r");
  // Checks if files have been opened properly
  if (fpPlaintext == NULL) {
    fprintf(stderr, "otp_enc error: %s cannot be opened\n", argv[1]);
  } else if (fpKey == NULL) {
    fprintf(stderr, "otp_enc error: %s cannot be opened\n", argv[2]);
  }
  fgets(plaintextBuff, sizeof(plaintextBuff) - 1, fpPlaintext);
  fgets(keyBuff, sizeof(keyBuff) - 1, fpKey);

  // Checks if any bad characters exist in supplied plaintext file
  int badChar = 0;
  for (int i = 0; i < strlen(plaintextBuff); i++) {
    if (!(isupper(plaintextBuff[i]) || (plaintextBuff[i] == ' '))) {
      if (plaintextBuff[i] != '\n') {
        badChar = 1;
        break;
      }
    }
  }
  // If so, output error message to stderror
  if (badChar) {
    fprintf(stderr, "otp_enc error: input contains bad characters\n");
    exit(1);
  }
  // Outputs an error to stderror if the key is shorter than the plaintext
  if (strlen(keyBuff) < strlen(plaintextBuff)) {
    fprintf(stderr, "Error: key '%s' is too short\n", argv[2]);
    exit(1);
  }

  // Removes the trailing newline character in the plaintext
  plaintextBuff[strcspn(plaintextBuff, "\n")] = '\0';
  // Copies the plaintext into the buffer then concatenates the key after the
  // "@" used as a delimiter
  strcpy(buffer, plaintextBuff);
  strcat(buffer, "@");
  strcat(buffer, keyBuff);
  // A "$" symbol is used to identify this message as coming from otp_enc
  strcat(buffer, "$");

  /////////////////////////////////////////////////////////////////////////////

  // Setting up the server address struct
  // Clearing out the struct
  memset((char*)&serverAddress, '\0', sizeof(serverAddress));
  // Getting the port number and converting to an integer from a string
  portNumber = atoi(argv[3]);
  // Creating a network-capable socket
  serverAddress.sin_family = AF_INET;
  // Storing the port number
  serverAddress.sin_port = htons(portNumber);
  // Converting the machine name into a special form of address
  serverHostInfo = gethostbyname("localhost");
  if (serverHostInfo == NULL) {
    fprintf(stderr, "otp_enc error: No such host.\n");
    exit(1);
  }
  // Copying in the address
  memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr,
         serverHostInfo->h_length);

  // Creating and setting up the socket
  socketFD = socket(AF_INET, SOCK_STREAM, 0);
  if (socketFD < 0) {
    fprintf(stderr, "otp_enc error: Cannot open socket.\n");
    exit(1);
  }

  // Linking the socket and the address, then connecting to the server
  if (connect(socketFD, (struct sockaddr*)&serverAddress,
              sizeof(serverAddress)) < 0) {
    fprintf(stderr, "Error: could not contact otp_enc_d on port %s\n", argv[3]);
    exit(2);
  }

  /////////////////////////////////////////////////////////////////////////////

  // Sending the message to the server
  charsWritten = send(socketFD, buffer, strlen(buffer), 0);
  // If charsWritten does not equal the number of characters in buffer,
  // an error message will be output to stderror
  if (charsWritten < 0) {
    fprintf(stderr, "otp_enc error: Cannot write to socket.\n");
    exit(1);
  }
  if (charsWritten < strlen(buffer)) {
    fprintf(stderr, "otp_enc warning: Not all data written to socket.\n");
  }

  // Getting the return message from the server
  // Clearing out the buffer again for reuse
  memset(buffer, '\0', sizeof(buffer));
  charsRead = 0;
  // The while loop will continue until a "\n" character is read in the
  // return message
  while (strstr(buffer, "\n") == NULL) {
    // Reading data from the socket, leaving null terminator at the end
    if ((charsReadSoFar = recv(socketFD, buffer, sizeof(buffer) - 1, 0)) < 0) {
      break;
    } else {
      // Keeps the total number of characters read so far up-to-date
      charsRead += charsReadSoFar;
    }
  }
  // If charsRead is below 0, an error message is output to stderr
  if (charsRead < 0) {
    fprintf(stderr, "otp_enc error: Cannot read from socket.\n");
    exit(1);
  }

  // Checks if connection was rejected
  if (strchr(buffer, '!') != NULL) {
    fprintf(stderr, "%s\n", "Error: otp_enc cannot connect to otp_dec_d.");
    exit(1);
  }

  // Outputs return message to stdout
  fprintf(stdout, "%s", buffer);

  /////////////////////////////////////////////////////////////////////////////

  // Closing the socket
  close(socketFD);
  return 0;
}
