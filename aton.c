/*4 byte dot seperated. 4 byte value will be result wherein a.b.c.d will be in format 'a' will be in LSByte and 'd' will be in 
 * MSByte.*/

#include<stdio.h>
       #include <sys/socket.h>
       #include <netinet/in.h>
       #include <arpa/inet.h>

#define IP_STRING "49.248.39.204"

int main()
{
	int ret=0xFF;
	struct in_addr st_addr;

	ret = inet_aton(IP_STRING, &st_addr);
	if(!ret)
	{
		printf("Invalid address\r\n");
		return 0;
	}

	printf("Bin Value = %x\r\n", st_addr.s_addr);

}
