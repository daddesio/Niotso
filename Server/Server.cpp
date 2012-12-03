/*
    Niotso Server - Niotso daemon based on PostgreSQL
    Server.cpp
    Copyright (c) 2012 Niotso Project <http://niotso.org/>
    Author(s): Fatbag <X-Fi6@phppoll.org>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <syslog.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include "Server.hpp"

#define SHUTDOWN(X)      do { syslog(LOG_INFO, "<info> Could not recover from errors. Shutting down."); \
                              exitval = EXIT_FAILURE; goto X; } while(0)
#define SHUTDOWN_M(X, Y) do { syslog(LOG_ERR, "<error> %s (%s)", X, strerror(errno)); \
                              SHUTDOWN(Y); } while(0)

//IPV6_V6ONLY

static void term(int)
{
}

int main(int, char **)
{
    const pid_t pid = fork();
    if(pid < 0){
        const char *msg1 = "<error> Failed to create child process (%s).", *msg2 = strerror(errno);
        openlog(SERVER_NAME, 0, LOG_DAEMON);
        syslog(LOG_ERR, msg1, msg2);
        closelog();
        fprintf(stderr, msg1, msg2);
        fprintf(stderr, "\n");
        return EXIT_FAILURE;
    }else if(pid > 0)
        return EXIT_SUCCESS;

    int exitval = EXIT_SUCCESS, sockfd, epollfd;
    { //New scope required for error handling
    int ret;
    FILE * fd;

    umask(0);

    openlog(SERVER_NAME, LOG_PID, LOG_DAEMON);

    if(setsid() < 0)
        SHUTDOWN_M("Failed to create session", close_msg);

    if(chdir(CONFIG_DIR) < 0)
        SHUTDOWN_M("Failed to change into \""CONFIG_DIR"\"", close_msg);

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    struct sigaction sigact;
    sigact.sa_handler = term;
    sigemptyset(&sigact.sa_mask);
    sigact.sa_flags = 0;
    sigaction(SIGTERM, &sigact, NULL);
    sigaction(SIGINT, &sigact, NULL);

    syslog(LOG_INFO, "<info> "SERVER_NAME" (version "VERSIONSTR") is starting...");

    fd = fopen("network.conf", "r");
    if(!fd)
        SHUTDOWN_M("Failed to open \""CONFIG_DIR"/network.conf\"", close_msg);
    unsigned int port = 0;
    ret = fscanf(fd, "%u", &port);
    fclose(fd);
    if(ret < 0)
        SHUTDOWN_M("Failed to read \""CONFIG_DIR"/network.conf\"", close_msg);
    if(port > 65535){
        syslog(LOG_ERR, "<error> Invalid port '%u' specified in \""CONFIG_DIR"/network.conf\".", port);
        SHUTDOWN(close_msg);
    }

    sockfd = socket(AF_INET6, SOCK_DGRAM, 0);
    if(sockfd < 0)
        SHUTDOWN_M("Failed to open socket", close_msg);

    int flags = fcntl(sockfd, F_GETFL, 0);
    if(flags < 0)
        SHUTDOWN_M("Failed to read socket flags", close_socket);
    if(fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) < 0)
        SHUTDOWN_M("Failed to set socket flags", close_socket);

    sockaddr_in6 server_addr, client_addr;
    memset(&server_addr, 0, sizeof(sockaddr_in6));
    memset(&client_addr, 0, sizeof(sockaddr_in6));
    server_addr.sin6_family = AF_INET6;
    server_addr.sin6_port   = htons((uint16_t) port);
    server_addr.sin6_addr   = in6addr_any;
    client_addr.sin6_family = AF_INET6;
    client_addr.sin6_addr   = in6addr_any;

    if(bind(sockfd, (const sockaddr*) &server_addr, sizeof(sockaddr_in6)) < 0)
        SHUTDOWN_M("Failed to bind to socket", close_socket);

    epollfd = epoll_create(1);
    if(epollfd < 0)
        SHUTDOWN_M("Failed to create an epoll handle", close_socket);

    epoll_event epev[1];
    epev[0].events = EPOLLIN | EPOLLPRI;
    epev[0].data.fd = sockfd;
    if(epoll_ctl(epollfd, EPOLL_CTL_ADD, sockfd, epev) < 0)
        SHUTDOWN_M("Failed to register the socket to the epoll handle", close_epoll);

    //Now that we're running, stop being terminate-on-failure-happy
    syslog(LOG_INFO, "<info> Running.");

    /****
    ** Listen loop
    */

    int eventcount;
    while((eventcount = epoll_wait(epollfd, epev, 1, -1)) >= 0){
        if(eventcount == 0)
            continue;
        else if(epev[0].events & EPOLLERR)
            SHUTDOWN_M("Socket closed unexpectedly with EPOLLERR", close_epoll);
        else if(epev[0].events & EPOLLHUP)
            SHUTDOWN_M("Socket closed unexpectedly with EPOLLHUP", close_epoll);
        else if(!(epev[0].events & EPOLLIN) && !(epev[0].events & EPOLLPRI))
            continue;

        uint8_t packetdata[1500];
        socklen_t addrlen = sizeof(sockaddr_in6);
        ssize_t packetsize = recvfrom(epev[0].data.fd, packetdata, 1500, 0, (sockaddr*) &client_addr, &addrlen);
        if(packetsize < 0){
            if(errno == EINTR || errno == ECONNRESET || errno == ENOTCONN || errno == ETIMEDOUT ||
                errno == EAGAIN || errno == EWOULDBLOCK)
                continue;

            SHUTDOWN_M("Socket closed unexpectedly on call to recvfrom", close_epoll);
        }

        //Pass the packet down (even zero-length packets might be meaningful in the protocol)
        //...
    }

    /****
    ** Shutdown
    */

    }
    close_epoll:  close(epollfd);
    close_socket: close(sockfd);
    close_msg:    syslog(LOG_INFO, "<info> Shut down gracefully.");
    closelog();

    return exitval;
}
