#include<stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include<string.h>
#include<stdlib.h>
#include<fcntl.h>
#include<unistd.h>


#define DEBUG

//#define IP_STRING "8.8.8.8"
#define IP_STRING "147.182.252.2"
#define LOOPBACK_IP "127.0.0.1"
#define WRITE_BUFF_SIZE 4
#define READ_BUFF_SIZE 2000
#define PACKET_FILE "HTTPtestReq"

void rcvRoutine(char **);
void sendRoutine(char *);
void createSendPacket(char **);
unsigned int sizeofFile(int );

int sockFD[2], retval;


int main()
{
	struct sockaddr st_sock;
	struct addrinfo st_sockInfo, *st_output;	//st_sockInfo to have expectation,
							//st_output to have required details which will be used in further syscalls
	/*unsigned*/ char *writeBuff, *readBuff;

	sockFD[0] = socket(AF_INET,SOCK_STREAM,0);
	if(sockFD[0] < 0) {
		printf("socket syscall failed\r\n");
		return 0;
	}
	printf("sockFD[0] = %d\r\n",sockFD[0]);


	st_sockInfo.ai_family = AF_INET;
	st_sockInfo.ai_socktype = SOCK_STREAM;
	st_sockInfo.ai_protocol = 0;
	st_sockInfo.ai_flags = AI_NUMERICSERV /*| AI_NUMERICHOST*/;	//0
	
	/*1. AI_PASSIVE (used in example, but in manpage no requirement found as per our basic application)
	2. AI_NUMERICSERV flag means you can place decimal number in "service" arg of getaddringo // st_sockInfo.ai_flags
	3. Program also running with AI_NUMERICHOST OR'ed in st_sockInfo.ai_flags
	AI_NUMERICHOST indicates format of the IP= supporting aton()*/

	//all other members are suggested to keep 0 or NULL, depending on the type of member
	st_sockInfo.ai_addrlen = 0;
	st_sockInfo.ai_addr = NULL;
	st_sockInfo.ai_canonname = NULL;
	st_sockInfo.ai_next = NULL;

	//getaddrinfo(IP_STRING, "http", &st_sockInfo, &st_output);
	int listCtr = 0;
	getaddrinfo(IP_STRING, "80", &st_sockInfo, &st_output);
	listCtr++;
	printf("Line after 1st getaddrinfo, count = %d\r\n", listCtr);

	while(st_output->ai_next != NULL)
	{
		getaddrinfo(LOOPBACK_IP/*IP_STRING*/, "80", &st_sockInfo, &st_output);
		listCtr++;
		printf("Line after getaddrinfo, count = %d\r\n", listCtr);
	}

	/*memcpy(&st_sock, st_output.ai_addr, sizeof(struct sockaddr));
	retval = connect(sockFD[0], &st_sock, );*/
	retval = connect(sockFD[0], st_output->ai_addr, st_output->ai_addrlen);
	if(retval < 0) {
		printf("connect syscall failed\r\n");
		return 0;
	}

	sendRoutine(writeBuff);

	//printf("readBuff = %s\r\n\r\n", readBuff);
	rcvRoutine(&readBuff);

	//close(sockFD[0]);
}

void rcvRoutine(char **readBuff)
{
	*readBuff = (char *)malloc(READ_BUFF_SIZE);	//500 bytes
	if(*readBuff == NULL) {
		printf("malloc failed\r\n");
		//return 0;
	}
	printf("Allocated %d bytes at addr %p\r\n", READ_BUFF_SIZE, *readBuff);
	//read response
#ifdef DEBUG
	printf("result of strlen(*readBuff) = %d\r\n", strlen(*readBuff));
#endif
	retval = recv(sockFD[0], *readBuff, READ_BUFF_SIZE/*strlen(*readBuff)*/+1, 0);
	if(retval < 0) {
		printf("recv syscall failed\r\n");
		//return 0;
	}
	else {
		printf("%d bytes recv\r\n", retval);
	}

	printf("Response in *readBuff = \r\n%s\r\n\r\n", *readBuff);
#ifdef DEBUG
	printf("Result of strlen(*readBuff) = %d\r\n", strlen(*readBuff));
#endif
	*readBuff = realloc(*readBuff, retval+1);
	if(*readBuff == NULL)
	{
		printf("realloc failed\r\n");
	}
#ifdef DEBUG
	printf("strlen(*readBuff) after realloc = %d\r\n", strlen(*readBuff));
#endif
	free(*readBuff);
}

void sendRoutine(char *writeBuff)
{
	char *tempBuff;
	
	printf("data baseAddr0 = %p\r\n", writeBuff);
	createSendPacket(&writeBuff);	//passing pointer to address. 
	//A ptr can fetch a value from another func, a ptr to an address can fetch me an address from another func

	printf("data baseAddr2 = %p\r\n", writeBuff);
	printf("writeBuff = \r\n%s\r\n\r\n", writeBuff);

	printf("strlen(writeBuff) = %d\r\n", strlen(writeBuff));	//debug print
	retval = send(sockFD[0], writeBuff, strlen(writeBuff), 0);
	if(retval < 0) {
		printf("send syscall failed\r\n");
		//return 0;
	}
	else {
		printf("%d bytes sent\r\n", retval);
	}
	free(writeBuff);
}

void createSendPacket(char **ptr)
{
	int fd, ret, fileSize;
	char *fileReadBuff;
        fd = open(PACKET_FILE, O_RDWR, 666);       //open existing file

        if(fd < 0) {
                printf("Failed to open/create %s file\r\n",PACKET_FILE);
                //return fd;
        }

	fileSize = sizeofFile(fd);
	printf("sizeofFile = %d\r\n", fileSize);
	*ptr = (char *)malloc(fileSize+1);
	if(*ptr == NULL) {
		printf("malloc failed\r\n");
		//return 0;
	}
	printf("Allocated %d bytes at addr %p using malloc\r\n", fileSize+1, *ptr);
	
#ifdef DEBUG
	ret = lseek(fd, 0, SEEK_END);
	printf("ret value of lseek() = %d\r\n", ret);
#endif

	lseek(fd, 0, SEEK_SET);	//reposition the offset to 0
	ret = read(fd, *ptr, fileSize);
	if(ret < 0) {
		printf("read syscall failed\r\n");
		return;
	}
	printf("read %d bytes\r\n", ret);
	printf("data baseAddr = %p\r\n", *ptr);

#ifdef DEBUG
	printf("readBuff = \r\n%s\r\n\r\n", *ptr);
#endif

}


/*Note: This function is used to get the size of a file. open() system call should be called first and 
 * then the corresp. fd should be passed as argument*/

unsigned int sizeofFile(int fd)
{
        return (unsigned int)lseek(fd,0,SEEK_END);
}

