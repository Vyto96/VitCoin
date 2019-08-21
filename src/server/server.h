#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>


#include "../header/p2p.h"
#include "../header/list.h"
#include "../header/net.h"
#include "../header/hash.h"
#include "../header/fullio.h"
#include "../header/utils.h"
#include "../header/sockwrap.h"


#define SERVER_USAGE "Usage: ./server \n \
    \t[-l LISTEN_PORT || default = 7777]\n \
    \t[-p PASSWORD || default = \"VitCoin\"]\n \
    \t-h HELP: display this usage\n"

#define MIN_PEERS_PERC 50/100

//--------------------------------------------------------------------GLOBAL VAR
List network; // list of peers service Net_ent
// struct s_net_ent client_ent; // TODO: CONTROLLA SE SERVE
hash_t hash_psw; // password for access to network
int client_fd; // used from every functions

// REQUEST FUNCTIONS
void hook_peer();
void hook_wallet();
void close_peer();

// UTILITY
void read_cli_param(int argc, char **argv, unsigned short* port);
void print_status();
#endif
