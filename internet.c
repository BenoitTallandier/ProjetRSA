#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main(void) {
    //Stream sockets and rcv()

    struct addrinfo hints, *res;
    int sockfd;

    char buf[2056];
    int byte_count;

    //get host info, make socket and connect it
    memset(&hints, 0,sizeof hints);
    hints.ai_family=AF_INET;
    getaddrinfo("www.benoittallandier.com","80", &hints, &res);
    sockfd = socket(PF_INET,SOCK_STREAM,0);
    printf("Connecting...\n");
    connect(sockfd,(struct sockaddr *) &client_addr, sizeof(client_addr)
    //connect(sockfd,res->ai_addr,res->ai_addrlen);
    printf("Connected!\n");
    char *header = "GET /index.html HTTP/1.1\r\nHost: www.benoittallandier.com\r\n\r\n";
    send(sockfd,header,strlen(header),0);
    printf("GET Sent...\n");
    //all right ! now that we're connected, we can receive some data!
    byte_count = recv(sockfd,buf,sizeof(buf),0);
    printf("recv()'d %d bytes of data in buf\n",byte_count);
    printf("%.*s",byte_count,buf); // <-- give printf() the actual data size
    return 0;
}
