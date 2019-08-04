#include "peer_pkg.h"



int send_hook_pkg(int fd, Hook_pkg hp)
{
  if ( full_write(fd, hp, HOOK_PKG_SIZE) != 0 )
  {
    perror("send_hook_pkg()");
    return -1;
  }
  return 0;
}


int recv_hook_pkg(int fd, Hook_pkg hp)
{
  if (full_read(fd, hp, HOOK_PKG_SIZE) != 0)
  {
    perror("rec_hook_pkg()");
    return -1;
  }
  return 0;
}


int send_block_pkg(int fd, Block bl)
{
  struct s_block_pkg bp;

  bp.transaction = *(Trns) bl->info;
  hashcpy(bp.id, bl->id);
  hashcpy(bp.prev_id, bl->prev_id);
  bp.rand_sec = bl->rand_sec;
  bp.seq = bl->seq;
  bp.creator = bl->creator;
  bp.confirmed = bl->confirmed;

  if ( full_write(fd, &bp, BLOCK_PKG_SIZE) != 0 )
  {
    perror("send_block_pkg()");
    return -1;
  }
  return 0;
}

int recv_block_pkg(int fd, Block bl)
{
  Block_pkg bp = (Block_pkg)obj_malloc(BLOCK_PKG_SIZE);

  if (full_read(fd, bp, BLOCK_PKG_SIZE) != 0)
  {
    perror("recv_block_pkg()");
    return -1;
  }

  Trns transaction = (Trns)obj_malloc(TRNS_SIZE);
  *transaction = bp->transaction;

  bl->info = (void*)transaction;
  hashcpy(bl->id, bp->id);
  hashcpy(bl->prev_id, bp->prev_id);
  bl->rand_sec = bp->rand_sec;
  bl->seq = bp->seq;
  bl->creator = bp->creator;
  bl->confirmed = bp->confirmed;

  free(bp);

  bl->next = NULL;
  bl->side = NULL;

  return 0;
}
