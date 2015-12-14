#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>

#define PORT 8128

int main( int argc, char* argv[] ){

  int sock;
  unsigned short port;
  struct sockaddr_in server;
  struct hostent *hp;

  //Creating socket
  sock = socket (PF_INET, SOCK_STREAM, 0);
  if(sock < 0){
    perror( "socket()");
    exit(1);
  }

  //Connecting to host
  server.sin_family = PF_INET;
  hp = gethostbyname( "localhost" );  //local host
  if( hp == NULL){
    perror ("Unknown host");
    exit(1);
  }

  /* could also use memcpy */
  bcopy( (char *)hp->h_addr, (char *)&server.sin_addr, hp->h_length );
  port = PORT;
  server.sin_port = htons( port );

  //Connecting
  if ( connect( sock, (struct sockaddr *)&server, sizeof( server) ) < 0 ) {
    perror( "connect()" );
    exit( 1 );
  }

  int i = 0;
  for (i = 0; i < 2; ++i) {
    char* command = "ADD hello.txt 3\nABC";
    send(sock, command, strlen(command), 0);
  }

  //Close connection
  close(sock);
  return 0;
}
