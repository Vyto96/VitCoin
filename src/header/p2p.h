#ifndef P2P_H
#define P2P_H

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "utils.h"
#include "fullio.h"
#include "hash.h"

// MACRO FOR REQUESTS MADE AT SERVER
enum server_macro{
  HOOK_PEER,
  RE_HOOK_PEER,
  HOOK_WALLET,
  RE_HOOK_WALLET
};

// MACRO FOR REQUESTS MADE AT PEER
enum peer_macro{
  HOOK_P2P,
  HOOK_W2P,
  W_BALANCE,
  W_TRANSACTION,
  P_BLOCK,
  SHUTDOWN_NET,
  SOCKET_ERROR
};

//----------------------------------------------------------NET UTILITY FUNCTION
void fill_address(
    struct sockaddr_in*   socket_address,
    sa_family_t           family,
    char*                 ip_address,
    unsigned short        port
  );

bool sha256_auth(int server_fd, char *psw);


//----------------------------------------------------------NET TRAFFIC FUNCTION
int send_char(int fd, char n);
int recv_char(int fd, char *n);

int send_short(int fd, short n);
int recv_short(int fd, short *n);

int send_int(int fd, int n);
int recv_int(int fd, int *n);

#endif
