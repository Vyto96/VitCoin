#ifndef PEER_PKG
#define PEER_PKG

#include "../header/net.h"
#include "../header/vitc.h"
#include "../header/blockchain.h"
#include "../header/hash.h"

// package used for hook procedure
struct s_hook_pkg
{
  struct s_net_ent ent;
  short donwload_flag;
};
typedef struct s_hook_pkg *Hook_pkg;
#define HOOK_PKG_SIZE sizeof(struct s_hook_pkg)

int send_hook_pkg(int fd, Hook_pkg hp);
int recv_hook_pkg(int fd, Hook_pkg hp);

// package used for blocks exchanges
struct s_block_pkg
{
  struct s_trns transaction;
  unsigned char id[ SHA256_DIGEST_LENGTH ];
  unsigned char prev_id[ SHA256_DIGEST_LENGTH ];
  short rand_sec;
  int seq;
  struct s_net_ent creator;
  short confirmed;
};
typedef struct s_block_pkg *Block_pkg;
#define BLOCK_PKG_SIZE sizeof(struct s_block_pkg)

int send_block_pkg(int fd, Block bl);
int recv_block_pkg(int fd, Block bl);


/* pacchetto per la ricreazione di un blocco:
  - blocco nuovo ricreato
  - id vecchio
*/
#endif
