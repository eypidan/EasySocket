/*
	statusCode:
	666 -> sucess code
	200 -> get time
	201 -> get name
	203 -> get client list
	204 -> send info to another client
	403 -> bad request
	10086 -> packet from another server
*/
//a easy socket sever
#include <stdio.h>
#include <iostream>
#include <cstdlib>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h> 
#include <vector>
#include <string>
#include <time.h>
#include <signal.h>
#include <limits.h>
#include <memory>
#define SERVER_PORT	5750 //listen port
#define CLIENT_NUM 100
#define BUFFERSIZE 2000
#define QUEUEKEY 9999
using namespace std;

extern int errno;

int listenfd;
int numberOfResponse = 0;
//data packet
struct dataPacket{
	char content[BUFFERSIZE];
	int statusCode;
};


//client list struct
class client{
public:
	int fd; //handle in windows ,file descriptor in linux
	char ip[20];
	unsigned int port;

	client(int input_fd, char *input_ip, unsigned int port){
		this->fd = input_fd;
		strcpy(ip,input_ip);
		this->port = port;
	}

	~client(){
		close(fd);
	}
};

//thread function to create a new thread accept next request
void *createNewThread(void *vargp);
void sendClientList(int socketid);
void deleteClient(int socketid);
void getClientList2Packet(char *pacAddr);
void my_handler(int s);

vector<shared_ptr<client>> clientList;

int main() {	

	int connfd, i, *newsock;
	int iClientSize;
	struct sockaddr_in serverAddr;
	struct sockaddr_in  clientAddr[CLIENT_NUM];
	//create a signale struct to catch ctrl+C stop signal
	struct sigaction sigIntHandler;
	sigIntHandler.sa_handler = my_handler;
	sigemptyset(&sigIntHandler.sa_mask);
	sigIntHandler.sa_flags = 0;
	sigaction(SIGINT,&sigIntHandler,NULL);
	//create a socket:
	if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		printf("socket() failed! code:%d\n", errno);
		return -1;
	}
	
	//bind:
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	bzero(&(serverAddr.sin_zero), 8);
	if(bind(listenfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
		printf("bind() failed! code:%d\n", errno);
		close(listenfd);
		return -1;
	}
		
	// 侦听控制连接请求：
	if(listen(listenfd, 100) == -1) {
		printf("listen() failed! code:%d\n", errno);
		close(listenfd);
		return -1;
	}
	

	printf("Waiting for client connecting!\n");
	printf("tips : Ctrl+c to quit!\n");
	iClientSize = sizeof(struct sockaddr_in);
	//接受客户端连接请求：
	i = 0;
	while(i < 100){
		pthread_t thread_id;
		newsock = new int;
		if((connfd = accept(listenfd, (struct sockaddr *)(clientAddr+i),(socklen_t *) &iClientSize)) == -1) {
			printf("accept() failed! code:%d\n", errno);
			close(listenfd);
			return -1;
		}
		*newsock = connfd;
		printf("Accepted client: %s:%d\n", inet_ntoa(clientAddr[i].sin_addr), ntohs(clientAddr[i].sin_port));
		shared_ptr<client> client_ptr;
		client_ptr = make_shared<client>(connfd, inet_ntoa(clientAddr[i].sin_addr), ntohs(clientAddr[i].sin_port));
		clientList.push_back(client_ptr);


		if(pthread_create(&thread_id , NULL , createNewThread ,(void *)newsock)) {
			printf("pthread_create failed! code:%d\n", errno);
			close(listenfd);
			return -1;
		}
		++i;
	}

	close(listenfd);//关闭套接字
	return 0;
}

void *createNewThread(void *socketfd){
	int ret;
	void *ptr;
	int socketid = *(int *)socketfd;
	int socketTarget;
	struct dataPacket dataSendToClient,dataGetFromClient;

	dataSendToClient.statusCode = 666;
	sprintf(dataSendToClient.content,"Hello! We are connected!Your fd is %d",socketid);

	printf("New thread is created\n");	
	printf("Socketid is %d\n",socketid);
	if(send(socketid , &dataSendToClient , sizeof(dataPacket) , 0) == -1) {
		printf("In phtread send() failed!\n");
		deleteClient(socketid);
		pthread_exit(NULL);
	}


	//start to recv packet from client
	while(1){
		socketTarget = socketid;
		int nLeft = sizeof(dataPacket);
		ptr = &dataGetFromClient;
		while(nLeft >0)	{
			//接收数据：
			ret = recv(socketid, ptr, nLeft, 0);
			//printf("debug,if not recv hang you will see me\n");
			if(ret <= 0) {
				printf("this thread recv() failed!\n");
				deleteClient(socketid);
				pthread_exit(NULL);
			}
			nLeft -= ret;
			ptr = (char *)ptr + ret;
			//printf("degbug :nLeft %d\n",nLeft);
		}
		
		switch(dataGetFromClient.statusCode){
			//classify the req according to statusCode
			case 200:{
				time_t t = time(NULL);
				struct tm tm = *localtime(&t);
				sprintf(dataSendToClient.content,"now: %d-%d-%d %d:%d:%d", tm.tm_year + 1900, tm.tm_mon + 1,tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
				dataSendToClient.statusCode = 666;
				// dataSendToClient.content
			}
			break;

			case 201:{
				char hostname[HOST_NAME_MAX];
				gethostname(hostname, HOST_NAME_MAX);
				sprintf(dataSendToClient.content,"i am %s\n",hostname);
				dataSendToClient.statusCode = 666;
			}
			break;

			case 203:{
				getClientList2Packet(dataSendToClient.content);
				dataSendToClient.statusCode = 666;
			}
			break;
			
			case 204:{
				int targetClientFd = 0;
				for(int a = 0; a<4 ;a++)
					targetClientFd = 256 * targetClientFd + dataGetFromClient.content[a];
				
				printf("debug:target fd :%d\n",targetClientFd);			
				socketTarget = targetClientFd;
				strcpy(dataSendToClient.content,dataGetFromClient.content+4);
				dataSendToClient.statusCode = 10086;
			}
			break;

			default:{
				printf("bad request");
				sprintf(dataSendToClient.content,"bad request");
				dataSendToClient.statusCode = 403;
			}
			break;
		}
		if(send(socketTarget, &dataSendToClient,sizeof(dataPacket),0) == -1){
			printf("in pthread send() failed!\n");
			deleteClient(socketid);
			pthread_exit(NULL);
		}
		numberOfResponse++;
		printf("debug:%s\n",dataSendToClient.content);
		printf("debug:number Of Response package is %d\n",numberOfResponse);
	}
	
}

void deleteClient(int socketid){
	close(socketid);
	vector<shared_ptr<client>>::iterator iter;
	for(iter = clientList.begin(); iter != clientList.end(); ++iter){
		
		if((*iter)->fd == socketid){
			printf("get it,delete it\n");
			clientList.erase(iter);
			break;
		}
	}
}

void getClientList2Packet(char *pacAddr){

	vector<shared_ptr<client>>::iterator iter;
	for(iter = clientList.begin(); iter != clientList.end(); ++iter){
		sprintf(pacAddr,"fd:%d,ip:%s,port:%d\n",(*iter)->fd,(*iter)->ip,(*iter)->port);
		int x = strlen(pacAddr);
		pacAddr += x;
	}

}

void my_handler(int s){
	
	vector<shared_ptr<client>>::iterator iter;
	for(iter = clientList.begin(); iter != clientList.end(); ++iter)
		close((*iter)->fd);
	
	close(listenfd);
	printf("Caught signale %d, all socket connections are stopped.\n",s);
	exit(0);
}