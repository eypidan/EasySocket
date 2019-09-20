/*
	statusCode:
	666 -> sucess code
	200 -> get time
	201 -> get name
	203 -> get client list
	403 -> bad request

	message queue key => 8888
*/
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/msg.h>
#include <unistd.h>
#include <netdb.h> 

//10.79.25.117 
#define SERVER_PORT	5750 //侦听端口
#define QUEUEKEY 8888
#define BUFFERSIZE 2000

extern int errno;
bool connectStatus = false;
bool getMessage = false;
pthread_t threadid;

struct dataPacket{
	char content[BUFFERSIZE];
	int statusCode;
};

struct msg_st {
	long int msg_type;
	char content[BUFFERSIZE];
	int statusCode;
};

void *recvInfoThread(void *sockid);
void sendTimeReq(void *sockid);
void sendNameReq(void *sockid);
void sendClientListReq(void *sockid);
int main() {	
	
	
	int sockfd,funcChoose = 0;
	int msgidRecv = -1;
	struct sockaddr_in serverAddr;
	struct hostent *hptr;  
	struct msg_st dataRecv;
	char serverIP[20] = "10.79.25.117 ";
    char message[100] , timeMessage[100] , nameMessage[100];
	//create message queue
	msgidRecv = msgget((key_t)QUEUEKEY, 0666 | IPC_CREAT);
	if(msgidRecv == -1){
		printf("msgget failed with error: %d\n", errno);
		exit(1);
	}
	//clean the former message queue
	// msgctl(msgidRecv, IPC_RMID, NULL);

	while(1){
		getMessage = false;
		printf("\nHello,please input 1-7 to use function\n");
		if(connectStatus)
			printf("2.close connect.\n3.get server time.\n4.get server name.\n5.get client list.\n6.send info to another client.\ninput:");
		else
			printf("1.connect server.\n9.quit.\n");
		
		std::cin >> funcChoose;
		//funcChoose = 1;
		switch(funcChoose) {
			case 1:
				if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
					printf("socket() failed! code:%d\n", errno);
					return -1;
				}
				//printf("input server ip:");
				//set server info
				if((hptr = gethostbyname(serverIP)) == NULL){
					printf("Geting error in gethostbyname func!\n");
					return 0;
				}
				serverAddr.sin_family = AF_INET;
				serverAddr.sin_port = htons(SERVER_PORT);
				//serverAddr.sin_addr.s_addr = inet_addr(argv[1]);
				memcpy(&serverAddr.sin_addr, hptr->h_addr, hptr->h_length);
				bzero(&(serverAddr.sin_zero), 8);
				if(connect(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
					printf("connect() failed! code:%d\n", errno);
					close(sockfd);
					return -1;
				}
				connectStatus = true;
				printf("connected sucessfully~\n");
				
				//create a to child thread recv
				if(pthread_create( &threadid ,NULL ,recvInfoThread ,(void *)&sockfd)) {
					printf("pthread_create failed! code:%d\n", errno);
					return -1;
				}
				break;

			case 2:
				pthread_cancel(threadid);
				close(sockfd);
				connectStatus = false;
				printf("connected stop\n");
				break;

			case 3:
				sendTimeReq((void *)&sockfd);
				break;

			case 4:
				sendNameReq((void *)&sockfd);
				break;

			case 5:
				sendClientListReq((void *)&sockfd);
				break;
			case 9:
				exit(0);
				break;
			default:
				printf("Bad input!\n");
				break;
		}

		//start to recv info from thread
		if(connectStatus == true){
			if(msgrcv(msgidRecv, (void*)&dataRecv, sizeof(msg_st)-sizeof(long int), 1, 0) == -1) {
				printf("msgrcv failed with errno: %d\n", errno);
				exit(1);
			}
			printf("In MainThread:\n%s %d\n" , dataRecv.content , dataRecv.statusCode);
		}
		
		printf("input 'c' to Continue");
		char q = 'a';
		while(q !='c')
			scanf("%c",&q);
		
	}

	return 0;
}

void *recvInfoThread(void *sockid){
	int *socketid_ptr = (int *)sockid;
	int sofd = *socketid_ptr;
	int msgidSend = -1;
	void *ptr;
	int ret = 0;
	dataPacket dataGetFromServer;
	msg_st dataSendToMainThread;
	msgidSend = msgget((key_t)QUEUEKEY, 0666 | IPC_CREAT);
	
	if(msgidSend == -1){
		printf("msgget failed with error: %d\n", errno);
		exit(1);
	}
	while(connectStatus){
		//int running = 1;
		//接收客户端的数据：		
		int nLeft = sizeof(dataPacket);
		ptr = &dataGetFromServer;
		while(nLeft >0)	{
			//接收数据：
			ret = recv(sofd, ptr, nLeft, 0);
			if(ret <= 0) {
				printf("recv() failed!\n");
				close(sofd);
				exit(-1);
			}
			nLeft -= ret;
			ptr = (char *)ptr + ret;
		}

		strcpy(dataSendToMainThread.content,dataGetFromServer.content);
		dataSendToMainThread.statusCode = dataGetFromServer.statusCode;
		dataSendToMainThread.msg_type = 1;
		//send message to main thread
		if((msgsnd(msgidSend,(void*)&dataSendToMainThread,sizeof(msg_st)-sizeof(long int),0)) == -1){
			printf("msgsnd failed!\n");
			exit(1);
		}
	}
}

void sendTimeReq(void *sockid){
	int sofd = *(int *)sockid;
	dataPacket sendToServer;
	sendToServer.statusCode = 200;
	strcpy(sendToServer.content,"i want server time.");
	if(send(sofd, &sendToServer,sizeof(dataPacket),0) == -1){
		printf("send() failed!\n");
		close(sofd);
		exit(-1);
	}
}

void sendNameReq(void *sockid){
	int sofd = *(int *)sockid;
	dataPacket sendToServer;
	sendToServer.statusCode = 201;
	strcpy(sendToServer.content,"i want server name.");
	if(send(sofd, &sendToServer,sizeof(dataPacket),0) == -1){
		printf("send() failed!\n");
		close(sofd);
		exit(-1);
	}
}

void sendClientListReq(void *sockid){
	int sofd = *(int *)sockid;
	dataPacket sendToServer;
	sendToServer.statusCode = 203;
	strcpy(sendToServer.content,"i want your client list.");
	if(send(sofd, &sendToServer,sizeof(dataPacket),0) == -1){
		printf("send() failed!\n");
		close(sofd);
		exit(-1);
	}
}