#include <sys/types.h>      
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <ctype.h>
#include "TcpServer.h"
#include <fcntl.h>
#include "util.h"

extern void runCallBack(const char* buf);
extern bool isTimeout();
extern void onTimeout(const char* msg);
extern void startTimer();

void TcpServer::start(IO_TYPE ioType) {
	    struct sockaddr_in serveraddr, clientaddr;
        unsigned int sockfd, addrlen, confd, i;
        ssize_t len;
        char ipstr[128];
        fd_set fds;
        struct timeval timeout; 

        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        bzero(&serveraddr, sizeof(serveraddr));
        serveraddr.sin_family = AF_INET;
        serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
        serveraddr.sin_port = htons(SERVER_PORT);
        bind(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
        listen(sockfd, 128);

        addrlen = sizeof(clientaddr);
        confd = accept(sockfd, (struct sockaddr *)&clientaddr, &addrlen);
        inet_ntop(AF_INET, &clientaddr.sin_addr.s_addr, ipstr, sizeof(ipstr));
        printf("client ip %s\tport %d\n",
            inet_ntop(AF_INET, &clientaddr.sin_addr.s_addr, ipstr, sizeof(ipstr)),
            ntohs(clientaddr.sin_port));
            if( ioType == IO_NON_BLOCKING
                || ioType == IO_MUTIPLEXING ) {
                int flags = fcntl(confd, F_SETFL, flags|O_NONBLOCK);
            } 
        //start timer after accept the connection
        startTimer();
        int loopIndex = 0;
        while (1) {
            char buf[MAXLINE] = {0};
            loopIndex++;
            printf("++++Start loopIndex: %d\n", loopIndex);
            
            if(ioType == IO_MUTIPLEXING) {
                FD_ZERO(&fds);
		        FD_SET(confd, &fds);
                timeout.tv_sec  = 8;
                timeout.tv_usec = 0;
                int ready = select(confd+1, &fds, NULL, NULL, &timeout);
                printf("select returns: %d\n", ready);
                if(isTimeout()) {
                    onTimeout("Server timeout.");
                }
                if( FD_ISSET(confd, &fds))
		        {
			        len = recv(confd, buf, sizeof(buf), 0);
                    //printf("Call callback function with message: %s\n", buf);
                    runCallBack(buf);
		        } else {
                    printf("Socket is not ready.\n");
                }
            } else {
                len = recv(confd, buf, sizeof(buf), 0);
                printf("recv len=%ld\n", len);  
                if(len > 0) {
                    runCallBack(buf);
                } else {
                    printf("Not ready this time.\n");             
                }
                sleep(1);
            }

            buf[len < MAXLINE ? len : MAXLINE - 1] = 0;
            printf("+++End loopIndex: %d\n\n\n", loopIndex);

        }
    close(confd);
    close(sockfd);

    return;
}
