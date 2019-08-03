#ifndef PEER_H
#define PEER_H

#include "../header/p2p.h"
#include "../header/list.h"
#include "../header/net.h"
#include "../header/hash.h"
#include "../header/fullio.h"
#include "../header/utils.h"
#include "../header/sockwrap.h"
#include "../header/blockchain.h"

#include "connected_ent.h"
#include "peer_pkg.h"

#define PEER_USAGE "Usage: ./peer \n \
      \t[-a CENTRAL_SERVER_IP_ADDRESS || 127.0.0.1]\n \
      \t[-b CENTRAL_SERVER_PORT || 7777]\n \
      \t[-p PASSWORD || \"VitCoin\"]\n \
      \t-s SERVICE_PEER_PORT\n\
      \t[-t TEST_MODE: create a fake multi-tail blockchain]\n\
      \t-h HELP: display this usage\n"

#define DEFAULT_SERVER_ADD "127.0.0.1"
#define DEFAULT_SERVER_PORT 7777
#define DEFAULT_PSW "VitCoin"

//--------------------------------------------------------------------GLOBAL VAR
struct sockaddr_in server_add;
struct s_net_ent my_service_ent, server;
bool test_mode;
hash_t hash_psw // password for for access to network

// thread & their Sincro stuff
pthread_attr_t *attr;

RW_sincro sincro_conn_peer;
RW_sincro sincro_conn_wallet;
RW_sincro sincro_block_chain;
sem_t *printer_sem;

//---------------------------------------------------MUTEX ACCESS
// Blockchain stuff
Bchain block_chain;
int next_seq;
pthread_t reserved_tid;
bool flag_tid;

// connections stuff
List conn_peer;
List conn_wallet;
int max_fd;
int fd_open[FD_SETSIZE];

// signal stuff
bool exit_flag, refresh_flag;

// ----------------------------------------------------------------------NETWORK
/* request to hook to a peer. NULL is return if failled. Else it returns
   the Connected_ent to add to conn_peer list */
Connected_ent hook_to_peer(struct s_net_ent peer_ent, short download);

bool hook_network();

// ----------------------------------------------------REQUEST SERVED BY THREADS
void *peer_state_printer();
void *hook_p2p(void *arg);

// ----------------------------------------------------------------------UTILITY
void read_cli_param(int argc, char **argv);
void init_global_var();
void destroy_global_var();
void sig_handler(int n);





#endif
