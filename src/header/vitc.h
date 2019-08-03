#ifndef VITC_H
#define VITC_H

#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#include "net.h"
#include "utils.h"

#define TRNS_LEN 100

// transaction structure
struct s_trns
{
  float amount;
  struct s_net_ent src;
  struct s_net_ent dst;
  char time_stamp[TIME_STAMP_LEN];
};
typedef struct s_trns *Trns;
#define TRNS_SIZE sizeof(struct s_trns)

Trns create_transaction(char* ts, float am, struct s_net_ent src, struct s_net_ent dst);

char* describe_trns(Trns t);

void visit_trns(void *args);

void printViTCmsg(char *s);


int send_trns(int fd, Trns trns);

int recv_trns(int fd, Trns trns);

bool is_src_or_dst_equal(void *x, void *y);

bool compare_trns(void *x, void *y);

#endif
