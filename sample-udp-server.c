#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORTNUM 8000
#define BUFSIZE 4096

int main(void)
{
	struct sockaddr_in saddr, caddr;
	int                fd1, ret, len;
	char               buf[BUFSIZE];
	
	/* make server's socket */
	if ((fd1 = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		perror("socket");
		return -1;
	}

	memset(&saddr, 0, sizeof(saddr));
	saddr.sin_family      = AF_INET;
	saddr.sin_port        = htons(PORTNUM);
	saddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if(bind(fd1, (struct sockaddr*)&saddr, sizeof(saddr))) {
		perror("bind");
		return -1;
	}

	if ((ret = recvfrom(fd1, buf, BUFSIZE, 0, 
			    (struct sockaddr*)&caddr, &len)) > 0) {
		write(1,buf,ret);
	}

	close(fd1);
	
	return 0;
}
