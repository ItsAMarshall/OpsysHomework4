/**
 * Lucas Silva
 * silval@rpi.edu
 * Operating Systems HW 3
 * @file The implementation file for each thread.
 *       Actually does the work of handling the work of manipulating
 *       the files on the server.
 */

#include "headers/thread-handler.h"
#include "headers/filelist.h"

#define DEBUG 0

/**
 * Create a new thread to handle a client that has connected to the server.
 */
void CreateThreadHandler(pthread_t* pthread_buf, int newSocketId) {
  // Create a new thread, and pass the sock to it
  pthread_create(pthread_buf, NULL, &thread, &newSocketId);
}

/**
 * thread The function executed by each thread. This handles the
 *   adding, removing, and appending of files to and from the server.
 * @param  arg The socket.
 * @return     NULL
 */
void *thread(void *arg) {
  // The socket this client is connected to.
  int sock = *(int*) arg;

  // Keep getting commands until the socket connection
  // closes.
  while (1) {
    char header[BUFFER_SIZE];
    char fileName[BUFFER_SIZE];

    #if DEBUG
      fprintf(stderr, "[thread %lu] DEBUG: WAITING TO PARSE COMMAND.\n",
        pthread_self());
    #endif

    // Parse the command and place it into the header array.
    GetHeader(header, BUFFER_SIZE, sock);

    #if DEBUG
      fprintf(stderr, "[thread %lu] DEBUG: HEADER: %s.\n", pthread_self(), header);
    #endif

    // Read the command from the header.
    char* saveptr = NULL;
    char* command = strtok_r(header, " \r\n", &saveptr);

    // Check if command is invalid
    if (command == NULL) {
      fprintf(stderr, "[thread %lu] ERROR: COMMAND INVALID.\n", pthread_self());
      continue;
    }

    #if DEBUG
      fprintf(stderr, "[thread %lu] DEBUG: COMMAND: %s.\n", pthread_self(),
        command);
    #endif

    // If command is ADD, then handle the adding of the file.
    if (strcmp(command, "ADD") == 0) {
      #if DEBUG
        fprintf(stderr, "[thread %lu] DEBUG:  COMMAND RECOGNIZED AS ADD.\n",
          pthread_self());
      #endif
      // If the file name is present, get it.
      if (GetFileName(&fileName, &saveptr, sock)) {
        #if DEBUG
          fprintf(stderr, "[thread %lu] DEBUG: FILENAME:%s-\n",
            pthread_self(), fileName);
        #endif
        // Get the size of the file in bytes.
        int size = GetFileSize(&saveptr, sock);
        // If size is present and valid, then procceed with processing
        // the command.
        #if DEBUG
          fprintf(stderr, "[thread %lu] DEBUG: SIZE:%d\n",
            pthread_self(), size);
        #endif

        if (size >= 0) {
          printf("[thread %lu] Rcvd: %s %s %d\n", pthread_self(), command,
            fileName, size);
          // Enter critical section for adding the file to the list.
          // Only one thread can modify the list at a time.
          pthread_mutex_lock(&listMutex);
          // If the file already exists, then don't do anything.
          if (FileExists(fileName, sock)) {
          	pthread_mutex_unlock(&listMutex);
            continue;
          }
          // Add the file to the file list.
          addFileToList(fileName, size);
          #if DEBUG
          fprintf(stderr, "[thread %lu] DEBUG: ADDED %s TO THE LIST.\n",
            pthread_self(), fileName);
          #endif
          // Get the file from the master list.
          fileStruct* file = getFile(fileName);
          // End of critical section for the list mutex. Can now release
          // the list for other threads to add other files.
          pthread_mutex_unlock(&listMutex);
          // Lock the file, so only this thread has access to it.
          pthread_mutex_lock(&(file->lock));
          // Add the file to the storage directory.
          #if DEBUG
          fprintf(stderr, "[thread %lu] DEBUG: ENTER ADD COMMAND HANDLER.\n",
            pthread_self());
          #endif
          HandleAddCommand(fileName, size, sock);
          // Release the file to other threads.
          pthread_mutex_unlock(&(file->lock));
          }
        }
      } else if (strcmp(command, "APPEND") == 0) {
        #if DEBUG
          fprintf(stderr, "[thread %lu] DEBUG: COMMAND RECOGNIZED AS APPEND.\n",
            pthread_self());
        #endif
        // If the file name is present, get it.
        if (GetFileName(&fileName, &saveptr, sock)) {
          // Get the size of the content to be appended.
          #if DEBUG
            fprintf(stderr, "[thread %lu] DEBUG: FILENAME: %s\n",
              pthread_self(), fileName);
          #endif
          int size = GetFileSize(&saveptr, sock);

          #if DEBUG
          fprintf(stderr, "[thread %lu] DEBUG: SIZE:%d\n",
            pthread_self(), size);
          #endif
          // If size is present and valid, then procceed with processing
          // the command.
          if (size >= 0) {
            // Print the received command message.
            printf("[thread %lu] Rcvd: %s %s %d\n", pthread_self(), command,
              fileName, size);
            fileStruct* file = getFile(fileName);
            if (file == NULL) {
              PrintAndSendMessage("ERROR: FILE DOES NOT EXIST\n", sock);
              continue;
            }
            #if DEBUG
              fprintf(stderr, "[thread %lu] DEBUG: ENTER APPEND COMMAND HANDLER\n",
              pthread_self());
            #endif
            // Enter critical section for append. While this thread is appending
            // to the file, no other threads may be appending to it.
            pthread_mutex_lock(&(file->lock));
            HandleAppendCommand(fileName, size, sock);
            pthread_mutex_unlock(&(file->lock));
          }
        }
      } else if (strcmp(command, "READ") == 0) {
        // If the file name is present, get it.
        if (GetFileName(&fileName, &saveptr, sock)) {
          printf("[thread %lu] Rcvd: %s %s\n", pthread_self(), command,
            fileName);
          if (fileName[strlen(fileName) - 1] == '\n' ||
            fileName[strlen(fileName) - 1] == ' ' ) {
            fileName[strlen(fileName) - 1] = '\0';
          }
          fileStruct* file = getFile(fileName);
          if (file != NULL) {
            HandleReadCommand(fileName, sock);
          } else {
            PrintAndSendMessage("ERROR: FILE DOES NOT EXIST\n", sock);
          }
        }
      } else if (strcmp(command, "LIST") == 0) {
        // Run through the master list of files, and
        // print them out.
        printf("[thread %lu] Rcvd: %s\n", pthread_self(), command);
        char numFilesAsString[BUFFER_SIZE];
        sprintf(numFilesAsString, "%d\n", numFiles);
        send(sock, numFilesAsString, strlen(numFilesAsString), 0);
        int i = 0;
        pthread_mutex_lock(&listMutex);
        fileStruct* file = fileList;
        for (i = 0; i < numFiles; ++i) {
          char name[BUFFER_SIZE];
          sprintf(name, "%s\n", file->name);
          send(sock, name, strlen(name), 0);
          file = file->next;
        }
        pthread_mutex_unlock(&listMutex);
      } else if (strcmp(command, "DELETE") == 0) {
        if (GetFileName(&fileName, &saveptr, sock)) {
          fileStruct* file = getFile(fileName);
          if (file == NULL) {
            PrintAndSendMessage("ERROR: FILE NOT FOUND\n", sock);
          } else {
            if (file->readersCount > 0) {
              while(file->readersCount > 0) {}
            }
            pthread_mutex_lock(&listMutex);
            if (remove(fileName) < 0) {
              PrintAndSendMessage("ERROR: REMOVE FAILED\n", sock);
            } else {
              deleteFile(fileName);
              SendAcknowledgeMessage(sock);
            }
            pthread_mutex_unlock(&listMutex);
          }
        }
      } else {
        PrintAndSendMessage("ERROR: COMMAND UNKNOWN\n", sock);
        continue;
      }
    }
}

/**
 * Handles the adding of files to the storage directory.
 * @param fileName The name of the file to be added.
 * @param size The size of the file in bytes.
 * @param sock The socket file descriptor.
 */
void HandleAddCommand(char* fileName, int size, int sock) {
  char* fileContent = malloc(sizeof(char)*(size+1));
  // Read the content of the file from the socket.
  int rc = read(sock, fileContent, size);
  if (rc <= 0) {
    TerminateSocketAndExit(sock);
    return;
  }
  FILE* newFile = fopen(fileName, "wb");
  if (newFile == NULL) {
    PrintAndSendMessage("ERROR: COULD NOT OPEN NEW FILE\n", sock);
  } else if (!fwrite(fileContent, sizeof(char), size, newFile)) {
    PrintAndSendMessage("ERROR: COULD NOT WRITE INTO NEW FILE\n", sock);
  } else {
    printf("[thread %lu] Transferred file (%d bytes)\n", pthread_self(), size);
    SendAcknowledgeMessage(sock);
  }
  free(fileContent);
  fclose(newFile);
}

/**
 * [HandleAppendCommand description]
 * @param fileName [description]
 * @param size     [description]
 * @param sock     [description]
 */
void HandleAppendCommand(char* fileName, int size, int sock) {
  char* fileContent = malloc(sizeof(char)*(size+1));
  // Parse the command.
  int rc = read(sock, fileContent, size);
  if (rc <= 0) {
    TerminateSocketAndExit(sock);
    return;
  }
  FILE* newFile = fopen(fileName, "a");
  if (newFile == NULL) {
    PrintAndSendMessage("ERROR: COULD NOT OPEN FILE\n", sock);
  } else if (!fwrite(fileContent, sizeof(char), size, newFile)) {
    PrintAndSendMessage("ERROR: COULD NOT WRITE INTO FILE\n", sock);
  } else {
    printf("[thread %lu] Transferred file (%d bytes)\n", pthread_self(), size);
    SendAcknowledgeMessage(sock);
  }
  fileStruct* file = getFile(fileName);
  file->size += size;
  free(fileContent);
  fclose(newFile);
}

/**
 * HandleReadCommand takes in the name of the file
 *   and the socket, and then outputs the contents of the file.
 * @param fileName The name of the file.
 * @param sock     The socket.
 */
void HandleReadCommand(char* fileName, int sock) {
  fileStruct* file = getFile(fileName);
  pthread_mutex_lock(&(file->lock));
  file->readersCount++;
  pthread_mutex_unlock(&(file->lock));
  int size = file->size;
  FILE* newFile = fopen(fileName, "rb");
  char* fileContent = malloc(sizeof(char)*(size+1));
  pthread_mutex_lock(&(file->lock));
  if (newFile == NULL) {
    PrintAndSendMessage("ERROR: COULD NOT OPEN FILE\n", sock);
  } else if (!fread(fileContent, sizeof(char), size, newFile)) {
    PrintAndSendMessage("ERROR: COULD NOT READ FILE\n", sock);
  } else {
    SendAcknowledgeMessage(sock);
    char sizeAsString[BUFFER_SIZE];
    sprintf(sizeAsString, "%d\n", size);
    send(sock, sizeAsString, strlen(sizeAsString), 0);
    send(sock, fileContent, size, 0);
    send(sock, "\n", sizeof("\n"), 0);
    printf("[thread %lu] Transferred file (%d bytes)\n",
      pthread_self(), size);
  }
  pthread_mutex_unlock(&(file->lock));
  free(fileContent);
  fclose(newFile);
}

/**
 * PrintAndSendMessage takes in a message and a socket
 *   and prints the message with the thread info in the server log,
 *   while also sending the message to the client.
 * @param message The message string to be printed.
 * @param sock    The socket to send the message to.
 */
void PrintAndSendMessage(char* message, int sock) {
  fprintf(stderr, "[thread %lu] %s", pthread_self(), message);
  send(sock, message, strlen(message), 0);
}


/**
 * GetFileName gets the file name from the user command
 *   and checks if the file already exists.
 * @param  fileName The place where the name of the file wil go.
 * @param  saveptr  The saveptr for the strtok_r call.
 * @return          1 if file name successfully retrieved, false otherwise.
 */
int GetFileName(char (*fileName)[], char** saveptr, int sock) {
  strncpy(*fileName, strtok_r(NULL, " \r\n", saveptr), BUFFER_SIZE);
  (*fileName)[strlen(*fileName)] = '\0';
  if (*fileName == NULL) {
    PrintAndSendMessage("ERROR: COULD NOT READ FILENAME\n", sock);
    return 0;
  }
  return 1;
}

void SendAcknowledgeMessage(int sock) {
  send(sock, "ACK\n", strlen("ACK\n"), 0);
  printf("[thread %lu] Sent: %s\n", pthread_self(), "ACK");
}

/**
 * FileExists checks if a file already exists in the directory.
 * @param  fileName The name of the file to be checked.
 * @param  sock     The socket to relay messages back to.
 * @return          1 if file exists, 0 otherwise.
 */
int FileExists(char fileName[], int sock) {
  if (getFile(fileName) != NULL) {
    PrintAndSendMessage("ERROR: FILE ALREADY EXISTS\n", sock);
    return 1;
  }
  return 0;
}

/**
 * Gets the size of the file in bytes.
 * @param  saveptr The saveptr for the strtok_r call.
 * @param  sock    The socket to relay messages back to the client.
 * @return         The size of the file, or -1 if error occurs.
 */
int GetFileSize(char** saveptr, int sock) {
  char* temp = strtok_r(NULL, " \n", saveptr);
  if (temp == NULL) {
    PrintAndSendMessage("ERROR: BYTE SIZE NEEDED\n", sock);
    return -1;
  }
  // The number of bytes this file should be.
  int size = atol(temp);
  if (size < 0) {
    PrintAndSendMessage("ERROR: BYTE SIZE MUST BE NON-NEGATIVE\n", sock);
    return -1;
  }
  return size;
}

void GetHeader(char* header, int size, int sock) {
  char buffer = '\0';
  int i = 0;
  for (i = 0; i < BUFFER_SIZE; ++i) {
    int rc = read(sock, &buffer, 1);
    if (rc <= 0) {
      TerminateSocketAndExit(sock);
      return;
    }
    if (buffer == '\n' || buffer == '\r') {
      break;
    }
    header[i] = buffer;
    #if DEBUG
      fprintf(stderr, "[thread %lu] DEBUG: READ IN:%c\n", pthread_self(), buffer);
    #endif
  }
  header[i] = '\0';
  // header now contains command, bytes, and filename.
}


void TerminateSocketAndExit(int sock) {
  if (!IsSocketOpen(sock)) {
    fprintf(stderr, "[thread %lu] Client closed it's socket...terminating\n",
      pthread_self());
    close(sock);
    pthread_exit(NULL);
  }
}


int IsSocketOpen(int sock) {
  int open = send(sock, '\0', 1, MSG_NOSIGNAL);
  if (open < 0) {
    return 0;
  } else {
    return 1;
  }
}
