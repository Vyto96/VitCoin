#include "connected_ent.h"

void visit_connected_ent(void *arg)
{
  Connected_ent p = (Connected_ent)arg;
  printf("fd=%d -->IP:port = %s:%d\n", p->fd, p->ent.addr, p->ent.port);
}


bool compare_connected_ent(void *x, void *y)
{
  Connected_ent a = (Connected_ent)x;
  Connected_ent b = (Connected_ent)y;

  return compare_net_ent( (void*) &(a->ent), (void*) &(b->ent) );
}


bool compare_connected_ent_by_fd(void *x, void *y)
{
  Connected_ent a = (Connected_ent)x;
  Connected_ent b = (Connected_ent)y;
  if(a == NULL || b == NULL)
  {
    fprintf(stderr, "\nATTENTION: Connected_ent error, NULL pointer found in compare_connected_ent_by_fd()\n" );
    return false;
  }

  return (a->fd == b->fd);
}
