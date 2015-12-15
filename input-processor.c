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
		char* header[128];

		#if DEBUG
      		fprintf(stderr, "[thread %d] DEBUG: WAITING TO PARSE COMMAND.\n",
        	(int)pthread_self());
    	#endif
		
		GetCmd(header, 128, soc);
		
		#if DEBUG
      		fprintf(stderr, "[thread %d] DEBUG: HEADER: %s.\n", (int)pthread_self(), header[0]);
    	#endif

		char* instruction = header[0];
		
		if (instruction == NULL) {
			fprintf(stderr, "We done fucked up, command is invalid.");
			pthread_exit(NULL);
		}
		
		if( strcmp(instruction, "ADD") == 0 ) {
			//call add function
			Add(header, soc);
		}
		else if( strcmp(instruction, "READ") == 0 ) {
			//call read function
			Read(header, soc);
		}
		else if( strcmp(instruction, "DELETE") == 0 ) {
			//call delete function
			Delete(header, soc);
		}
		else if( strcmp(instruction, "DIR\n") == 0 ) {
			//call dir function
			Dir(header, soc);
		}
		else {
			fprintf(stderr, "Command <%s> is invalid.\n", instruction);
			pthread_exit(NULL);
		}

	}
}

void Add (char* command[], int soc) {
	#if DEBUG
      	fprintf(stderr, "[thread %d] DEBUG: Starting ADD: %s.\n", (int)pthread_self(), command[0]);
    #endif
}

void Read (char* command[], int soc) {
	#if DEBUG
      	fprintf(stderr, "[thread %d] DEBUG: Starting READ: %s.\n", (int)pthread_self(), command[0]);
    #endif
}

void Delete (char* command[], int soc) {
	#if DEBUG
      	fprintf(stderr, "[thread %d] DEBUG: Starting DELETE: %s.\n", (int)pthread_self(), command[0]);
    #endif
}

void Dir (char* command[], int soc) {
	#if DEBUG
      	fprintf(stderr, "[thread %d] DEBUG: Starting DIR: %s.\n", (int)pthread_self(), command[0]);
    #endif
}

void GetCmd(char* command[], int length, int soc) {
	//printf("Getting Command with length of %d\n", length);
	char *param, *str;
	char recString[128];
	char *savePtr;
	int j = read(soc, &recString, 128);
	printf("Got a return from read of -%d-\n", j);
	if (j <= 0) {
		Kill(soc);
		pthread_exit(NULL);
	}
	int i = 0;
	for( str = recString; ; str = NULL, i++) {
		param = strtok_r(str, " ", &savePtr);
		if( param == NULL ){
			command[i] = NULL;
			return;
		}
		command[i] = param;
		printf( "New Paramater --> %s\n", param);
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

int SocketIsOpen(int socket) {
	int i = send(socket, NULL, 1, 0);
	if (i < 0) {
		return 0;
	}
	else {
		return 1;
	}
}