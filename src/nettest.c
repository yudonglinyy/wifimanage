#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <fcntl.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/select.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <assert.h>
#include <getopt.h>
#include "log.h"
#include "nettest.h"
#include "cmdcall.h"

bool conn_tcpport(const char *serverIP, int serverport, int timeout)
{
    assert(serverIP != NULL && *serverIP != '\0');
    int sockfd;
    struct hostent *h;
    struct sockaddr_in server_addr;

    if (((h=gethostbyname(serverIP)) == NULL)) {
        LOG("gethostbyname %s fail", serverIP);
    }

    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    // server_addr.sin_port = htons(serverport);
    server_addr.sin_port = htons(22);
    server_addr.sin_addr = *((struct in_addr *)h->h_addr);


    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        LOG("client socket failure");
    }

    bool res = true;
    int opt = 1;
    //set non-blocking
    if (ioctl(sockfd, FIONBIO, &opt) < 0) {
        close(sockfd);
        LOG("ioctl");
    }

    if (connect(sockfd, (struct sockaddr *) &server_addr, sizeof(struct sockaddr)) == -1) {
        if (errno == EINPROGRESS) {
            int error;
            int len = sizeof(int);
            struct timeval tv_timeout;
            fd_set writefds;

            tv_timeout.tv_sec  = timeout;
            tv_timeout.tv_usec = 0;

            FD_ZERO(&writefds);
            FD_SET(sockfd, &writefds);

            if(select(sockfd + 1, NULL, &writefds, NULL, &tv_timeout) > 0) {
                getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, (socklen_t *)&len);
                if(error != 0) {
                    res = false;
                }
            } else { //timeout or select error
                res = false;
            }
        } else {
            res = false;
        }
    }

    close(sockfd);
    return res;
}

bool conn_udpport(const char *serverIP, int serverport, int timeout)
{
    char cmd[100];
    /*'-u' will try to conn udp port*/
    sprintf(cmd, "nc -w %d -zu %s %d", timeout, serverIP, serverport);
    return !cmdcall_no_output(cmd);
}

bool test_conn_vpn_server(char *ip)
{
    /*The max fail times for connecting the remote server is 1, timeout is 5, success will true*/
    if (conn_tcpport(ip, 1723, 5) || conn_udpport(ip, 1701, 5)) {
        return true;
    }
    return false;
}