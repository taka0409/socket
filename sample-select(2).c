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
#include <sys/select.h>

#define PORTNUM 8000 
#define BUFSIZE 4096

int main(void)
{
	struct sockaddr_in saddr, caddr;
	int                fd1, fd2, ret, len;
	char               buf[BUFSIZE];
	
	fd_set             fds, prev_fds;
	

	/* make server's socket */
	if ((fd1 = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
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

	if(listen(fd1, 5)) {
		perror("listen");
		return -1;
	}

	FD_ZERO(&prev_fds);
	FD_SET(fd1, &prev_fds);
	fds = prev_fds;
	for (;;) {
		fds = prev_fds;
		if ((ret = select(FD_SETSIZE, &fds, 0, 0, 0)) <= 0) {
			perror("select");
			return -1;
		}

		if (FD_ISSET(fd1, &fds)) {
			if((fd2 = accept(fd1, (struct sockaddr*)&caddr, &len)) < 0) {
				perror("accept");
				exit(1);
			}
			FD_SET(fd2, &prev_fds);
			continue;
		}

		if (FD_ISSET(fd2, &fds)) {
			if ((ret = recv(fd2, buf, BUFSIZE, 0)) > 0) {
				write(1,buf,ret);
			}
			close(fd2);
			FD_CLR(fd2, &prev_fds);
		}
	}
	close(fd1);
	
	return 0;
}
