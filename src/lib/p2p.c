#include "../header/p2p.h"

//----------------------------------------------------------NET UTILITY FUNCTION
// function for rapid initialization of an address
void fill_address(
  struct sockaddr_in* socket_address, // I/O param
  sa_family_t         family,         // Address Family (AF_INET)
  char*               ip_address,     // Dotted_decimal IP address
  unsigned short      port
)
{
  // initialize address memory area
  memset((void *) socket_address, 0, sizeof(*socket_address));

  // fill the sockaddr_in structure
  socket_address->sin_family = family;
  // port in network order
  socket_address->sin_port = htons(port);

  if(ip_address == NULL) // default connect from anywhere
    socket_address->sin_addr.s_addr = htonl ( INADDR_ANY );
  else
    // IP address assigned by inet_pton (presentation to network)
    if ( (inet_pton(family, ip_address, &(socket_address->sin_addr) ) ) <= 0)
    {
      perror("Address creation error");
      exit(EXIT_FAILURE);
    }

}


// authorizzation protocol for access to network
bool sha256_auth(int server_fd, hash_t hash_psw)
{
  short response;
  printf("sending hash code to server...\n");

  if( full_write(server_fd, hash_psw, SHA256_DIGEST_LENGTH) != 0)
    exit(EXIT_FAILURE);

  printf("Waiting for response...\n" );

  recv_short(server_fd, &response);

  if(response)
    return true;
  else
    return false;
}



//----------------------------------------------------------NET TRAFFIC FUNCTION
int send_char(int fd, char n)
{
  if( full_write(fd, &n, 1) != 0 )
  {
    perror("send_char()");
    return -1;
  }
  return 0;
}


int recv_char(int fd, char *n)
{
  if(full_read(fd, n, 1) != 0)
  {
    perror("rec_char()");
    return -1;
  }
  return 0;
}



int send_short(int fd, short n)
{
  char buf[8];
  memset((void*) buf, 0, 8);
  sprintf(buf, "%hd", n);
  if( full_write(fd, buf, 8) != 0 )
  {
    perror("send_short()");
    return -1;
  }
  return 0;
}


int recv_short(int fd, short *n)
{
  char buf[8];
  if(full_read(fd, &buf, 8) != 0)
  {
    perror("rec_short()");
    return -1;
  }
  sscanf(buf, "%hd", n);
  return 0;
}



int send_int(int fd, int n)
{
  char buf[16];
  memset((void*) buf, 0, 16);
  sprintf(buf, "%d", n);
  if( full_write(fd, buf, 16) != 0 )
  {
    perror("send_int()");
    return -1;
  }
  return 0;
}


int recv_int(int fd, int *n)
{
  char buf[16];
  if(full_read(fd, buf, 16) != 0)
  {
    perror("rec_int()");
    return -1;
  }
  sscanf(buf, "%d", n);
  return 0;
}
