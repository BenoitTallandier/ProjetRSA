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
#define TAILLE_MAX 300

//gzip, deflate


struct argThread{
	int socketServer;
	int socketClient;
	int numero;
};

struct argThreadServ{
	char ** pub;
	int socketClient;
	int numero;
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
	printf("		demarage thread\n");
	struct argThread * argument = (struct argThread *)arg;
	int socketServer = argument->socketServer;
	int socketClient = argument->socketClient;
	int num = argument->numero;
	int tailleBuff = 100080;
	char  bufferReception[tailleBuff];
	char  bufferReceptionCpy[tailleBuff];
	int result = 1;
	int size = -1;
	int length = -1;
	struct timeval tv;
	tv.tv_sec = 2;  /* 10 Secs Timeout */
	tv.tv_usec = 0;  // Not init'ing this can cause strange errors
	setsockopt(socketServer, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv,sizeof(struct timeval));

	while((result = recv(socketServer,bufferReception, sizeof(bufferReception)-1 , 0))>0){//} && (size<0 || length<size)){
		if(result<tailleBuff){bufferReception[result]='\0';}
		/*memcpy(bufferReceptionCpy,bufferReception,strlen(bufferReception));
		char * sze=strstr(bufferReceptionCpy,"Content-Length");
		if(strlen(sze)>0){
			sze = strtok(sze,"\n");
			sze = strtok(sze,": ");
			sze = strtok(NULL,": ");
			size = atoi(sze); //pour la taille de l'entête
			printf("size : (%d)\n",size );
		}*/
		//printf("--------------reception %d ------------\n%s \n\n",result,bufferReception);
		if(result==0){
			break;
		}
		int error = 0;
		socklen_t len = sizeof (error);
		int retval = getsockopt (socketClient, SOL_SOCKET, SO_ERROR, &error, &len);
		if (retval != 0) {
	    	fprintf(stderr, "error getting socket error code: %s\n", strerror(retval));
		    pthread_exit(NULL);
		}
		if (error != 0) {
		    fprintf(stderr, "socket error: %s\n", strerror(error));
			pthread_exit(NULL);
		}
		send(socketClient,&bufferReception,strlen(bufferReception),0 );
		/*if(strstr(bufferReception,"\r\n\r\n")!=NULL){
			//printf("----END reception ----\n");
			break;
		}*/
		length = length + result;
		printf("	reception %d , %d\n",result,num);
		memset(bufferReception,0,strlen(bufferReception));
	}

	//printf("THREAD END\n");
	pthread_exit(NULL);
}


char * getHost(char * buffer){
	if(strlen(buffer)!=0){
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
		printf("HOST : (%s) \n",host);
		free(bufferCpy);
		return host;
	}
}

void * ecouteClient(void * arg){
	printf("Demarage thread client\n");
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
	int num = ar.numero;
	struct addrinfo hints, *res;
	pthread_t * threads;
	threads = malloc(sizeof(pthread_t));
	printf("test1 %d\n",num);
	struct argThread *arguments;
	arguments = malloc(sizeof(struct argThread));
	char * buffer=malloc(18000*sizeof(char));//"GET /index.html HTTP/1.1\r\nHost: www.benoittallandier.com\r\n\r\n";
	//char * messageVide = malloc(18000*sizeof(char));
	//memset(messageVide,0,strlen(messageVide));
	int j;

	int dialogSocket;
	int nonPub =1;
	int x = recv(listClient, buffer , 18000 , 0);
	if(x>=18000){printf("erreur\n");}
	if(strlen(buffer)==0){
		printf("buffer vide\n");
		memset(buffer,0,strlen(buffer));
		free(threads);
		free(arguments);
		free(buffer);
		close(listClient);
		pthread_exit(NULL);
	}
	printf("test2 %d\n",num);

	//printf("------------------\n%s\n-------------------------\n",buffer);
	/*if(size<=0 || memcmp(buffer,&messageVide,1452)==0){
		printf("fin d'ecoute client\n");
		break;
	}*/
	//printf("client : %s\n",buffer);
				//recv(listClient, buffer , 1452 , 0);
	if(strspn(buffer, "GET") >0){
		char * host = getHost(buffer);
		if(strlen(host)==0){
			printf("host vide \n");
			free(arguments);
			free(buffer);
			close(listClient);
			pthread_exit(NULL);
		}
		printf("test3 %d\n",num);
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
		printf("test4 %d\n",num);

		if(nonPub==1){
			printf("HOST : (%s) %d\n",host,num);
			memset(&hints, 0,sizeof hints);
		    hints.ai_family=AF_INET;
		    getaddrinfo(host,"80", &hints, &res);
			//client_addr.sin_addr.s_addr = hostInt;
			if( (dialogSocket = socket(PF_INET,SOCK_STREAM,0))<0){
				printf("erreur creation socket\n");
				perror("echec de création socket \n");
			}
			printf("socket created %d \n",num);

			if(connect(dialogSocket,res->ai_addr,res->ai_addrlen) < 0){
				printf("erreur connexion\n");
				perror("erreur connect \n");
			}
			arguments[0].socketClient = listClient;
			arguments[0].socketServer = dialogSocket;
			arguments[0].numero = num;

			pthread_create(&threads[0],NULL,ecouteServeur,&arguments[0]);
			printf("test5 %d\n",num);

			//printf("-------------\nenvoi de \n%s\n------------------\n",buffer);
			send(dialogSocket,buffer,strlen(buffer),0);
			pthread_join(threads[0],NULL);

			//printf("retour au thread\n\n");
		}
	}
	else{
		//printf("request non HTTP \n%s\n",buffer);
		//printf("-------------\nenvoi de \n%s\n------------------\n",buffer);
		send(listClient,response,strlen(response),0);
	}
	printf("test6 %d\n",num);
	pthread_join(threads[0],NULL);
	memset(buffer,0,strlen(buffer));
	free(threads);
	free(arguments);
	free(buffer);
	close(listClient);
	pthread_exit(NULL);
}




int main(int argc, char const *argv[]){
	printf("start \n");

	int nbThreads = SOMAXCONN/2-10;
	char ** pub = LectureFile();
	pthread_t * threads;
	threads = malloc(nbThreads*sizeof(pthread_t));
	struct argThreadServ * arg = malloc(nbThreads*sizeof(struct argThreadServ));
	struct sockaddr_in serv_addr, cli_addr;
	int socketServer,clilen;
	int * listClient;
	listClient = malloc(1*sizeof(int));
	memset(&serv_addr, 0, sizeof(serv_addr));
	int i;
	int flagPremiereBoucle=1;
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
		if(flagPremiereBoucle==0){
			printf("on attend la fin du thread %d ...\n",i);
			pthread_join(threads[i],NULL);
		}
		printf("	accept : %d\n",*listClient);
		arg[i].socketClient = *listClient;
		arg[i].pub = pub;
		arg[i].numero = i;
		printf("demarage de %d\n",i);
		pthread_create(&threads[i],NULL,ecouteClient,&arg[i]);
		i = i+1;
		//pthread_join(threads[i],NULL);
		if(i>=nbThreads){
			printf("l imite atteinte\n");
			i=0;
			flagPremiereBoucle = 0;
			//sleep(5);
		}
	}
	printf("end\n");
	return 0;
}
