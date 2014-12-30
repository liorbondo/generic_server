/***************************************
Author: 			 			Lior Bondorevsky
Creation date: 			19/11/2014
Last modofied date: 28/11/2014
Description: 				TCP clients loop
***************************************/
#include <string.h> /* strlen */
#include <time.h>   /* time */
#include <stdlib.h> /* system */
#include <signal.h> /* siaction */
#include <stdio.h>  /* perror */
#include <errno.h>  /* errno */
#include <unistd.h> /* close, usleep */
#include <sys/socket.h> /* socket */
#include <netinet/in.h> /* sockaddr_in */
#include <arpa/inet.h>  /* inet_pton */
#include <sys/fcntl.h>  /* fnctl */
#include <sys/select.h> /* select */
#include <sys/resource.h> /* setrlimit */


#define CONNECT_PERCENTAGE 3 /* 1/3 = 33% */
#define SEND_PERCENTAGE    2 /* 1/2 = 50% */

#define SYS_ERR       -1
#define OK             0
#define FALSE          0
#define TRUE           1
#define ERR            1
#define SERVER_PORT    5060

#define DATA_SIZE      64
#define BUFFER_SIZE    80
#define SLEEP_TIME     1000000 /* in usec */
#define CLIENTS_NUM    500

#define ILYA_IP        "192.168.1.105"
#define EYAL_IP        "192.168.1.72"
#define BENNY_IP       "192.168.1.59"
#define LOOPBACK_IP    "127.0.0.1"

#define SERVER_IP      LOOPBACK_IP 


int main()
{
  int clientSocket[CLIENTS_NUM];
  int bytesWritten;
  int clientIndex;
  
  struct rlimit rl;
  
  struct sockaddr_in serverAddr;
  
  char   data[DATA_SIZE];
  int    dataLen;
  char   buffer[BUFFER_SIZE];
  
  /* Init serverAddr */
  memset(&serverAddr, 0, sizeof(serverAddr));
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port   = htons(SERVER_PORT);
  inet_pton(AF_INET, SERVER_IP, &serverAddr.sin_addr); 
  
  /* Init random seed */
  srand(time(NULL));
  
  /* Set open files limit */
  rl.rlim_cur = CLIENTS_NUM + 10;
  rl.rlim_max = CLIENTS_NUM + 10;

  if(SYS_ERR == setrlimit(RLIMIT_NOFILE, &rl))
  {
    perror("setrlimit");
    return errno;
  }
 
  /* Clients loop */
  for(clientIndex = 0; clientIndex < CLIENTS_NUM; ++clientIndex)
  { 
    /* Init client socket */
    clientSocket[clientIndex] = socket(AF_INET, SOCK_STREAM, 0);
    if(SYS_ERR == clientSocket[clientIndex])
    {
      perror("socket");
      return errno;
    }
    
    /* Connect to server --> only CONNECT_PERCENTAGE of the clients */
    if(!(rand() % CONNECT_PERCENTAGE))
    {
      if(SYS_ERR == connect(clientSocket[clientIndex], (const struct sockaddr*)&serverAddr, sizeof(serverAddr)))
      {
        perror("connect");
        return errno;
      }
      
      /* send to server --> only SEND_PERCENTAGE of the clients */
      if(!(rand() % SEND_PERCENTAGE))
      {

        /* Write message */
        sprintf(data, "hello server! its client #%d", clientIndex + 1);
        dataLen = strlen(data) + 1;
        bytesWritten = write(clientSocket[clientIndex], data, dataLen);
        
        if(SYS_ERR == bytesWritten || bytesWritten != dataLen)
        {
          perror("write");
          return errno;
        }
       
        printf("\nclient sent: %s\n", data);
            
        /* Recieve response */
        if(SYS_ERR == read(clientSocket[clientIndex], buffer, sizeof(buffer)))    
        {
          perror("read");
          return errno;
        }
        printf("client recieved: %s\n", buffer);
        
      }
      /* delay */
      usleep(SLEEP_TIME);
    }
  } /* for(...) */
  
  /* Close all client sockets */
  for(clientIndex = 0; clientIndex < CLIENTS_NUM; ++clientIndex)
  {
    if(SYS_ERR == close(clientSocket[clientIndex]))
    {
      perror("close");
    }
  }
	return OK;
}
