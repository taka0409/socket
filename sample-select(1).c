#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>

#define PORTNUM 8000 
#define BUFSIZE 4096

long getFileSize(char *file)
{
    struct stat statBuf;
 
    if (stat(file, &statBuf) == 0)
        return statBuf.st_size;
 
    return -1L;
}

int main(void)
{
	struct sockaddr_in saddr, caddr;
	int                fd1, fd2[BUFSIZE], ret, len,i,yes=1,fdnum=0;
	long size;
	char               buf[BUFSIZE],tmp[BUFSIZE],s[BUFSIZE];
	fd_set             fds, prev_fds;
	FILE *fp;

	/* make server's socket */
	if ((fd1 = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		perror("socket");
		return -1;
	}

	memset(&saddr, 0, sizeof(saddr));
	saddr.sin_family      = AF_INET;
	saddr.sin_port        = htons(PORTNUM);
	saddr.sin_addr.s_addr = htonl(INADDR_ANY);
	setsockopt(fd1, SOL_SOCKET, SO_REUSEADDR, (const char *)&yes, sizeof(yes));

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
			if((fd2[fdnum] = accept(fd1, (struct sockaddr*)&caddr, &len)) < 0) {
				perror("accept");
				exit(1);
			}
			FD_SET(fd2[fdnum], &prev_fds);
			fdnum++;
			continue;
		}
		for(i=0;i<fdnum;i++){
			if (FD_ISSET(fd2[i], &fds)) {
				if ((ret = recv(fd2[i], buf, BUFSIZE, 0)) > 0) {
					if(strncmp(buf,"GET",3)==0){
						i=0;
						while(buf[i+4]!='>')
							i++;
						strncpy(tmp,&buf[4],i);
						if ((fp = fopen(tmp, "r")) == NULL) {
							printf("NOT FOUND\n");
						}
						else{
							size=getFileSize(tmp);
							printf("FILE(%ld): ",size);
							while (fgets(s, 256, fp) != NULL) {
								printf("%s", s);
							}
							fclose(fp);
							puts("");
						}				
						send(fd2[i], buf, strlen(buf), 0);
					}
					else if(strncmp(buf,"PUT",3)==0){
						i=0;
						while(buf[i+4]!='>')
							i++;
						strncpy(tmp,&buf[4],i);
						if ((fp = fopen(tmp, "w")) == NULL) {
							printf("FAIL\n");
						}
						else{
							send(fd2[i], buf, strlen(buf), 0);
							ret = recv(fd2[i], s, BUFSIZE, 0);
							fputs(s,fp);
							fclose(fp);
							printf("put: %s\n",tmp);
						}
						send(fd2[i], buf, strlen(buf), 0);
					}
					else if(strncmp(buf,"LS",2)==0){
						if((ret=fork())==0){
							if (execlp("ls", "ls",NULL,NULL) < 0)
								perror("execvp");
						}
						puts("");
						send(fd2[i], buf, strlen(buf), 0);
					}
					else if(strncmp(buf,"DEL",3)==0){
						i=0;
						while(buf[i+4]!='>')
							i++;
						strncpy(tmp,&buf[4],i);
						if(remove(tmp)==0){
							printf("delete: %s\n",tmp);
						}else{
							puts("fail");
						}
						send(fd2[i], buf, strlen(buf), 0);
					}
					else{
						write(1,buf,ret);
						puts("");
						send(fd2[i], buf, strlen(buf), 0);	
					}
				}
			}
		}
	}
	close(fd1);
	
	return 0;
}
