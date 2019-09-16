#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <unistd.h>
#include <netdb.h>   
//10.79.25.117 
#define SERVER_PORT	5750 //侦听端口

extern int errno;

//客户端向服务器传送的结构：
struct student {
	char name[32];
	int age;
};

int main(int argc, char *argv[])
{	
	int sockfd;
	struct sockaddr_in serverAddr;
	struct student stu;
	struct hostent *hptr;  
	char *ptr;
    char message[100] , timeMessage[100] , nameMessage[100];
    // char *helloptr , *timePtr, *namePtr;
	if(argc != 4) {
		printf("usage: informLinuxClient serverIP name age\n");
		return -1;
	}

	// 创建一个socket：
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		printf("socket() failed! code:%d\n", errno);
		return -1;
	}
	
	ptr = argv[1];
	// 设置服务器的地址信息：
	if((hptr = gethostbyname(ptr)) == NULL){
		printf("Geting error in gethostbyname func!\n");
		return 0;
	}
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	//serverAddr.sin_addr.s_addr = inet_addr(argv[1]);
	memcpy(&serverAddr.sin_addr, hptr->h_addr, hptr->h_length);
	bzero(&(serverAddr.sin_zero), 8);
	printf("set sucessfully.\nIP:%d\nserver address:%s\n",SERVER_PORT,argv[1]);
	//客户端发出连接请求：
	printf("connecting....\n");
	if(connect(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
		printf("connect() failed! code:%d\n", errno);
		close(sockfd);
		return -1;
	}
	printf("Connected!\n");
	
	// strcpy(stu.name, argv[2]);
	// stu.age = atoi(argv[3]);
	// if(send(sockfd, (char *)&stu, sizeof(stu), 0) == -1)
	// {
	// 	printf("send() failed!\n");
	// 	close(sockfd);
	// 	return -1;
	// }

	// printf("student info has been sent!\n");
    //get hello info from server
    if(recv(sockfd , message , 100 , 0) <= 0) {
        printf("recv() failed!\n");
        close(sockfd);
        return -1;
    }
    printf("info from sever:%s\n" , message);
	close(sockfd);//关闭套接字
	return 0;
}
