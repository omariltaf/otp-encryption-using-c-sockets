// Omar Iltaf

#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  // Server variables
  int listenSocketFD, establishedConnectionFD, portNumber, charsWritten,
      charsRead, charsReadSoFar;
  struct sockaddr_in serverAddress, clientAddress;
  socklen_t sizeOfClientInfo;
  // Buffers set to large enough capacity
  char buffer[200000];
  char plaintextBuff[100000];
  char keyBuff[100000];
  char *token = malloc(sizeof(char) * 200000);
  // Clearing out the buffers with null terminators
  memset(buffer, '\0', sizeof(buffer));
  memset(plaintextBuff, '\0', sizeof(plaintextBuff));
  memset(keyBuff, '\0', sizeof(keyBuff));
  memset(token, '\0', sizeof(token));

  // Checks usage & args
  if (argc < 2) {
    fprintf(stderr, "USAGE: %s port\n", argv[0]);
    exit(1);
  }

  /////////////////////////////////////////////////////////////////////////////

  // Setting up the address struct for this process (the server)
  // Clearing out the struct
  memset((char *)&serverAddress, '\0', sizeof(serverAddress));
  // Getting the port number and converting to an integer from a string
  portNumber = atoi(argv[1]);
  // Creating a network-capable socket
  serverAddress.sin_family = AF_INET;
  // Storing the port number
  serverAddress.sin_port = htons(portNumber);
  // Any address is allowed to connect to this process
  serverAddress.sin_addr.s_addr = INADDR_ANY;

  // Creating and setting up the socket
  listenSocketFD = socket(AF_INET, SOCK_STREAM, 0);
  if (listenSocketFD < 0) {
    fprintf(stderr, "otp_enc_d error: Cannot open socket.\n");
    exit(1);
  }

  // Enabling the socket to begin listening by connecting it to the port
  if (bind(listenSocketFD, (struct sockaddr *)&serverAddress,
           sizeof(serverAddress)) < 0) {
    fprintf(stderr, "otp_enc_d error: Cannot bind socket.\n");
    exit(1);
  }
  // Flipping the socket on - it can now receive up to 5 connections
  listen(listenSocketFD, 5);

  /////////////////////////////////////////////////////////////////////////////

  // Process forking variables
  pid_t spawnpid = -5;
  pid_t wpid = -5;
  int childExitStatus = -5;
  static int counter = 0;

  while (counter < 5) {
    // Getting the size of the address for the client that will connect
    sizeOfClientInfo = sizeof(clientAddress);
    // Accepts a connection, blocking if one is not available until one connects
    establishedConnectionFD = accept(
        listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo);
    if (establishedConnectionFD < 0) {
      fprintf(stderr, "otp_enc_d error: Cannot accept connection.\n");
    }

    // Checks for and clears out any zombie processes and adjusts counter,
    // so no more than five child processes are running at any given time
    wpid = waitpid(-1, &childExitStatus, WNOHANG);
    if (wpid > 0) {
      counter--;
    }

    ///////////////////////////////////////////////////////////////////////////

    // Creates a new child process
    spawnpid = fork();
    switch (spawnpid) {
    case -1:
      // Outputs error message to stderror if process creation failed
      fprintf(stderr, "Error: Child process creation failed.\n");
      close(establishedConnectionFD);
      break;
    case 0:
      // Now in the new child process
      // Incrementing the counter to control total number of child processes
      counter++;
      // Closes the listening socket as it is unneeded in this process
      close(listenSocketFD);

      // The while loop will continue until a "\n" character is read in the
      // message
      while (strstr(buffer, "\n") == NULL) {
        // Reading data from the socket, leaving null terminator at the end
        if ((charsReadSoFar = recv(establishedConnectionFD, buffer,
                                   sizeof(buffer) - 1, 0)) < 0) {
          break;
        } else {
          charsRead += charsReadSoFar;
        }
      }
      // If charsRead is below 0, an error message is output to stderr
      if (charsRead < 0) {
        fprintf(stderr, "otp_enc_d error: Cannot read from socket.\n");
        goto A;
      }

      // Checks if the message came from otp_enc
      // If not, sends back rejection message to client and the process exits
      if (strchr(buffer, '$') == NULL) {
        charsWritten = send(establishedConnectionFD, "!\n", 2, 0);
        // If charsRead is below 0, an error message is output to stderr
        if (charsWritten < 0) {
          fprintf(stderr, "otp_enc_d error: Cannot write to socket.\n");
          goto A;
        }
        goto A;
      }

      /////////////////////////////////////////////////////////////////////////

      // Begins handling the message from the client
      // Obtaining the key and plaintext using tokens
      token = strtok(buffer, "@");
      strcpy(plaintextBuff, token);
      token = strtok(NULL, "@");
      strcpy(keyBuff, token);
      // Removes the trailing "\n" character from the key buffer
      keyBuff[strcspn(keyBuff, "\n")] = '\0';

      /////////////////////////////////////////////////////////////////////////

      // Variables used as part of encryption process
      char *charPT;
      char *charKey;
      char charCT;
      int indexPT;
      int indexKey;
      int indexCT;
      char *charSet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
      char ciphertext[100000];
      // Clearing out the ciphertext buffer
      memset(ciphertext, '\0', sizeof(ciphertext));

      // Loops through and applies the encryption algorithm to each character
      // of plaintext
      for (int i = 0; i < strlen(plaintextBuff); i++) {
        charPT = strchr(charSet, plaintextBuff[i]);
        indexPT = (int)(charPT - charSet);

        charKey = strchr(charSet, keyBuff[i]);
        indexKey = (int)(charKey - charSet);

        indexCT = (indexPT + indexKey) % 27;
        charCT = charSet[indexCT];
        ciphertext[strlen(ciphertext)] = charCT;
      }
      // Adds a trailing "\n" character to the ciphertext buffer
      ciphertext[strlen(ciphertext)] = '\n';

      // Sends the encrypted plaintext (ciphertext) back to the client
      charsWritten =
          send(establishedConnectionFD, ciphertext, strlen(ciphertext), 0);
      if (charsWritten < 0) {
        fprintf(stderr, "otp_enc_d error: Cannot write to socket.\n");
        goto A;
      }

    // The goto statement used above will link to this point
    A:
      // Close the existing socket which is connected to the client
      close(establishedConnectionFD);
      // Free any allocated memory
      // free(token);
      // Exit the child process successfully
      exit(0);
      break;
    default:
      // Incrementing the counter to control total number of child processes
      counter++;
      // Closing the connection socket as it's unneeded in the parent process
      close(establishedConnectionFD);
      break;
    }
  }

  /////////////////////////////////////////////////////////////////////////////

  // Closing the listening socket
  close(listenSocketFD);
  return 0;
}
