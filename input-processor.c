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

#include "input-processor.h"

void CreateInputProcess(pthread_t* p, int socketID) {
	
	pthread_create(p, NULL, &Input, &socketID);
}

void *Input(void *command) {
	
	int soc = *(int*) command;
	printf("Getting to Input() \n");
	while (1) {
		char header[128];
		char fileName[128];

		#if DEBUG
      		fprintf(stderr, "[thread %lu] DEBUG: WAITING TO PARSE COMMAND.\n",
        	pthread_self());
    	#endif
		
		GetCmd(header, 128, soc);
		
		#if DEBUG
      		fprintf(stderr, "[thread %lu] DEBUG: HEADER: %s.\n", pthread_self(), header);
    	#endif

		char* temp = NULL;
		char* instructions = strtok_r(header, " \r\n", &temp);
		
		if (command == NULL) {
			fprintf(stderr, "We done fucked up, command is invalid.");
		}
		else {
			printf("Command inputted: %s\n", instructions);
		}
	}
}

void GetCmd(char* command, int length, int soc) {
	//printf("Getting Command with length of %d\n", length);
	char bufChar = '\0';
	int i = 0;
	for (i = 0; i < length; ++i) {
		int j = read(soc, &bufChar, 1);
		if (j == 0) {
			Kill(soc);
			pthread_exit(NULL);
		}
		if (bufChar == '\n' || bufChar == '\r') {
			break;
		}
		command[i] = bufChar;
	}
	command[i] = '\0';
}

void Kill(int socket){
	if (SocketIsOpen(socket) == 1) {
		printf("Killing Socket\n");
		close(socket);
		pthread_exit(NULL);
	}
}

int SocketIsOpen(int socket) {
	int i = send(socket, '\0', 1, 0);
	if (i < 0) {
		return 0;
	}
	else {
		return 1;
	}
}