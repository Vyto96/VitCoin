#include "peer_pkg.h"

// ---------------------------------------------------------------------HOOK PKG
int send_hook_pkg(int fd, Hook_pkg hp)
{
  if( full_write(fd, hp, HOOK_PKG_SIZE) != 0 )
  {
    perror("send_hook_pkg()");
    return -1;
  }
  return 0;
}


int recv_hook_pkg(int fd, Hook_pkg hp)
{
  if(full_read(fd, hp, HOOK_PKG_SIZE) != 0)
  {
    perror("rec_hook_pkg()");
    return -1;
  }
  return 0;
}


// --------------------------------------------------------------------BLOCK PKG
int send_block_pkg(int fd, Block bl)
{
  struct s_block_pkg bp;

  // if genesis block
  if(bl->info != NULL)
    bp.transaction = *(Trns)bl->info ;

  // copy all block data in block package (except pointer)
  hashcpy(bp.id, bl->id);
  hashcpy(bp.prev_id, bl->prev_id);
  bp.rand_sec = bl->rand_sec;
  bp.seq = bl->seq;
  bp.creator = bl->creator;

  if( full_write(fd, &bp, BLOCK_PKG_SIZE) != 0 )
  {
    perror("send_block_pkg()");
    return -1;
  }
  return 0;
}


int recv_block_pkg(int fd, Block bl)
{
  Block_pkg bp = (Block_pkg)obj_malloc(BLOCK_PKG_SIZE);

  if(full_read(fd, bp, BLOCK_PKG_SIZE) != 0)
  {
    perror("recv_block_pkg()");
    return -1;
  }

  Trns transaction = (Trns)obj_malloc(TRNS_SIZE);
  *transaction = bp->transaction;

  // copy all block package data in block
  bl->info = (void*)transaction;
  hashcpy(bl->id, bp->id);
  hashcpy(bl->prev_id, bp->prev_id);
  bl->rand_sec = bp->rand_sec;
  bl->seq = bp->seq;
  bl->creator = bp->creator;

  bl->next = NULL;
  bl->side = NULL;

  free(bp);
  return 0;
}


// ----------------------------------------------------------------RECREATED PKG
int send_recreated_pkg(int fd, Recreated_pkg rb)
{
  if( full_write(fd, rb->old_id, SHA256_DIGEST_LENGTH) != 0 )
  {
    perror("send_recreated_pkg()-->full_write(old_id)");
    return -1;
  }

  if( send_block_pkg(fd, rb->rec_bl) == -1 )
  {
    perror("send_recreated_pkg()-->send_block_pkg()");
    return -1;
  }

  return 0;
}


int recv_recreated_pkg(int fd, Recreated_pkg rb)
{
  if( full_read(fd, rb->old_id, SHA256_DIGEST_LENGTH) != 0 )
  {
    perror("recv_recreated_pkg()-->full_read(old_id)");
    return -1;
  }

  if( recv_block_pkg(fd, rb->rec_bl) == -1)
  {
    perror("recv_recreated_pkg()-->recv_block_pkg()");
    return -1;
  }

  return 0;
}
