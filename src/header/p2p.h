#ifndef P2P_H
#define P2P_H

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "utils.h"
#include "fullio.h"
#include "hash.h"
#include "net.h"

#define DEFAULT_SERVER_ADD "127.0.0.1"
#define DEFAULT_SERVER_PORT 7777
#define DEFAULT_PSW "VitCoin"

// MACRO FOR REQUESTS MADE AT SERVER
enum server_macro{
  HOOK_PEER,
  HOOK_WALLET,
  CLOSE_PEER
};

// MACRO FOR REQUESTS MADE AT PEER
enum peer_macro{
  HOOK_P2P,
  HOOK_W2P,
  W_BALANCE,
  W_TRANSACTION,
  P_BLOCK,
  P_RECREATED_BLOCK,
  SOCKET_ERROR
};

//----------------------------------------------------------NET UTILITY FUNCTION
// function for rapid initialization of an address
void fill_address(
    struct sockaddr_in*   socket_address,
    sa_family_t           family,
    char*                 ip_address,
    unsigned short        port
  );


/*
  authorization protocol to be able to send requests to the server.
  In case of successfull connection it returns the server fd, -1 otherwise
*/
int server_auth(struct s_net_ent server, hash_t h_psw);


//----------------------------------------------------------NET TRAFFIC FUNCTION
int send_char(int fd, char n);
int recv_char(int fd, char *n);

int send_short(int fd, short n);
int recv_short(int fd, short *n);

int send_int(int fd, int n);
int recv_int(int fd, int *n);

#endif
