/***************************************
Author: 			 			Lior Bondorevsky
Creation date: 			19/11/2014
Last modofied date: 19/11/2014
Description: 				TCP server
***************************************/
#include <string.h> /* strlen */
#include <stdlib.h> /* system */
#include <stdio.h>  /* perror */
#include <errno.h>  /* errno */
#include <unistd.h> /* close, sleep */
#include <sys/socket.h> /* socket */
#include <netinet/in.h> /* sockaddr_in */
#include <arpa/inet.h>  /* inet_pton */

#define SYS_ERROR  -1
#define OK          0
#define TRUE        1
#define SERVER_PORT 5060
#define SERVER_IP   "127.0.0.1"

#define BUFFER_SIZE 80

int main()
{
  int clientSocket;
  int byteWrite;
  
  struct sockaddr_in serverAddr;
  
  char data[] = "hello server, its client #9\n";
  size_t dataLen = strlen(data) + 1;
  char buffer[BUFFER_SIZE];
  
  clientSocket = socket(AF_INET, SOCK_STREAM, 0);
  if(SYS_ERROR == clientSocket)
  {
    perror("socket");
    return errno;
  }
  
  /* Init serverAddr */
  memset(&serverAddr, 0, sizeof(serverAddr));
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port   = htons(SERVER_PORT);
  inet_pton(AF_INET, SERVER_IP, &serverAddr.sin_addr);
  
  /* Sent data to server */
  if(SYS_ERROR == connect(clientSocket, (const struct sockaddr*)&serverAddr, sizeof(serverAddr)))
  {
    perror("connect");
    return errno;
  }
  
  
  /* Write message */
  byteWrite = write(clientSocket, data, dataLen);
  if(SYS_ERROR == byteWrite || byteWrite != dataLen)
  {
    perror("write");
    return errno;
  }
  printf("client sent: %s\n", data);
  
  
  /* Recieve response */
  if(SYS_ERROR == read(clientSocket, buffer, sizeof(buffer)))
  {
    perror("read");
    return errno;
  }
  printf("client recieved: %s\n", buffer);
  
  usleep(5000000);
  
  if(SYS_ERROR == close(clientSocket))
  {
    perror("close");
    return errno;
  }
  
	return OK;
}
