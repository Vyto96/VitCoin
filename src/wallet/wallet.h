#ifndef WALLET_H
#define WALLET_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "../header/list.h"
#include "../header/sockwrap.h"
#include "../header/net.h"
#include "../header/p2p.h"
#include "../header/utils.h"
#include "../header/hash.h"
#include "../header/fullio.h"
#include "../header/vitc.h"


#define WALLET_USAGE "Usage: ./wallet \n \
    \t[-a CENTRAL_SERVER_IP_ADDRESS || default = 127.0.0.1]\n \
    \t[-b CENTRAL_SERVER_PORT || default = 7777]\n \
    \t[-p PASSWORD || default = \"VitCoin\"]\n \
    \t-h HELP: display this usage\n"


enum wallet_operation{
  CLOSE_WALLET,
  BALANCE,
  ADD_COIN,
  SEND_COIN
};

#define USER_BUFFER 16

//--------------------------------------------------------------------GLOBAL VAR
struct s_net_ent server_ent, my_ent, peer_ent;
hash_t hash_psw;
int peer_fd;
float wallet_amount;


// hook to peer received from server
bool hook_to_peer();

// send transaction to peer and read the confirm (used by send_coin and add_coin)
bool request_transaction(Trns trns);

//---------------------------------------------------------WALLET MENU FUNCTIONS
/* get, from peer, the list of transactions that concern wallet
   and calculate the balance */
void request_balance(bool print_balance);

// make a fake transaction, sending src and dst eguals to me.
void add_coin();

// create a transaction to another wallet and send it to peer
void send_coin();

// ----------------------------------------------------------------------UTILITY
// read the parameters passed by command line
void read_cli_param(int argc, char **argv);

void print_menu();




#endif
