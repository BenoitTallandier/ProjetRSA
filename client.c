#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>


int main(int argc, char const *argv[]){

	void * ecoute(void * arg){
		int dialogSocket = (int)arg;
		char messageReception[2052] = "";
		while(1){
			recv(dialogSocket,messageReception,2052,0);
			//if(memcmp(&messageReception,&messageVide,2052)!=0 ){
				printf("%s",messageReception);
			//}
			strcpy(messageReception,"");
		}
	}

	printf("start client \n");

	char messageEnvoye[2048];
	pthread_t * thread;
	thread = malloc(sizeof(pthread_t));

	struct sockaddr_in client_addr;
	int dialogSocket;
	char message[10];

	client_addr.sin_addr.s_addr = inet_addr("137.74.194.232");
//	client_addr.sin_addr.s_addr = inet_addr("82.216.253.246");
	//client_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	client_addr.sin_port = htons(atoi(argv[1])); /* on utilise htons pour le port */
	client_addr.sin_family = AF_INET;

	if( (dialogSocket = socket(PF_INET,SOCK_STREAM,0))<0){
		perror("echec de création socket \n");
	}


	if(connect(dialogSocket,(struct sockaddr *) &client_addr, sizeof(client_addr)) < 0){
		perror("erreur connect \n");
	}
	pthread_create(&thread,NULL,ecoute,(void*)dialogSocket);
	while(1){
		fgets(messageEnvoye, 2048, stdin);
		//scanf("%s", messageEnvoye);
		send(dialogSocket,messageEnvoye,strlen(messageEnvoye),0);
	}
	printf("end client \n");
	return 0;
}
