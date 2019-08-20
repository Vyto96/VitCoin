#ifndef PEER_H
#define PEER_H

#include <pthread.h>
#include <signal.h>
#include <sys/select.h>

#include <time.h>
#include <sys/time.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../header/p2p.h"
#include "../header/list.h"
#include "../header/net.h"
#include "../header/hash.h"
#include "../header/fullio.h"
#include "../header/utils.h"
#include "../header/sockwrap.h"
#include "../header/blockchain.h"
#include "../header/rwsincro.h"

#include "connected_ent.h"
#include "peer_pkg.h"

#define PEER_USAGE "Usage: ./peer \n \
      \t[-a CENTRAL_SERVER_IP_ADDRESS || default = 127.0.0.1]\n \
      \t[-b CENTRAL_SERVER_PORT || default = 7777]\n \
      \t[-p PASSWORD || default = \"VitCoin\"]\n \
      \t-s SERVICE_PEER_PORT\n\
      \t[-t TEST_MODE_MINING_TIME]: create a fake multi-tail blockchain\n\
      \t-h HELP: display this usage\n"


// *****************************************************************************GLOBAL VAR
struct s_net_ent my_service_ent, server_ent;
short test_mode;
hash_t hash_psw;

// Threads & their Sincro stuff
pthread_attr_t *attr;

RW_sincro sincro_conn_peer;
RW_sincro sincro_conn_wallet;
RW_sincro sincro_block_chain;
sem_t *printer_sem, *checker_sem;

//------------------------------------------------------------------------------MUTEX ACCESS
// -------------------------------------------------------------Blockchain stuff
Bchain block_chain;

// tid of the thread that is booked to insert in the blockchain
pthread_t reserved_tid;

// if set to true it means that some thread is trying to create/insert a block
bool flag_tid;

/*if set to true it means that it is necessary to check the tail_to_cut
  list to check if it is possible to recreate an obsolete block*/
bool check_ttc;

// ------------------------------------------------------------Connections stuff
List conn_peer;
List conn_wallet;
int max_fd;
int fd_open[FD_SETSIZE];

// flags used by signal handler function
bool exit_flag, refresh_flag;

// *****************************************************************************FUNCTIONS
// ----------------------------------------------------------------------NETWORK
/* request to hook to a peer. NULL is return if failled.
   Else it returns the Connected_ent to add to conn_peer list */
Connected_ent hook_to_peer(struct s_net_ent peer_ent, short download);

// returns true if the network connection protocol succesfull
bool hook_network();


// --------------------------------------------------------------USED BY THREADS
// create  (or recreate, if bl_to_add is not null)  block in block_chain
Block sync_in_blockchain(Trns tr, Block bl_to_add);

// flooding of re/created block package
void flooding(void* pkg, short pkg_type, struct s_net_ent pkg_sender);

// function to warn a wallet in case he received a transaction
void warn_wallet(Trns t);

// used from "block_to_recreate_checker" thread
void recreate_block();

// used from "peer_state_printer" thread
void print_peer_state();

// thread refresh of pselect using a signal
void refresh_state();

/* used only in test_mode and by only first peer of network.
   Create a fake blockchain with multi-tails for testing.*/
void test_bchain();
// ----------------------------------------------------REQUEST SERVED BY THREADS
void *peer_state_printer();
void *block_to_recreate_checker();

// have a peer hooked up
void *hook_p2p(void* arg);
// have a wallet hooked up
void *hook_w2p(void* arg);

// create and "flood" the transaction received from the wallet
void *w_transaction(void* arg);
// add block received and "flood it"
void *p_block(void* arg);
// add recreated block received and "flood it"
void *p_recreated_block(void* arg);
// search for all wallet transactions in the blockchain
void *w_balance(void* arg);

// handle the error detected on a socket
void *socket_error(void *arg);

// ----------------------------------------------------------------------UTILITY
void read_cli_param(int argc, char **argv);
void init_global_var();
void destroy_global_var();
void sig_handler(int n);
void delete_from_network();

#endif
