#include "server.h"

int main(int argc, char **argv)
{
  // set seed for use of RNG
  srand(time(NULL));

  struct sockaddr_in server_add;
  int list_fd;
  unsigned short listen_port;

  //---------------------------- ---------------------------------INIT GLOBAL VAR
  read_cli_param(argc, argv, &listen_port);
  network = create_list();

  //----------------------------------------------------------------SETUP SERVER
  fill_address(&server_add, AF_INET, NULL, listen_port);

  // listening on tcp socket type
  list_fd = Socket(AF_INET, SOCK_STREAM, 0);

  // set reuse address option on socket
  int optval = 1;
  setsockopt(list_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval) );

  // Bind of address
  Bind(list_fd, (struct sockaddr *) &server_add);

  // set the backlog for listen queue
  Listen(list_fd, BACKLOG);

  short request;
  hash_t client_psw = (hash_t)obj_malloc(SHA256_DIGEST_LENGTH);

  // START SERVER
  while(1)
  {
    print_status();
    client_fd = Accept(list_fd, NULL);

    // read network password
    full_read(client_fd, client_psw, SHA256_DIGEST_LENGTH);

    // check it
    if( !hash_equal(hash_psw, client_psw) ) // not authenticated
    {
      //send 0, refusing
      send_short(client_fd, 0);
      printf("Rejecting request, password is NOT correct.\n");
    }
    else //authenticated
    {
      // sending 1, accepting
      send_short(client_fd, 1);

      printf("\nSHA256 verified, serving request of type: ");

      // waiting for request (MACROs)
      recv_short(client_fd, &request);

      switch (request)
      {
        case HOOK_PEER:
          printf("HOOK_PEER\n");
          hook_peer();
          break;

        case HOOK_WALLET:
          printf("HOOK_WALLET\n");
          hook_wallet();
          break;

        case CLOSE_PEER:
          printf("CLOSE_PEER\n");
          close_peer();
          break;

        default:
          printf("MACRO not correct\n");
          printf("Aborting.\n");
          break;
      }
    }
    close(client_fd);
  }

  exit(EXIT_SUCCESS);
}
