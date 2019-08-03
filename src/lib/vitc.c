#include "../header/vitc.h"

Trns create_transaction(char* ts, float am, struct s_net_ent src, struct s_net_ent dst)
{
  Trns t = (Trns)obj_malloc(TRNS_SIZE);
  strncpy(t->time_stamp, ts, TIME_STAMP_LEN);
  t->src = src;
  t->amount = am;
  t->dst = dst;
  return t;
}

char* describe_trns(Trns t)
{
  char *desc = (char*)obj_malloc(sizeof(char) * TRNS_LEN);

  sprintf(desc, "%s__[%s:%d]->[%s:%d]=%5.2f", \
    t->time_stamp, t->src.addr, t->src.port, \
    t->dst.addr, t->dst.port,t->amount);
    return desc;
}

void visit_trns(void *args)
{
  Trns t = (Trns)args;

  printf("%s\n", t->time_stamp);
  printf("mittente: [%s:%d]\n", t->src.addr, t->src.port);
  printf("destinatario: [%s:%d]\n", t->dst.addr, t->dst.port);
  printf("\n-->amount: %5.2f ѶCoin \n\n", t->amount);
}


void printViTCmsg(char *msg)
{
  printf("\nѶ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ\n");
  printf("\n%s\n", msg);
  printf("\nѶ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ\n");
}


int send_trns(int fd, Trns trns)
{
  if(full_write(fd, trns, TRNS_SIZE) != 0)
  {
    perror("send_trns:");
    return -1;
  }
  return 0;
}


int recv_trns(int fd, Trns trns)
{
  if(full_read(fd, trns, TRNS_SIZE) != 0)
  {
    perror("recv_trns:");
    return -1;
  }
  return 0;
}

bool is_src_or_dst_equal(void *x, void *y)
{
  Trns a = (Trns) x;
  Trns b = (Trns) y;

  if(compare_net_ent(&(a->src), &(b->src))
  || compare_net_ent(&(a->src), &(b->dst))
  || compare_net_ent(&(a->dst), &(b->src))
  || compare_net_ent(&(a->dst), &(b->dst))
  )
    return true;
  return false;
}


bool compare_trns(void *x, void *y)
{
  Trns a = (Trns) x;
  Trns b = (Trns) y;

  if( compare_net_ent( &(a->src), &(b->src) )
  && compare_net_ent( &(a->dst), &(b->dst) )
  && a->amount == b->amount
  && strncmp(a->time_stamp, b->time_stamp, TIME_STAMP_LEN) == 0
  )
    return true;
  return false;

}
