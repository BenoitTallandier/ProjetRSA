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
#include <unistd.h>
#define TAILLE_MAX 30000

//gzip, deflate


struct argThread{
	int socketServer;
	int socketClient;
};

struct argThreadServ{
	char ** pub;
	int socketClient;
};

char** LectureFile() {

	char ligne[TAILLE_MAX] = "";
	int lenStr;
	FILE * listfile = fopen("adservers.txt", "r");
	int i = 0;
	while (fgets(ligne, TAILLE_MAX, listfile) != NULL) {
		i++;
	}
	printf(" --- i = %d\n",i);
	rewind(listfile);

	char** listsite = (char**)malloc(i*TAILLE_MAX + 1);
	int g = 0;
	while (fgets(ligne, TAILLE_MAX, listfile) != NULL) {
		listsite[g] = malloc(strlen(ligne)+2);
		lenStr = strlen(ligne);
		ligne[(lenStr - 1)] = '\0';
		strcpy(listsite[g], ligne);
		g++;
	}
	return listsite;
}


void * ecouteServeur(void * arg){
	//printf("		demarage thread\n");
	struct argThread * argument = (struct argThread *)arg;
	int socketServer = argument->socketServer;
	int socketClient = argument->socketClient;
	int taille = 45808;
	char  bufferReception[taille];
	int result = 1;
	int size = -1;
	int length = -1;
	while((result = recv(socketServer,bufferReception, sizeof(bufferReception) , 0))!=0){ //&& (size<0 || length<size)){

		char * sze=strstr(bufferReception,"Content-Length");
		if(sze){
			sze = strtok(sze,"\n");
			sze = strtok(sze,": ");
			sze = strtok(NULL,": ");
			size = atoi(sze); //pour la taille de l'entête
			printf("size : (%d)\n",size );
		}
		//printf("--------------reception %d ------------\n\n",result);
		printf("	reception %d\n",result);
		bufferReception[result] = '\0';
		send(socketClient,&bufferReception,strlen(bufferReception),0 );
		/*if(strstr(bufferReception,"\r\n\r\n")!=NULL){
			//printf("----END reception ----\n");
			break;
		}*/
		length = length + result;
		memset(bufferReception,0,strlen(bufferReception));
	}

	printf("THREAD END\n");
	pthread_exit(NULL);
}


char * getHost(char * buffer){
	int x=0;
	if(strspn(buffer, "GET") >0){
		x=1;
	}
	if(strspn(buffer, "CONNECT")>0){
		x=2;
	}
	char * bufferCpy = malloc(1452*sizeof(char));
	memcpy(bufferCpy,buffer,1452);
	char * host = malloc(sizeof(char)*100);
	char *token;
	token = strtok (bufferCpy,"\n");
	while (token != NULL)
	{
		if(x<=1 && strspn(token, "Host: ") >0){
			sscanf(token,"Host: %s",host);
		}
		else if(x==2 && strspn(token, "Host: ") >0){
			sscanf(token,"Host: %s",host);
			host = strtok(host,":");
		}
		token = strtok (NULL, "\n");
	}
	//printf("HOST : (%s) \n",host);
	return host;
}

void * ecouteClient(void * arg){
	//printf("Demarage thread client\n");
	/*char pub[][20] = {
     	"adleadevent.com",
     	"amazon-adsystem.com",
     	"adnxs.com",
		"smartadserver.com",
		"dtech.de"
	};*/
	char * response = "HTTP/1.1 202 Ok\r\n\r\n";
	struct argThreadServ ar= *((struct argThreadServ *)arg);
	int listClient = ar.socketClient;
	char ** pub = ar.pub;
	struct addrinfo hints, *res;
	pthread_t * threads;
	threads = malloc(10*sizeof(pthread_t));
	struct argThread *arguments;
	arguments = malloc(10*sizeof(struct argThread));
	char * buffer=malloc(18000*sizeof(char));//"GET /index.html HTTP/1.1\r\nHost: www.benoittallandier.com\r\n\r\n";
	char * messageVide = malloc(18000*sizeof(char));
	memset(messageVide,0,strlen(messageVide));
	int i,j;

	int dialogSocket;
	i = 0;
	int nonPub =1;
	int x = recv(listClient, buffer , 18000 , 0);
	if(x>=18000){printf("erreur\n");}
	if(strlen(buffer)==0){
		pthread_exit(NULL);
	}
	//printf("------------------\n%s\n-------------------------\n",buffer);
	/*if(size<=0 || memcmp(buffer,&messageVide,1452)==0){
		printf("fin d'ecoute client\n");
		break;
	}*/
	//printf("client : %s\n",buffer);
				//recv(listClient, buffer , 1452 , 0);
	if(strspn(buffer, "GET") >0 || strspn(buffer, "CONNECT") >0){
		char * host = getHost(buffer);
		if(strlen(host)==0){
			printf("host vide\n");
			close(listClient);
			pthread_exit(NULL);
		}
		char * result;
		char token[80];

		for(j=0;j<46585;j++){
			if(strlen(pub[j])>0 && (result=strstr(host, pub[j]))!=NULL){
				sprintf(token,".%s",result);
				if(strlen(host) == strlen(pub[j]) || strstr(host,token)!=NULL){
					printf("%s bloqué par %s\n",host,pub[j]);
					nonPub = 0;
				}
			}
		}
		if(nonPub==0){
			send(listClient, response,strlen(response),0);
			//printf("pub bloqué : %s\n",host);
		}
		if(nonPub==1){
			printf("HOST : (%s) \n",host);
			memset(&hints, 0,sizeof hints);
		    hints.ai_family=AF_INET;
		    getaddrinfo(host,"80", &hints, &res);
			//client_addr.sin_addr.s_addr = hostInt;
			if( (dialogSocket = socket(PF_INET,SOCK_STREAM,0))<0){
				perror("echec de création socket \n");
			}
			//printf("socket created\n");

			if(connect(dialogSocket,res->ai_addr,res->ai_addrlen) < 0){
				perror("erreur connect \n");
			}
			arguments[i].socketClient = listClient;
			arguments[i].socketServer = dialogSocket;

			pthread_create(&threads[i],NULL,ecouteServeur,&arguments[i]);

			//printf("-------------\nenvoi de \n%s\n------------------\n",buffer);
			send(dialogSocket,buffer,strlen(buffer),0);
			pthread_join(threads[i],NULL);
			//printf("retour au thread\n\n");
			close(dialogSocket);
		}
	}
	else{
		//printf("request non HTTP \n%s\n",buffer);
		//printf("-------------\nenvoi de \n%s\n------------------\n",buffer);
		send(listClient,response,strlen(response),0);
	}
	memset(buffer,0,strlen(buffer));
	close(listClient);
	pthread_exit(NULL);
}




int main(int argc, char const *argv[]){
	printf("start\n");
	char ** pub = LectureFile();
	pthread_t * threads;
	threads = malloc(100*sizeof(pthread_t));
	struct argThreadServ * arg = malloc(100*sizeof(struct argThreadServ));
	struct sockaddr_in serv_addr, cli_addr;
	int socketServer,clilen;
	int * listClient;
	listClient = malloc(1*sizeof(int));
	memset(&serv_addr, 0, sizeof(serv_addr));
	int i;
	clilen = sizeof(cli_addr);
	/* configuration de serv_addr */
	serv_addr.sin_family = 	AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); // on peut recevoir les messages de toute ip
	serv_addr.sin_port = htons(atoi(argv[1]));
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
	i=0;
	for(;;){
		//printf("Attente d'un client\n");
		*listClient = accept(socketServer, (struct sockaddr *) &cli_addr,(socklen_t *)&clilen);
		//printf("	accept : %d\n",*listClient);
		arg[i].socketClient = *listClient;
		arg[i].pub = pub;
		//printf("demarage de %d\n",i);
		pthread_create(&threads[i],NULL,ecouteClient,&arg[i]);
		i = i+1;
		//pthread_join(threads[i],NULL);
		if(i>=100){
			printf("limite atteinte\n");
			i=0;
			//sleep(5);
		}
	}
	printf("end\n");
	return 0;
}
