#include "../header/net.h"

// getsockname() wrapper. It returns the local address of socket via Net_ent
int getsock_net_ent(int fd, Net_ent ent)
{
  struct sockaddr_in sock_addr; // local address of socket
  socklen_t sock_addr_len = sizeof(sock_addr);

  if( ( getsockname(fd, (struct sockaddr*) &sock_addr, &sock_addr_len ) ) == -1)
  {
    perror("internal getsock_net_ent error(getsockname)\n");
    return -1;
  }
  // Port of net_ent (network to host short)
  ent->port = (unsigned short) ntohs(sock_addr.sin_port) ;

  // IP address assigned by inet_ntop (network to presentation)
  if(inet_ntop(AF_INET, &sock_addr.sin_addr, ent->addr, sizeof(ent->addr)) == NULL)
  {
    perror("internal getsock_net_ent error(inet_ntop)");
    return -1;
  }
  return 0; // return 0 on success
}


// getpeername() wrapper. It returns the remote address of socket via Net_ent
int getpeer_net_ent(int fd, Net_ent ent)
{
  struct sockaddr_in sock_addr; // remote address of socket
  socklen_t sock_addr_len = sizeof(sock_addr);
  if( ( getpeername(fd, (struct sockaddr*) &sock_addr, &sock_addr_len ) ) == -1)
  {
    perror("internal getpeer_net_ent error(getpeername): ");
    return -1;
  }
  // Port of net_ent (network to host short)
  ent->port = ((unsigned short) ntohs(sock_addr.sin_port) );

  // IP address assigned by inet_ntop (network to presentation)
  if(inet_ntop(AF_INET, &sock_addr.sin_addr, ent->addr, sizeof(ent->addr)) == NULL)
  {
    perror("internal getpeer_net_ent error(inet_ntop)");
    return -1;
  }
  return 0; // return 0 on success
}


// function to print Net_ent (void pointer used for generic implementation)
void visit_net_ent(void *arg)
{
  struct s_net_ent n = * ((Net_ent)arg);
  // printf("[IP:port] = [%s:%hd]\n", n->addr, n->port);
  printf("[IP:port] = [%s:%u]\n", n.addr, n.port);
}


// function to compare two Net_ent (void pointer used for generic implementation)
bool compare_net_ent(void *x, void *y)
{
  Net_ent a = (Net_ent) x;
  Net_ent b = (Net_ent) y;
  if( !(strncmp(a->addr, b->addr, ADDRESS_LEN)) && (a->port == b->port) )
    return true;
  return false;
}


// send of a Net_ent
int send_net_ent(int fd, Net_ent n)
{
  if ( full_write(fd, n, NET_ENT_SIZE) != 0 )
  {
    perror("send_net_ent()");
    return -1;
  }
  return 0;
}


// recv of a Net_ent
int recv_net_ent(int fd, Net_ent n)
{
  if (full_read(fd, n, NET_ENT_SIZE) != 0)
  {
    perror("rec_net_ent()");
    return -1;
  }
  return 0;
}
