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

#define perr(s) { perror(s); return 1; }

#define pferr(cond, s, arg) { if (cond) { printf(s, arg); return 1;} }

#define proner(func, s) { if (func) { perror(s); return 1; } } // PRintONERror :-)

int main(int argn, char* argv[])
{
	pferr(argn != 3, "Wrong number of arguments: %d\n", argn - 1);

	int port = 0;
	for (int i = 0; argv[1][i] != '\0'; ++i)
	{
		char c = argv[1][i];
		pferr(c < '0' || c > '9', "Wrong port: %s\n", argv[1]);
		port *= 10;
		port += c - '0';
	}
	pferr(port < 0 || port >= 0xFFFF, "Wrong port: %d\n", port);

	struct addrinfo hints;
    struct addrinfo *result, *rp;
    int sock;
    
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;  
    hints.ai_protocol = IPPROTO_TCP;
    
    if (getaddrinfo(NULL, argv[1], &hints, &result) != 0)
    {
    	perror("getaddrinfo");
        return 1;
    }

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sock == -1)
            continue;

        int one;
        if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int)) == -1)
        {
        	close(sock);
            return 1;
        }

        if (bind(sock, rp->ai_addr, rp->ai_addrlen) == 0)
            break;

        close(sock);
    }
    
    if (rp == NULL) 
        return 1;

    freeaddrinfo(result);
	if (listen(sock, 1) < 0)
	{
		perror("listen"); close(sock);
		return 1;
	}

	while (1)
	{
		struct sockaddr_in client;
		socklen_t sz = sizeof(client);
		int fd = accept(sock, (struct sockaddr*)&client, &sz);
		if (fd < 0)
		{
			perror("accept"); close(sock);
			return 1;
		} 
		int f = fork();
		if (f < 0)
		{
			perror("fork");	close(sock); close(fd);
			return 1;
		}
		if (f == 0)
		{
			close(sock);
			int file = open(argv[2], O_RDONLY);
			if (file < 0){
				printf("Can't read file: %s\n", argv[2]); close(fd);
				return 1;
			}
			struct buf_t* buf = buf_new(BUF_SIZE);
			while (1)
			{
				ssize_t rc = buf_fill(file, buf, buf_capacity(buf));
				if (rc < 0){
					printf("Can't read file: %s\n", argv[2]); close(file); close(fd);
					return 1;		
				} 
				if (rc == 0)
					break;
				if (buf_flush(fd, buf, rc) < 0){
					printf("Can't write client file\n"); close(file); close(fd);
					return 1;			
				}
			}
			close(file); close(fd);
			buf_free(buf);
			return 0;
		}
		close(fd);
	}
	close(sock);
	return 0;
}
