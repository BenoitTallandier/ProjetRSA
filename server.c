#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <math.h>

struct argThread{
	int dialogSocket;
	int * listClient;
	int * nbConnection;
};


void * ecoute(void * arg){
	printf("		demarage thread\n");
	char message[2048]="Serveur>> Connection : OK\n";
	char messageVide[2048] = "";
	char messageFormate[2052] = "";
	struct argThread * argument = (struct argThread *)arg;
	int dialogSocket = argument->dialogSocket;
	int * listClient = argument->listClient;
	int * nbConnection = argument->nbConnection;
	int result = 1;
	send(dialogSocket,message,sizeof(message),0);
	for(int i=0;i<fmin(10,*nbConnection);i++){
		if(listClient[i]>0){
			sprintf(messageFormate,"Serveur>> %d a rejoint\n",dialogSocket);
			printf("		envoi du message arrive à %d  : '''%s'''\n",listClient[i],messageFormate);
			send(listClient[i], messageFormate, strlen(messageFormate),0);
		}
 	}
	while(result!=0){
		printf("		demarage boucle\n" );
		memset(message,'\0',2048);
		memset(messageFormate,'\0',2052);
		if( (result = recv(dialogSocket , message , 2048 , 0))>0){
			if(memcmp(&message,&messageVide,2048)!=0 ){
				printf("		message recv from %d : %s\n",dialogSocket,message );
				for(int i=0;i<fmin(10,*nbConnection);i++){
					if(dialogSocket != listClient[i] && listClient[i]>0){
						printf("			envoi de message au client %d = %d \n",i,listClient[i]);
						sprintf(messageFormate,"%d>> ",dialogSocket);
						strcat(messageFormate,message);
						send(listClient[i], messageFormate, sizeof(messageFormate),0);
					}
				}
				printf("\n");
				strcpy(message, "");
			}
		}

	}
	for(int i=0;i<fmin(10,*nbConnection);i++){
		if(dialogSocket == listClient[i]){
			printf("	client %d lived\n",dialogSocket);
			listClient[i]=-1;
		}
		else{
			sprintf(messageFormate,"Serveur>> %d is gone \n",dialogSocket);
			send(listClient[i],messageFormate,sizeof(messageFormate),0);
		}
	}
	return ;
}


int main(int argc, char const *argv[]){
	printf("start\n");

	int i;
	pthread_t * threads;
	threads = malloc(10*sizeof(pthread_t));
	struct argThread *arguments;
	arguments = malloc(10*sizeof(struct argThread));

	struct sockaddr_in serv_addr, cli_addr;
	int socketServer,clilen;
	int * listClient;
	listClient = malloc(10*sizeof(int));
	memset(&serv_addr, 0, sizeof(serv_addr));

	clilen = sizeof(cli_addr);
	/* configuration de serv_addr */
	serv_addr.sin_family = 	AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); // on peut recevoir les messages de toute ip
	serv_addr.sin_port = htons(atoi(argv[1]));


	if( (socketServer = socket(PF_INET, SOCK_STREAM,0))<0 ){
		perror("echec creation socket \n");
	}
	if(bind(socketServer,(struct sockaddr *) &serv_addr,sizeof(serv_addr) ) <0) {
		perror ("servecho: erreur bind\n");
		printf(" addr : %d\n", ntohl(serv_addr.sin_addr.s_addr));
		printf(" port : %d\n", ntohs(serv_addr.sin_port));
		exit (1);
  	}
	if(listen(socketServer, 10)<0){
		perror("echec\n");
	}

	i=0;
	while(1){
		listClient[i] = accept(socketServer, (struct sockaddr *) &cli_addr,(socklen_t *)&clilen);
		printf("	accept : %d\n",listClient[i]);
		arguments[i].dialogSocket = listClient[i];
		arguments[i].listClient = listClient;
		arguments[i].nbConnection = &i;
		pthread_create(&threads[i],NULL,ecoute,&arguments[i]);
		printf("	Connection client %d \n",i);
		i++;
	}
	printf("end\n");
	return 0;
}
