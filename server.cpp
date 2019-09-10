/*
 *	���������ҪĿ������˵��socket��̵Ļ������̣����Է�����/�ͻ��˵Ľ������̷ǳ��򵥣�
 *  ֻ���ɿͻ��������������һ��ѧ����Ϣ�Ľṹ��
 */
//informLinuxServer.cpp������ʱû�в�����ʹ�ö˿�6666����������
#include <stdio.h>
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

int main()
{	
	
	int sockfd, comfd;
	struct sockaddr_in serverAddr, clientAddr;
	int ret, iClientSize;
	struct student stu;
	void *ptr;
	
	// ����һ��socket��
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		printf("socket() failed! code:%d\n", errno);
		return -1;
	}
		
	// �󶨣�
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	bzero(&(serverAddr.sin_zero), 8);
	if(bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
	{
		printf("bind() failed! code:%d\n", errno);
		close(sockfd);
		return -1;
	}
		
	// ����������������
	if(listen(sockfd, 5) == -1)
	{
		printf("listen() failed! code:%d\n", errno);
		close(sockfd);
		return -1;
	}
		
	printf("Waiting for client connecting!\n");
	printf("tips : Ctrl+c to quit!\n");
	
	//���ܿͻ�����������
	iClientSize = sizeof(struct sockaddr_in);
	if((comfd = accept(sockfd, (struct sockaddr *)&clientAddr,(socklen_t *) &iClientSize)) == -1)
	{
		printf("accept() failed! code:%d\n", errno);
		close(sockfd);
		return -1;
	}

	printf("Accepted client: %s:%d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
	
	//���տͻ��˵����ݣ�		
	int nLeft = sizeof(stu);
	ptr = &stu;
	while(nLeft >0)
	{
		//�������ݣ�
		ret = recv(comfd, ptr, nLeft, 0);
		if(ret <= 0)
		{
			printf("recv() failed!\n");
			close(sockfd);//�ر��׽���
			close(comfd);
			return -1;
		}
	
		nLeft -= ret;
		ptr = (char *)ptr + ret;
	}
	
	printf("name: %s\nage:%d\n", stu.name, stu.age);

	close(sockfd);//�ر��׽���
	close(comfd);
	return 0;
}
