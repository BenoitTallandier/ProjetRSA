#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>


int main(int argc, char const *argv[]){

	printf("start client \n");

	char messageEnvoye[2048];

	struct sockaddr_in client_addr;
	int dialogSocket;

	client_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	client_addr.sin_port = htons(atoi(argv[1])); /* on utilise htons pour le port */
	client_addr.sin_family = AF_INET;

	if( (dialogSocket = socket(PF_INET,SOCK_STREAM,0))<0){
		perror("echec de création socket \n");
	}


	if(connect(dialogSocket,(struct sockaddr *) &client_addr, sizeof(client_addr)) < 0){
		perror("erreur connect \n");
	}
	char message[] = "GET http://benoittallandier.com/ HTTP/1.1\nHost: benoittallandier.com\nUser-Agent: Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:52.0) Gecko/20100101 Firefox/52.0\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\nAccept-Language: en-US,en;q=0.5\nAccept-Encoding: gzip, deflate\nConnection: keep-alive\nUpgrade-Insecure-Requests: 1\n";
	printf("len : %d\n",strlen(message));	
	send(dialogSocket,message,strlen(message),0);
	printf("end client \n");
	return 0;
}
