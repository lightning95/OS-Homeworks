#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <netdb.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#include "bufio.h"

#define BUF_SIZE 65536

#define pferr(cond, s, arg) { if (cond) { printf(s, arg); return 1;} }

int create_socket(const char * port)
{
	struct addrinfo hints;
    struct addrinfo *result, *rp;
    int sock;
    
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;  
    hints.ai_protocol = IPPROTO_TCP;
    
    if (getaddrinfo(NULL, port, &hints, &result) != 0)
    {
    	perror("getaddrinfo");
        return -1;
    }

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sock == -1)
            continue;

        int one;
        if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int)) == -1)
        {
        	close(sock);
            return -1;
        }

        if (bind(sock, rp->ai_addr, rp->ai_addrlen) == 0)
            break;

        close(sock);
    }
    
    if (rp == NULL) 
        return -1;

    freeaddrinfo(result);
    return sock;
}

int check_port(char *argv)
{
	int port = 0;
	for (int i = 0; argv[i] != '\0'; ++i)
	{
		char c = argv[i];
		if (c < '0' || c > '9')
			return -1;
		port *= 10;
		port += c - '0';
	}
	return port < 0 || port >= 0xFFFF ? -1 : port;
}

int main(int argn, char* argv[])
{
	pferr(argn != 3, "Wrong number of arguments: %d\nRequired 2\n", argn - 1);
	if (check_port(argv[1]) < 0 || check_port(argv[2]) < 0){
		perror("Wrong port");
		return 1;
	}

	int sock1 = create_socket(argv[1]);
	int sock2 = create_socket(argv[2]);
	if (sock1 < 0 || sock2 < 0)
	{
		perror("sock"); close(sock1); close(sock2);
		return 1;
	}

	if (listen(sock1, 1) < 0 || listen(sock2, 1) < 0)
	{
		perror("listen"); close(sock1); close(sock2);
		return 1;
	}

    struct sockaddr_in client1;
    struct sockaddr_in client2;
    socklen_t sz1 = sizeof(client1);
    socklen_t sz2 = sizeof(client2);

	while (1)
	{
		int fd1 = accept(sock1, (struct sockaddr*)&client1, &sz1);
		int fd2 = accept(sock2, (struct sockaddr*)&client2, &sz2);
		if (fd1 < 0 || fd2 < 0)
		{
			perror("accept"); close(sock1); close(sock2); close(fd1); close(fd2);
			return 1;
		} 
		int f = fork();
		if (f < 0)
		{
			perror("fork");	close(sock1); close(sock2); close(fd1); close(fd2);
			return 1;
		}
		if (f == 0)
		{
			close(sock1); close(sock2);
			struct buf_t* buf = buf_new(BUF_SIZE);
			while (1)
			{
				ssize_t rc = buf_fill_once(fd1, buf);
				if (rc < 0){
					perror("Can't read client1"); close(fd1); close(fd2);
					return 1;		
				} 
				if (rc == 0)
					break;
				if (buf_flush(fd2, buf, rc) < 0){
					perror("Can't write client2"); close(fd1); close(fd2);
					return 1;			
				}
			}
			close(fd1); close(fd2);
			buf_free(buf);
			return 0;
		}
		if ((f = fork()) < 0)
		{
			perror("fork");	close(sock1); close(fd1); close(sock2); close(fd2);
			return 1;
		}
		if (f == 0)
		{
			close(sock1); close(sock2);
			struct buf_t* buf = buf_new(BUF_SIZE);
			while (1)
			{
				ssize_t rc = buf_fill_once(fd2, buf);
				if (rc < 0){
					perror("Can't read client2"); close(fd1); close(fd2);
					return 1;		
				} 
				if (rc == 0)
					break;
				if (buf_flush(fd1, buf, rc) < 0){
					perror("Can't write client1"); close(fd1); close(fd2);
					return 1;			
				}
			}
			close(fd1); close(fd2);
			buf_free(buf);
			return 0;
		}

		close(fd1); close(fd2);
	}
	close(sock1); close(sock2);
	return 0;
}
