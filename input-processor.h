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

void CreateInputProcess(pthread_t* p, int sID);

void *input(void *command);

void GetCmd(char* command, int length, int socket);

void Kill(int socket);

void SocketIsOpen(int socket);