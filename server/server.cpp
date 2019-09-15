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
#include "./include/json.hpp"

using json = nlohmann::json;
#define SERVER_PORT	5750 //listen port
#define CLIENT_NUM 100
using namespace std;
extern int errno;

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


//thread function to create a new thread accept next request
void *createNewThread(void *vargp);
void sendClientList(int socketid);

pthread_t thread_id[CLIENT_NUM];
vector<client> clientList;

// namespace ns{
// 	void to_json(json& j, const client& c) {
//         j = json{{"client_fd", c.fd}, {"ip", p.address}, {"client_port", p.age}};
//     }

//     void from_json(const json& j, client& p) {
//         j.at("name").get_to(p.name);
//         j.at("address").get_to(p.address);
//         j.at("age").get_to(p.age);
//     }
// }

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

		client *client_ptr;
		client_ptr = new client(connfd, inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
		client_ptr->setRetval();
		clientList.push_back(*client_ptr);

		if(pthread_create(thread_id+i ,NULL ,createNewThread ,(void *)&connfd)) {
			printf("pthread_create failed! code:%d\n", errno);
			close(listenfd);
			return -1;
		}
		i++;
	}
	close(listenfd);//关闭套接字

	
	return 0;
}

void *createNewThread(void *socketfd){
	

	int ret;
	int *socketid_ptr = (int *)socketfd;
	void *ptr;
	
	int socketid = *socketid_ptr;
	char messageHello[] = "Hello! We are connected!\n";

	printf("New thread is created\n");


	
	//接收客户端的数据：		
	// int nLeft = sizeof(stu);
	// ptr = &stu;
	// while(nLeft >0)	{
	// 	//接收数据：
	// 	ret = recv(socketid, ptr, nLeft, 0);
	// 	if(ret <= 0) {
	// 		printf("recv() failed!\n");
	// 		close(socketid);
	// 		exit(-1);
	// 	}
	
	// 	nLeft -= ret;
	// 	ptr = (char *)ptr + ret;
	// }
	
	// printf("name: %s\nage:%d\n", stu.name, stu.age);
	printf("Socketid is %d\n",socketid);
	if(send(socketid , messageHello , strlen(messageHello) , 0) == -1){
		printf("send() failed!\n");
		close(socketid);
		exit(-1);
	}
	sendClientList(socketid);
	//close(socketid);
}

void sendClientList(int socketid){
	json currentList;
	
	for(vector <client>::iterator iter = clientList.begin();iter != clientList.end();++iter){

	}
	
	currentList["pi"] = 3.14;
	std::cout << currentList.dump() << std::endl;
	// if(send(socketid , messageHello , strlen(messageHello) , 0) == -1){
	// 	printf("send() failed!\n");
	// 	close(socketid);
	// 	exit(-1);
	// }
}