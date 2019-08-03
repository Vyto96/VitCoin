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


int getsock_net_ent(int fd, Net_ent ent);
int getpeer_net_ent(int fd, Net_ent ent);

void visit_net_ent(void *arg);
bool compare_net_ent(void *x, void *y);

int send_net_ent(int fd, Net_ent n);
int recv_net_ent(int fd, Net_ent n);


#endif
