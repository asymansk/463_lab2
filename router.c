#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "ne.h"
#include "router.h"

typedef struct sockaddr SA; //global, need to set type in bind call

/*int open_udpfd(int port)
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
	if(bind(udpfd, (SA *)&serveraddr, sizeof(serveraddr)) < 0)
	{
		return -1;
	}

	return udpfd;
}*/

int main(int argc, char **argv)
{
	int myID, ne_port, router_port, router_fd;
	struct hostent *hp;
	struct sockaddr_in neAddr, routerAddr;

	myID = atoi(argv[1]);

	//connect to network emmulator 
	if((hp = gethostbyname(argv[2])) == NULL)
	{
		return -1;
	}

	bzero((char *) &neAddr, sizeof(neAddr));
	neAddr.sin_family = AF_INET;
	bcopy((char *)hp->h_addr,
		  (char *)&neAddr.sin_addr.s_addr, hp->h_length);
	ne_port = atoi(argv[3]);
	neAddr.sin_port = htons(ne_port);

	//create router port 
	router_port = atoi(argv[4]);
	if((router_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		return -1; 
	}

	bzero((char *) &routerAddr, sizeof(routerAddr));
	routerAddr.sin_family = AF_INET;
	routerAddr.sin_addr.s_addr = INADDR_ANY;
	routerAddr.sin_port = htons(router_port);

	if(bind(router_fd, (SA *)&routerAddr, sizeof(routerAddr)) < 0)
	{
		return -1;
	}

	//sends initial request 
	struct pkt_INIT_REQUEST initReq;
	initReq.router_id = htonl(myID);

	sendto(router_fd, (struct pkt_INIT_RESPONSE*)&initReq, sizeof(initReq), 0, (SA *)&neAddr, sizeof(neAddr));

	//receives initial response 
	struct pkt_INIT_RESPONSE initRes;	
	recvfrom(router_fd, (struct pkt_INIT_RESPONSE*)&initRes, sizeof(initRes), 0, (SA *)&neAddr, (socklen_t*)sizeof(neAddr));
	ntoh_pkt_INIT_RESPONSE(&initRes);

	//initalized router table
	InitRoutingTbl(&initRes, myID);

	char* file = (char*)malloc(15);
	file = strdup("ourRouter");
	strcat(file, argv[1]);
	strcat(file, ".log");
	FILE* printTo = fopen(file, "w");
	PrintRoutes(printTo, myID);
	fclose(printTo);

	return 0;
}
