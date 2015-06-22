#define _GNU_SOURCE
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/ioctl.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <netdb.h>
#include <limits.h>
#include <signal.h>
#include <poll.h>
#include <errno.h>

#include "bufio.h"

#define err(s) {perror(s);return 1;}

int get_port(char* s) {
    ssize_t res = 0;
    for (int i = 0; s[i] != '\0'; ++i){
        if (s[i] < '0' || s[i] > '9') {
            return -1;
        }
        res = res * 10 + (s[i] - '0');
    }
    return (res > (1 << 16) || res == 0) ? -1 : res;
}

void swapp(struct buf_t** a, struct buf_t** b) {
    struct buf_t* tmp = *a;
    *a = *b;
    *b = tmp;
}

void sigpipe_off(int sig) {}

int get_socket(char* argv){
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int sock;
    
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;  
    hints.ai_protocol = IPPROTO_TCP;
    
    if (getaddrinfo(NULL, argv, &hints, &result) != 0)
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
    if (listen(sock, 1) == -1)
        err("listen");  
    return sock;
}

int main(int argn, char** argv) {
    if (argn != 3) {
        printf("usage: polling <port 1> <port 2>\n");
        return 0;
    }
    if (get_port(argv[1]) == -1 || get_port(argv[2]) == -1) {
        printf("wrong port\n");
        return 0;
    }

    int clientfd;
    struct pollfd fds[256];
    memset(fds, 0, 256 * sizeof(struct pollfd));
    fds[0].fd = get_socket(argv[1]);
    fds[0].events = POLLIN;
    fds[1].fd = get_socket(argv[2]);
    fds[1].events = 0;
    nfds_t nfds = 2;
    struct buf_t* bufs[127][2];
    for (int i = 0; i < 127; i++) {
        bufs[i][0] = buf_new(4096);
        bufs[i][1] = buf_new(4096);
        if (bufs[i][0] == 0 || bufs[i][1] == 0) {
            err("buf malloc");
        }
    }
    int fails[256];//0 - ok, 1 - read, 2 - write, 3 - read&write
    int state = 0;
    while (1) {
        int polls = poll(fds, nfds, -1);
        if (polls == -1) {
            if (errno == EINTR) {
                continue;
            } else {
                err("poll");
            }
        }

        int oldnfds = nfds;
        if (state == 0 && (fds[0].revents & POLLIN) > 0){
            clientfd = accept(fds[0].fd, NULL, NULL);
            if (clientfd < 0){
                return 1;
            }

            state = 1;
            fds[0].events = 0;
            fds[1].events |= POLLIN;
        }

        if (state == 1 && (fds[1].revents & POLLIN) > 0) {
            int clientfd2 = accept(fds[1].fd, NULL, NULL);
            if (clientfd2 < 0){
                return 1;
            }
            state = 0;
            fds[0].events |= POLLIN;
            fds[1].events = 0;

            buf_clear(bufs[nfds / 2][0]);
            buf_clear(bufs[nfds / 2][1]);
            
            fds[nfds].fd = clientfd;
            fds[nfds].events = POLLIN;
            fails[nfds] = 0;
            nfds++;

            fds[nfds].fd = clientfd2;
            fds[nfds].events = POLLIN;
            fails[nfds] = 0;
            nfds++;
            if (nfds >= 254){
                fds[0].events = 0;
                fds[1].events = 0;
            }
        }

        for (int i = 2; i < oldnfds; i++) {
            if ((fds[i].revents & POLLIN) > 0) {
                int buf = i / 2 - 1;
                int number = i % 2;
                if (buf_fill_once(fds[i].fd, bufs[buf][number]) <= 0) {
                    shutdown(fds[i].fd, SHUT_RD);
                    fails[i] |= 1;
                    if (i % 2 == 0) {
                        fails[i + 1] |= 2;
                    } else {
                        fails[i - 1] |= 2;
                    }
                }
            }
        }
        for (int i = 2; i < oldnfds; i++) {
            if ((fds[i].revents & POLLOUT) > 0) {
                int buf = i / 2 - 1;
                int number = i % 2;
                if (buf_flush(fds[i].fd, bufs[buf][1 - number], buf_size(bufs[buf][1 - number])) <= 0) {
                    shutdown(fds[i].fd, SHUT_WR);
                    fails[i] |= 2;
                    if (i % 2 == 0) {
                        fails[i + 1] |= 1;
                    } else {
                        fails[i - 1] |= 1;
                    }
                }
            }
        }
        for (int i = oldnfds / 2 - 1; i >= 0; i--) {
            fds[2 + 2 * i].events = 0;
            fds[3 + 2 * i].events = 0;
            if (buf_size(bufs[i][0]) > 0 && (fails[3 + 2 * i] & 2) == 0) {
                fds[3 + 2 * i].events |= POLLOUT;
            }
            if (buf_size(bufs[i][0]) < buf_capacity(bufs[i][0]) && (fails[2 + 2 * i] & 1) == 0) {
                fds[2 + 2 * i].events |= POLLIN;
            }
            if (buf_size(bufs[i][1]) > 0 && (fails[2 + 2 * i] & 2) == 0) {
                fds[2 + 2 * i].events |= POLLOUT;
            }
            if (buf_size(bufs[i][1]) < buf_capacity(bufs[i][1]) && (fails[3 + 2 * i] & 1) == 0) {
                fds[3 + 2 * i].events |= POLLIN;
            }
        }
        for (int i = 2; i < nfds; i += 2) {
            if (fails[i] == 3 || fails[i + 1] == 3 || (fails[i] == fails[i + 1] && (fails[i] == 1 || fails[i] == 2))) {
                close(fds[i].fd);
                close(fds[i + 1].fd);
                fds[i] = fds[nfds - 2];
                fds[i + 1] = fds[nfds - 1];
                fails[i] = fails[nfds - 2];
                fails[i + 1] = fails[nfds - 1];
                swapp(&bufs[(i - 2) / 2][0], &bufs[(nfds - 3) / 2][0]);
                swapp(&bufs[(i - 2) / 2][1], &bufs[(nfds - 3) / 2][1]);
                nfds -= 2;
                if (nfds <= 254){
                    fds[0].events |= POLLIN;
                }
            }
        }
    }

    close(fds[0].fd);
    close(fds[1].fd);
    return 0;
}
