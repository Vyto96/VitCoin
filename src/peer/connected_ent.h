#ifndef CONNECTED_ENT_H
#define CONNECTED_ENT_H

#include "../header/net.h"
#include <bool.h>
#include <stdio.h>

struct s_connected_ent
{
  int fd;
  struct s_net_ent ent;
};
typedef struct s_connected_ent *Connected_ent;
#define CONNECTED_ENT_SIZE sizeof(struct s_connected_ent)


// I/O params void used for compatibility with list library
void visit_connected_ent(void *arg);
bool compare_connected_ent(void *x, void *y)
bool compare_connected_ent_by_fd(void *x, void *y)



#endif
