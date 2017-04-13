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

	struct sockaddr_in serv_addr, cli_addr;
	int socketServer,clilen;
	int * listClient;
	listClient = malloc(1*sizeof(int));
	memset(&serv_addr, 0, sizeof(serv_addr));
	char buffer[8384]="\n";

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
		if(strspn(buffer, "GET") >0){
			printf("request HTTP\n");
			char * host=malloc(sizeof(char)*100);
			char *token;
			token = strtok (buffer,"\n");
			while (token != NULL)
		    {
				printf("%s\n",token);
				if(strspn(token, "Host: ") >0){
					sscanf(token,"Host: %s",host);
				}
			   	token = strtok (NULL, "\n");
		    }
			char * hostInt;
			printf("HOST : (%s) \n",host);
			printf("%s\n",buffer);
			struct hostent *hp = gethostbyname(host);
			if (hp == NULL) {
			   fprintf(stderr,"gethostbyname() failed\n");
			   exit(1);
			} else {
			   printf("%s = ", hp->h_name);
			   unsigned int i=0;
			   if( hp -> h_addr_list[0] != NULL) {
			   		hostInt = inet_ntoa( *( struct in_addr*)( hp -> h_addr_list[0]));
				 printf( "%s ",hostInt);
			   }
			  }
			struct sockaddr_in client_addr;
			int dialogSocket;
			char message[10];
			client_addr.sin_addr.s_addr = inet_addr(hostInt);
			client_addr.sin_port = htons(80); // on utilise htons pour le port
			client_addr.sin_family = AF_INET;

			if( (dialogSocket = socket(PF_INET,SOCK_STREAM,0))<0){
				perror("echec de création socket \n");
			}


			if(connect(dialogSocket,(struct sockaddr *) &client_addr, sizeof(client_addr)) < 0){
				perror("erreur connect \n");
			}
			printf("server connected \n");
			send(dialogSocket,buffer,strlen(buffer),0);
			recv(dialogSocket,buffer , 8384 , 0);
			printf("%s\n",buffer);
		}
		else{
			printf("request non HTTP \n%s\n",buffer);
			printf("%c%c%c",buffer[0],buffer[1],buffer[2]);
		}
		//sleep(5);
	//}
	printf("end\n");
	return 0;
}
