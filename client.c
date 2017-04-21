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
	char  bufferReception[512*sizeof(char)] = "";
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
	char * message = "GET /index.html HTTP/1.1\r\nHost: benoittallandier.com\r\n\r\n";
	printf("len : %d\n",strlen(message));
	send(dialogSocket,message,strlen(message),0);
	int result;
	while((result = recv(dialogSocket,bufferReception, sizeof(bufferReception) , 0))>=0){
		if(result==0){
			printf("#### END reception ####\n");
			break;
		}
		printf("############reception %d ##########\n %s\n\n",result,bufferReception);
		memset(bufferReception,0,strlen(bufferReception));
	}
	printf("end client \n");
	return 0;
}
