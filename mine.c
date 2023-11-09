/*
-----------------------------------------------------------------------------
								Libraries
-----------------------------------------------------------------------------
*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <pthread.h>
#include <string.h>

/*
-----------------------------------------------------------------------------
								Global Variable
-----------------------------------------------------------------------------
*/
#define MAXPENDING 100
#define RCVBUFSIZE 1024
#define MAXCLIENT 10


// Array for BoradCast
int clntsSocks[MAXCLIENT];
int recvMsgSize;
/*
-----------------------------------------------------------------------------
								Functions
-----------------------------------------------------------------------------
*/

///////////////////////////
//		ERROR 			//
/////////////////////////
void dieWithError(char *errorMessage)
{
	perror(errorMessage);
	exit(1);
}

/*
-----------------------------------------------------------------------------
							Server Functions
-----------------------------------------------------------------------------
*/		
///////////////////////////
// 		1)BroadCast		//
/////////////////////////
void broadcast(const char *message, int clntSock ) {

		int index = 0 ;
    for(int i=0; i<MAXCLIENT; i++){
        if(clntsSocks[i] == clntSock){
            index = i + 1;
            break;
        }
    }
    char sender[20] = "";
    sprintf(sender, "Client(%d): ", index);	

 

    for(int i=0; i<MAXCLIENT; i++){
			if(clntsSocks[i] == 0)
				break;

			else if(clntsSocks[i] != clntSock){
 if (send(clntsSocks[i], sender, strlen(sender), 0) != strlen(sender))
                dieWithError("send() failed");
				if (send(clntsSocks[i], message, recvMsgSize, 0) != recvMsgSize  )
					dieWithError("send() failed");
			}
		}
}

///////////////////////////
// 		2)Thread 		//
/////////////////////////
void* serverThread(void* arg){

	int clntSock=*((int *)arg);

	while(1){
		char message[RCVBUFSIZE];
		
		memset(message, '\0', RCVBUFSIZE);

		if ((recvMsgSize = recv(clntSock, message, RCVBUFSIZE, 0)) < 0)
			dieWithError("recv() failed");

		//Broad Cast::
		broadcast(message, clntSock);
	}
}	

///////////////////////////
// 		3)server 		//
/////////////////////////
void serverProcess()
{
	int servSock;
	struct sockaddr_in servAddr, clntAddr;
	unsigned short port;
	unsigned int clntLen;
	char servPort[5];


	printf("Enter a port number to continue (or enter 0 to select random port)\n");
	printf("Remember, The port should be bigger than 1024 and not more 65000 : ");
	scanf("%s",servPort);
	if(atoi(servPort) == 0){
		//port=(rand() *(20000 -1)) +1 + 1024;
		port=rand()+20000;
	}else 
		port=atoi(servPort);

	if ((servSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
		dieWithError("socket() failed");

	memset(&servAddr, 0, sizeof(servAddr));

	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAddr.sin_port = htons(port);

	if (bind(servSock, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0)
		dieWithError("bind() failed");

	if (listen(servSock, MAXPENDING) < 0)
		dieWithError("listen() failed");

	printf("server: waiting for Members on port %d...\n",port);

	// Define Threads:
	pthread_t clientsThreads[MAXCLIENT];
	
	for (int i=0; i<MAXCLIENT; i++)
	{
		clntLen = sizeof(clntAddr);

		if ((clntsSocks[i] = accept(servSock, (struct sockaddr *)&clntAddr, &clntLen)) < 0)
			dieWithError("accept() failed");

		pthread_create(&clientsThreads[i],NULL,serverThread,&clntsSocks[i]);
	}
}

/*
-----------------------------------------------------------------------------
							Client Functions
-----------------------------------------------------------------------------
*/
///////////////////////////
// 		1)sending		//
/////////////////////////
void* SendClient(void* arg){

	int servSock=*((int *)arg);
	char message[RCVBUFSIZE];
	
	scanf("%s",message);
	
	while (message != "exit")
	{
		int len=strlen(message);
		if (send(servSock,message, len, 0) != len)
 			dieWithError("send() failed");
		scanf("%s",message);
	}
}


///////////////////////////////
// 		2)Receiving			//
/////////////////////////////
void* RecvClient(void* arg){

	int servSock=*((int *)arg);
	char message[RCVBUFSIZE];
	int recvMsgSize;

	while(1){
		memset(message, '\0', RCVBUFSIZE);
		if ((recvMsgSize = recv(servSock,message, RCVBUFSIZE, 0)) < 0)
			dieWithError("recv() failed");

		puts(message);
	}
}

///////////////////////////
//		3)client 		//
/////////////////////////
void clientProcess()
{
	char servPort[5];
	unsigned int clntLen;
	printf("Enter port number of chatroom: ");
	scanf("%s",servPort);

	int status;
	struct addrinfo hint;
	struct addrinfo *servinfo,*i;

	memset(&hint, 0, sizeof hint);
	memset(&servinfo, 0, sizeof servinfo);

	hint.ai_family =AF_INET; 
	hint.ai_socktype = SOCK_STREAM; 
	hint.ai_flags=AI_PASSIVE;

	if ((status = getaddrinfo(NULL, servPort, &hint, &servinfo)) != 0)
		dieWithError("getaddrinfo() failed");
	

	for(i=servinfo; i!=NULL; i= i->ai_next){

		int sfd = socket(i->ai_family, i->ai_socktype, i->ai_protocol);
		if(sfd==-1){
			dieWithError("socket error()");
		}

		int con = connect(sfd, i->ai_addr, i->ai_addrlen);

		if(con<0){
			perror("connect error()");
			continue;
		}

		printf("connected \n");

		pthread_t sender,reciver;

		// is it will be in LOOP??


		pthread_create(&sender,NULL,SendClient,&sfd);
		pthread_create(&reciver,NULL,RecvClient,&sfd);
		
		while(1){
			}


	}
}

/*
-----------------------------------------------------------------------------
								MAIN
-----------------------------------------------------------------------------
*/
int main(int argc, char const *argv[])
{
	char choice;
	printf("Welcom! \nDo you want to create a new chatroom or join an existing one?\n");
	printf("to create room type c\nto join room press j\n");
	again:scanf("%c", &choice);
	if (choice == 'c' || choice == 'C')
		serverProcess();
	else if (choice == 'j'|| choice == 'J')
		clientProcess();
	else{
		printf("You pressed unknown KEY !!\n Try again : ");
		goto again;
	}

}
