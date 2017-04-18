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


int main(int argc, char const *argv[]){
	printf("start\n");

	int i;
	pthread_t * threads;
	threads = malloc(1*sizeof(pthread_t));

	struct addrinfo hints, *res;
	struct sockaddr_in serv_addr, cli_addr;
	int socketServer,clilen;
	int * listClient;
	listClient = malloc(1*sizeof(int));
	memset(&serv_addr, 0, sizeof(serv_addr));
	char * buffer=malloc(8384*sizeof(char));
	char * bufferCpy = malloc(8384*sizeof(char));//"GET /index.html HTTP/1.1\r\nHost: www.benoittallandier.com\r\n\r\n";
	char  bufferReception[1048576] = "a";

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

	listClient = accept(socketServer, (struct sockaddr *) &cli_addr,(socklen_t *)&clilen);
	printf("	accept : %d\n",listClient);
	printf("	Connection client \n");

	//while(1){
		recv(listClient, buffer , 8384 , 0);
		while(strspn(buffer, "GET") <0){
			recv(listClient, buffer , 8384 , 0);
		}
		memcpy(bufferCpy,buffer,8384);
		if(strspn(buffer, "GET") >0){
			printf("request HTTP\n");
			char * host=malloc(sizeof(char)*100);
			char *token;
			token = strtok (buffer,"\n");
			while (token != NULL)
		    {
				//printf("%s\n",token);
				if(strspn(token, "Host: ") >0){
					sscanf(token,"Host: %s",host);
				}
			   	token = strtok (NULL, "\n");
		    }
			printf("HOST : (%s) \n",host);
			memset(&hints, 0,sizeof hints);
		    hints.ai_family=AF_INET;
		    getaddrinfo(host,"80", &hints, &res);
			struct sockaddr_in client_addr;
			int dialogSocket;
			char message[10];
			//client_addr.sin_addr.s_addr = hostInt;
			if( (dialogSocket = socket(PF_INET,SOCK_STREAM,0))<0){
				perror("echec de création socket \n");
			}
			printf("socket created\n");

			if(connect(dialogSocket,res->ai_addr,res->ai_addrlen) < 0){
				perror("erreur connect \n");
			}
			printf("server connected \n");
			printf("-------------\nenvoi de \n%s\n------------------\n",bufferCpy);
			send(dialogSocket,bufferCpy,strlen(bufferCpy),0);
			recv(dialogSocket,bufferReception, 1048576 , 0);
			printf("reception : %s\n",bufferReception);
		}
		else{
			printf("request non HTTP \n%s\n",buffer);
			printf("%s\n",buffer);
		}
		//sleep(5);
	//}
	printf("end\n");
	return 0;
}
