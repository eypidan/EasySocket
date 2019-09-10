//a easy socket sever
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <unistd.h>

#define SERVER_PORT	5750 //侦听端口

extern int errno;

//客户端向服务器传送的结构：
struct student
{
	char name[32];
	int age;
};

int main()
{	
	
	int listenfd, connfd;
	struct sockaddr_in serverAddr, clientAddr;
	int ret, iClientSize;
	struct student stu;
	void *ptr;
	
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
	if(listen(listenfd, 5) == -1)
	{
		printf("listen() failed! code:%d\n", errno);
		close(listenfd);
		return -1;
	}
		
	printf("Waiting for client connecting!\n");
	printf("tips : Ctrl+c to quit!\n");
	
	//接受客户端连接请求：
	iClientSize = sizeof(struct sockaddr_in);
	if((connfd = accept(listenfd, (struct sockaddr *)&clientAddr,(socklen_t *) &iClientSize)) == -1)
	{
		printf("accept() failed! code:%d\n", errno);
		close(listenfd);
		return -1;
	}

	printf("Accepted client: %s:%d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
	
	//接收客户端的数据：		
	int nLeft = sizeof(stu);
	ptr = &stu;
	while(nLeft >0)
	{
		//接收数据：
		ret = recv(connfd, ptr, nLeft, 0);
		if(ret <= 0)
		{
			printf("recv() failed!\n");
			close(listenfd);//关闭套接字
			close(connfd);
			return -1;
		}
	
		nLeft -= ret;
		ptr = (char *)ptr + ret;
	}
	
	printf("name: %s\nage:%d\n", stu.name, stu.age);

	close(listenfd);//关闭套接字
	close(connfd);
	return 0;
}
