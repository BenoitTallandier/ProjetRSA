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
#include <netdb.h>


struct argThread{
	int socketServer;
	int socketClient;
};

void * ecouteClient(void * arg){

}

void * ecouteServeur(void * arg){
	printf("		demarage thread\n");
	struct argThread * argument = (struct argThread *)arg;
	int socketServer = argument->socketServer;
	int socketClient = argument->socketClient;
	char  bufferReception[512];
	int result = 1;
	while((result = recv(socketServer,bufferReception, sizeof(bufferReception) , 0))>=0){
		if(result==0){
			printf("----END reception ----\n");
			break;
		}
		//printf("--------------reception %d ------------\n %s\n\n",result,bufferReception);
		send(socketClient,&bufferReception,strlen(bufferReception),0 );
		memset(bufferReception,0,strlen(bufferReception));
	}
	printf("THREAD END\n");
	return;
}


char * getHost(char * buffer){
	int x=0;
	if(strspn(buffer, "GET") >0){
		x=1;
		printf("request GET\n");
	}
	if(strspn(buffer, "CONNECT")>0){
		x=2;
		printf("request CONNECT\n");
	}
	char * bufferCpy = malloc(8384*sizeof(char));
	memcpy(bufferCpy,buffer,8384);
	char * host = malloc(sizeof(char)*100);
	char *token;
	int port;
	token = strtok (bufferCpy,"\n");
	while (token != NULL)
	{
		if(x==1 && strspn(token, "Host: ") >0){
			sscanf(token,"Host: %s",host);
		}
		else if(x==2 && strspn(token, "Host: ") >0){
			sscanf(token,"Host: %s:",host);
			printf(" host connect : %s\n",host);
		}
		token = strtok (NULL, "\n");
	}
	printf("HOST : (%s) \n",host);
	return host;
}

int main(int argc, char const *argv[]){
	printf("start\n");

	pthread_t * threads;
	threads = malloc(10*sizeof(pthread_t));
	struct argThread *arguments;
	arguments = malloc(10*sizeof(struct argThread));


	struct addrinfo hints, *res;
	struct sockaddr_in serv_addr, cli_addr;
	int socketServer,clilen;
	int * listClient;
	listClient = malloc(1*sizeof(int));
	memset(&serv_addr, 0, sizeof(serv_addr));
	char * buffer=malloc(8384*sizeof(char));
	char * bufferCpy = malloc(8384*sizeof(char));//"GET /index.html HTTP/1.1\r\nHost: www.benoittallandier.com\r\n\r\n";
	char * messageVide = malloc(8384*sizeof(char));
	memset(messageVide,0,strlen(messageVide));
	int i;

	clilen = sizeof(cli_addr);
	/* configuration de serv_addr */
	serv_addr.sin_family = 	AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); // on peut recevoir les messages de toute ip
	serv_addr.sin_port = htons(atoi(argv[1]));
	printf("MAJ\n");
	if( (socketServer = socket(PF_INET, SOCK_STREAM,0))<0 ){
		perror("echec creation socket \n");
	}
	if(bind(socketServer,(struct sockaddr *) &serv_addr,sizeof(serv_addr) ) <0){
		perror ("servecho: erreur bind\n");
		printf(" addr : %d\n", ntohl(serv_addr.sin_addr.s_addr));
		printf(" port : %d\n", ntohs(serv_addr.sin_port));
		exit (1);
  	}
	printf(" addr : %d\n", ntohl(serv_addr.sin_addr.s_addr));
	printf(" port : %d\n", ntohs(serv_addr.sin_port));

	if(listen(socketServer, 1)<0){
		perror("echec\n");
	}
	while(1){
		printf("Attente d'un client\n");
		listClient = accept(socketServer, (struct sockaddr *) &cli_addr,(socklen_t *)&clilen);
		printf("	accept : %d\n",listClient);
		printf("	Connection client \n");
		int dialogSocket;
		i = 0;
		int size;
			printf("------------debut recv-------------\n");
			size = recv(listClient, buffer , 8384 , 0);
			printf("-----------fin recv %d-----------\n",size );
			/*if(size<=0 || memcmp(buffer,&messageVide,8384)==0){
				printf("fin d'ecoute client\n");
				break;
			}*/
			printf("client : %s\n",buffer);
							//recv(listClient, buffer , 8384 , 0);
				if(strspn(buffer, "GET") >0 || strspn(buffer, "CONNECT") >0){
					char * host = getHost(buffer);
					memset(&hints, 0,sizeof hints);
				    hints.ai_family=AF_INET;
				    getaddrinfo(host,"80", &hints, &res);
					struct sockaddr_in client_addr;
					char message[10];
					//client_addr.sin_addr.s_addr = hostInt;
					if( (dialogSocket = socket(PF_INET,SOCK_STREAM,0))<0){
						perror("echec de création socket \n");
					}
					printf("socket created\n");

					if(connect(dialogSocket,res->ai_addr,res->ai_addrlen) < 0){
						perror("erreur connect \n");
					}
					arguments[i].socketClient = listClient;
					arguments[i].socketServer = dialogSocket;
					printf("server connected \n");

					pthread_create(&threads[i],NULL,ecouteServeur,&arguments[i]);

					printf("-------------\nenvoi de \n%s\n------------------\n",buffer);
					send(dialogSocket,buffer,strlen(buffer),0);

				}
				else{
					printf("request non HTTP \n%s\n",buffer);
					printf("-------------\nenvoi de \n%s\n------------------\n",buffer);
					send(dialogSocket,buffer,strlen(buffer),0);
				}
				memset(buffer,0,strlen(buffer));

			//sleep(5);
	}
	printf("end\n");
	return 0;
}
