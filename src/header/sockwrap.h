#ifndef SOCKWRAP_H
#define SOCKWRAP_H

#define BACKLOG 1024

#include <sys/errno.h>
#include <sys/types.h>
#include <sys/socket.h>


int Socket(int domain, int type, int protocol);

int Connect(int sockfd, const struct sockaddr *addr);

int Bind(int sockfd, const struct sockaddr *addr);

int Listen(int sockfd, int backlog);

int Accept(int listfd, struct sockaddr *cli_add);

#endif
