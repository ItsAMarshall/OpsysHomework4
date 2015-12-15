/**
 * Lucas Silva
 * silval@rpi.edu
 * Operating Systems HW 3
 * @file This is the main entry point into the server,
 *       contains the main function that handles
 *       incoming connections and hands them off
 *       to threads.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <pthread.h>

#include "headers/thread-handler.h"

static int socketId = -1;
static int port = -1;
// Socket struct, found in socket.h
static struct sockaddr_in server;

// Constants
const unsigned int PORT_MAX = 9000;
const unsigned int PORT_MIN =  8000;
const char* FILE_STORAGE_DIRECTORY = ".storage";

/**
 * InitializeServer creates a socket and binds the server to it.
 * @param  portNumber The port number that the user
 *                    would like to make the server at.
 * @return            If successful, the value of sizeof(server),
 *                             else it is -1 on error.
 */
int InitializeServer(const int portNumber) {
  if (portNumber < PORT_MIN || portNumber > PORT_MAX) {
    fprintf(stderr, "ERROR: port number must be between %d and %d\n",
      PORT_MIN, PORT_MAX);
    return -1;
  }
  // Set the port to the port given by the user.
  port = portNumber;
  // Create TCP socket to listen for incoming connections.
  socketId = socket(AF_INET, SOCK_STREAM, 0);
  // Check for error in creating the socket.
  if (socketId < 0) {
    perror("socket() failed");
    return -1;
  }
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = INADDR_ANY;
  // Data marshalling for big endian vs little endian.
  server.sin_port = htons(port);
  unsigned int len = sizeof(server);
  // Try binding the socket.
  if (bind(socketId, (struct sockaddr *)&server, len) < 0) {
    perror("bind() failed");
    return -1;
  }
  // 10 is the maximum number of backlogged waiting clients.
  listen(socketId, 10);
  printf("Started file-server\n" "Listening on port %d\n", port);
  return (int)len;
}

/**
 * CreateStorageDirectory creates the directory to store all the files
 *   given to the server.
 * @param  fd The file descriptor for the directory.
 * @return    -1 if failure, 0 if success.
 */
int CreateStorageDirectory(char* fd) {
  struct stat fileStatus;
  int successMkDir;
  // If the directory does not already exist...
  if (stat(fd, &fileStatus) < 0) {
    successMkDir = mkdir(".storage", 0700);
    if (successMkDir != 0) {
      fprintf(stderr, "ERROR: unable to make storage directory\n");
      return -1;
    }
  }
  chdir(".storage");
  return 0;
}

/**
 * The main entry point into the program. Handles creation
 *   of server and incoming connections.
 * @param  argc The number of command line arguments.
 * @param  argv The arguments vector containing all arguments.
 * @return      EXIT_SUCESS if successful, EXIT_FAILURE otherwise.
 */
int main(int argc, char* argv[]) {
  // Check for valid command. If there is no port,
  // display error message.
  if ( argc != 2 ) {
    fprintf(stderr, "USAGE: %s [PORT]\n", argv[0]);
    return EXIT_FAILURE;
  }

  // Used to create the storage directory.
  char *fd = ".storage";
  // Try to create storage directory, handling any failures
  // if they occur.
  if (CreateStorageDirectory(fd) < 0) {
    fprintf(stderr, "ERROR: could not create storage directory.");
    return EXIT_FAILURE;
  }

  // Initialize the server and create the socket listener.
  socklen_t lenOfServer = (socklen_t)InitializeServer(atoi(argv[1]));
  // If we could not initialize server, exit the program.
  if (lenOfServer < 0) {
    fprintf(stderr, "ERROR: could not initialize server. Exiting...");
    return EXIT_FAILURE;
  }

  while (1) {
    // Blocks and waits to connect to a client.
    int newSocket = accept(socketId, (struct sockaddr *)&server, &lenOfServer);
    // Check for an error in connection.
    if (newSocket < 0) {
      fprintf(stderr, "ERROR: could not accept socket connection!\n");
    }
    printf("Received incoming connection from <%s>\n",
      inet_ntoa(server.sin_addr));

    static pthread_t thread;
    // Create a new thread to handle that client.
    CreateThreadHandler(&thread, newSocket);
  }
  return EXIT_SUCCESS;
}
