#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <pthread.h>

#ifndef inputprocessor_h
#define inputprocessor_h

void CreateInputProcess(pthread_t* p, int socketID);

void *Input(void *command);

void Add (char* fileName, int fileSize, int soc);

void Read (char* command[], int soc);

void Delete (char* fileName, int soc);

void Dir (int soc);

void GetCmd(char* command[], int length, int soc);

void Kill(int soc);

int SocketIsOpen(int soc);

void SendMessage(char* message, int soc);

void SendAckMessage(int soc);

#endif