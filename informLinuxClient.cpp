/*
 *	���������ҪĿ������˵��socket��̵Ļ������̣����Է�����/�ͻ��˵Ľ������̷ǳ��򵥣�
 *  ֻ���ɿͻ��������������һ��ѧ����Ϣ�Ľṹ��
 */
//informLinuxClient.cpp������Ϊ serverIP name age
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

#define SERVER_PORT	6666 //�����˿�

extern int errno;

//�ͻ�������������͵Ľṹ��
struct student
{
	char name[32];
	int age;
};

int main(int argc, char *argv[])
{	
	int sockfd;
	struct sockaddr_in serverAddr;
	struct student stu;
	
	if(argc != 4)
	{
		printf("usage: informLinuxClient serverIP name age\n");
		return -1;
	}

	// ����һ��socket��
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		printf("socket() failed! code:%d\n", errno);
		return -1;
	}
		
	// ���÷������ĵ�ַ��Ϣ��
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	serverAddr.sin_addr.s_addr = inet_addr(argv[1]);
	bzero(&(serverAddr.sin_zero), 8);
		
	//�ͻ��˷�����������
	printf("connecting!\n");
	if(connect(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
	{
		printf("connect() failed! code:%d\n", errno);
		close(sockfd);
		return -1;
	}

	printf("Connected!\n");
	
	strcpy(stu.name, argv[2]);
	stu.age = atoi(argv[3]);
	if(send(sockfd, (char *)&stu, sizeof(stu), 0) == -1)
	{
		printf("send() failed!\n");
		close(sockfd);
		return -1;
	}

	printf("student info has been sent!\n");
	close(sockfd);//�ر��׽���
	return 0;
}
