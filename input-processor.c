#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <dirent.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <pthread.h>

#include "input-processor.h"

void CreateInputProcess(pthread_t* p, int socketID) {
	
	pthread_create(p, NULL, &Input, &socketID);
}

void *Input(void *command) {
	
	int soc = *(int*) command;
	//printf("Getting to Input() \n");
	while (1) {
		char* header[128];
		char fileName[128];
		#if DEBUG
      		fprintf(stderr, "[thread %d] DEBUG: WAITING TO PARSE COMMAND.\n",
        	(int)pthread_self());
    	#endif
		
		GetCmd(header, 128, soc);
		
		#if DEBUG
      		fprintf(stderr, "[thread %d] DEBUG: HEADER: %s\n", (int)pthread_self(), header[0]);
    	#endif

		char* instruction = header[0];
		
		if (instruction == NULL) {
			SendMessage("command is invalid.", soc);
		}
		else if( strcmp(instruction, "STORE") == 0 ) {
			//call add function
			//printf("%s | %s | %d", header[0], header[1], atoi(header[2]));
			strcpy(fileName, header[1]);
			Add(fileName, atoi(header[2])+1, soc);
		}
		else if( strcmp(instruction, "READ") == 0 ) {
			//call read function
			Read(header, soc);
		}
		else if( strcmp(instruction, "DELETE") == 0 ) {
			//call delete function
			strcpy(fileName, header[1]);
			Delete(fileName, soc);
		}
		else if( strcmp(instruction, "DIR\n") == 0 ) {
			//call dir function
			Dir(soc);
			//printf("WE MADE IT OUT ALIVE!\n");
		}
		else {
			fprintf(stderr, "Command <%s> is invalid.\n", instruction);
		}
	}
}

void Add (char* fileName, int fileSize, int soc) {
	#if DEBUG
      	printf("[thread %d] DEBUG: Starting ADD: %s with size: %d\n", (int)pthread_self(), fileName, fileSize);
    #endif

    // if(strspn(command[2], "0123456789") != strlen(*command[2])) {
    // 	printf("ERROR: %s is an invalid file size\n", *command[2]);
    // 	return;
    // }

    char* fileText = malloc(sizeof(char) * fileSize);
    FILE* file = fopen(fileName, "wb");

    read(soc, fileText, fileSize);
	fwrite(fileText, sizeof(char), fileSize, file);

    free(fileText);
    fclose(file);
    SendAckMessage(soc);
}

void Read (char* command[], int soc) {
	#if DEBUG
      	printf("[thread %d] DEBUG: Starting READ: %s\n", (int)pthread_self(), command[0]);
    #endif
}

void Delete (char* fileName, int soc) {
	#if DEBUG
      	printf("[thread %d] DEBUG: Starting DELETE: %s\n", (int)pthread_self(), fileName);
    #endif
	
	int status;
	
	status = remove(fileName);
	
	if (status == 0) {
		printf("[thread %d] File '%s' successfully deleted\n", (int)pthread_self(), fileName);
		SendAckMessage(soc);
	}
	else {
		fprintf(stderr, "[thread %d] ERROR: File couldn't be deleted\n", (int)pthread_self());
	}
	
	
}

void Dir (int soc) {
	#if DEBUG
      	printf("[thread %d] DEBUG: Starting DIR\n", (int)pthread_self());
    #endif
    DIR *storageDir;
	struct dirent *checkFile;
	storageDir = opendir("../.storage/");
	int numFiles = 0;
	// Counts the number of files in the directory
	if (storageDir != NULL) {
		while ( (checkFile = readdir(storageDir)) != NULL) {
			if (checkFile->d_type == DT_REG) {
				numFiles++;
			}
		}
		closedir(storageDir);
	}
	else {
    	fprintf(stderr, "[thread %d] ERROR: Invalid Directory '.storage/' ", (int)pthread_self());
    	return;
    }

	printf("%d\n", numFiles);
	char *files[numFiles];
	char *temp = NULL;
	numFiles = 0;
	storageDir = opendir("../.storage/");
	while ( (checkFile = readdir(storageDir)) != NULL) {
		if (checkFile->d_type == DT_REG) {
			strcpy(files[numFiles], checkFile->d_name);
			numFiles++;
		}
	}
	
	/*
	int i;
	for (i = 0; i < numFiles; ++i) {
		printf("%s", files[i]);
	}*/
	int i, j;
	for (i = 1; i < numFiles; ++i) {
		for (j = 1; j < numFiles; ++j) {
			if (strcmp(files[j - 1], files[j]) > 0) {
				strcpy(temp, files[j - 1]);
				strcpy(files[j - 1], files[j]);
				strcpy(files[j], temp);
			}
		}
	}

	char msg[2];
	sprintf(msg, "%d", numFiles);
	SendMessage(msg, soc);

	// for (i = 0; i < numFiles; ++i) {
	// 	printf("-->%s\n", files[i]);
	// }
	closedir(storageDir);
	chdir(".storage");
	//printf("LOOK AT ME, I'M MR MESEEKS\n");
	return;
	
}

void GetCmd(char* command[], int length, int soc) {
	//printf("Getting Command with length of %d\n", length);
	char *param, *str;
	char recString[128];
	char *savePtr;
	int j = read(soc, &recString, 128);
	if (j <= 0) {
		printf("[thread %d] Client closed its socket....terminating\n", (int)pthread_self());
		pthread_exit(NULL);
	}
	printf("[thread %d] Rcvd: %s", (int)pthread_self(), recString);
	int i = 0;
	for( str = recString; ; str = NULL, i++) {
		param = strtok_r(str, " ", &savePtr);
		if( param == NULL ){
			command[i] = NULL;
			return;
		}
		command[i] = param;
		//printf( "New Paramater --> %s\n", param);
	}

	// char bufChar = '\0';
	// int i = 0;
	// for (i = 0; i < length; ++i) {
	// 	int j = read(soc, &bufChar, 1);
	// 	if (j == 0) {
	// 		Kill(soc);
	// 		pthread_exit(NULL);
	// 	}
	// 	if (bufChar == '\n' || bufChar == '\r') {
	// 		break;
	// 	}
	// 	command[i] = bufChar;
	// }
	// command[i] = '\0';
}

void Kill(int socket){
	if (SocketIsOpen(socket) == 1) {
		printf("Killing Socket\n");
		close(socket);
		pthread_exit(NULL);
	}
}

void SendMessage(char* message, int soc) {
  // fprintf(stderr, "[thread %d] %s", (int)pthread_self(), message);
  send(soc, message, strlen(message), 0);
}

void SendAckMessage(int soc) {
  send(soc, "ACK\n", strlen("ACK\n"), 0);
  printf("[thread %d] Sent: ACK\n", (int)pthread_self());
}

int SocketIsOpen(int socket) {
	int i = send(socket, NULL, 1, 0);
	if (i < 0) {
		return 0;
	}
	else {
		return 1;
	}
}