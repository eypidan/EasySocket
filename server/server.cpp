//a easy socket sever
#include <stdio.h>
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
#define SERVER_PORT	5750 //listen port
#define CLIENT_NUM 100
using namespace std;
extern int errno;

//client list struct
class client{
public:
	int fd; //handle in windows ,file descriptor in linux
	void setRetval(){
		len = sizeof (error);
		retval = getsockopt (fd, SOL_SOCKET, SO_ERROR, &error, &len);
	}
	bool getStatus(){
		if (retval != 0) {
			/* there was a problem getting the error code */
			fprintf(stderr, "error getting socket error code: %s\n", strerror(retval));
			return false;
		}

		if (error != 0) {
			/* socket has a non zero error status */
			fprintf(stderr, "socket error: %s\n", strerror(error));
			return false;
		}
		return true;
	}
private:
//three varible for check sokcet status
	socklen_t len;
	int error = 0;
	int retval;
};

//客户端向服务器传送的结构：
struct student {
	char name[32];
	int age;
};

//thread function to create a new thread accept next request
void *createNewThread(void *vargp);

pthread_t thread_id[CLIENT_NUM];
vector<client> clientList;

int main() {	

	int listenfd, connfd, i;
	int iClientSize;
	struct sockaddr_in serverAddr;
	struct sockaddr_in  clientAddr;
	//create a socket:
	if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
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
	if(listen(listenfd, CLIENT_NUM) == -1) {
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
		if((connfd = accept(listenfd, (struct sockaddr *)&clientAddr,(socklen_t *) &iClientSize)) == -1) {
			printf("accept() failed! code:%d\n", errno);
			close(listenfd);
			return -1;
		}
		printf("Accepted client: %s:%d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
		pthread_create(thread_id+i ,NULL ,createNewThread ,(void *)&connfd);
		i++;
	}
	close(listenfd);//关闭套接字

	
	return 0;
}


void *createNewThread(void *socketfd){
	
	struct student stu;
	int ret;
	int *socketid_ptr = (int *)socketfd;
	void *ptr;
	client *client_ptr;
	int socketid = *socketid_ptr;
	printf("New thread is created");
	
	client_ptr = new client;
	client_ptr->fd = socketid;
	client_ptr->setRetval();
	clientList.push_back(*client_ptr);
	
	//接收客户端的数据：		
	int nLeft = sizeof(stu);
	ptr = &stu;
	while(nLeft >0)	{
		//接收数据：
		ret = recv(socketid, ptr, nLeft, 0);
		if(ret <= 0) {
			printf("recv() failed!\n");
			close(socketid);
			exit(-1);
		}
	
		nLeft -= ret;
		ptr = (char *)ptr + ret;
	}
	
	printf("name: %s\nage:%d\n", stu.name, stu.age);
	printf("Socketid is %d\n",socketid);
	//close(socketid);
}