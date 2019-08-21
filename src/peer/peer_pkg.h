#ifndef PEER_PKG
#define PEER_PKG

#include "../header/net.h"
#include "../header/vitc.h"
#include "../header/blockchain.h"
#include "../header/hash.h"

// ---------------------------------------------------------------------HOOK PKG
// package used for hook procedure
struct s_hook_pkg
{
  struct s_net_ent ent;
  short download_flag;
};
typedef struct s_hook_pkg *Hook_pkg;
#define HOOK_PKG_SIZE sizeof(struct s_hook_pkg)

int send_hook_pkg(int fd, Hook_pkg hp);
int recv_hook_pkg(int fd, Hook_pkg hp);

// --------------------------------------------------------------------BLOCK PKG
// package used for blocks exchanges
struct s_block_pkg
{
  struct s_trns transaction;
  unsigned char id[ SHA256_DIGEST_LENGTH ];
  unsigned char prev_id[ SHA256_DIGEST_LENGTH ];
  short rand_sec;
  int seq;
  struct s_net_ent creator;
};
typedef struct s_block_pkg *Block_pkg;
#define BLOCK_PKG_SIZE sizeof(struct s_block_pkg)

int send_block_pkg(int fd, Block bl);
int recv_block_pkg(int fd, Block bl);

// ----------------------------------------------------------------RECREATED PKG
// package used for recreated blocks exchanges
struct s_recreated_pkg
{
  Block rec_bl; // recreated block
  unsigned char old_id[ SHA256_DIGEST_LENGTH ];
};
typedef struct s_recreated_pkg *Recreated_pkg;
#define RECREATED_PKG_SIZE sizeof(struct s_recreated_pkg)

int send_recreated_pkg(int fd, Recreated_pkg rb);
int recv_recreated_pkg(int fd, Recreated_pkg rb);


#endif
