#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "ne.h"
#include "router.h"

typedef struct sockaddr SA; //global, need to set type in bind call

int open_udpfd(int port)
{
	int udpfd, optval = 1;	
	struct sockaddr_in serveraddr;

	if((udpfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		return -1; //error handling
	}

	if(setsockopt(udpfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval, sizeof(int)) < 0)
	{
		return -1;
	}
	
	bzero((char *) &serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons((unsigned short)port);
	if(bind(listenfd, (SA *)&serveraddr, sizeof(serveraddr)) < 0)
	{
		return -1;
	}

	return udpfd;
}

int main(int argc, char **argv)
{
	int myID, ne_port, router_port, ne_fd, router_fd;
	
	myID = atoi(argv[1]);
	ne_port = atoi(argv[3]);
	router_port = atoi(argv[4]);

	ne_fd = open_udpfd(ne_port);
	router_fd = open_udpfd(router_port);
}


