#ifndef NET_H
#define NET_H

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "utils.h"
#include "fullio.h"


#define ADDRESS_LEN 32

//-----------------------------------------------------------------NET ENT STUFF
// struct for rapid access to ip-port
struct s_net_ent{
  char addr[ADDRESS_LEN];
  unsigned short port;
};
typedef struct s_net_ent* Net_ent;
#define NET_ENT_SIZE sizeof(struct s_net_ent)


// getsockname() wrapper. It returns the local address of socket via Net_ent
int getsock_net_ent(int fd, Net_ent ent);

// getpeername() wrapper. It returns the remote address of socket via Net_ent
int getpeer_net_ent(int fd, Net_ent ent);


// function to print Net_ent (void pointer used for generic implementation)
void visit_net_ent(void *arg);

// function to compare two Net_ent (void pointer used for generic implementation)
bool compare_net_ent(void *x, void *y);


// send of a Net_ent
int send_net_ent(int fd, Net_ent n);

// recv of a Net_ent
int recv_net_ent(int fd, Net_ent n);


#endif
