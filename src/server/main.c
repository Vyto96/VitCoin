#include "server.h"

int main(int argc, char **argv)
{
  srand(time(NULL));

  //------------------------------------------------------------------------VAR
  struct sockaddr_in server_add;
  int list_fd;
  unsigned short listen_port;

  //-------------------------------------------------------------INIT GLOBAL VAR
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

  short request, authorizzation = 0;
  hash_t client_psw = (hash_t)obj_malloc(SHA256_DIGEST_LENGTH);

  while(1)
  {
    print_status();
    client_fd = Accept(list_fd, NULL);

    //------------------------------------------------------------CHECK PASSWORD
    full_read(client_fd, client_psw, SHA256_DIGEST_LENGTH);
    if( !hash_equal(hash_psw, client_psw) ) // not authenticated
    {
      authorizzation = 0;
      send_short(client_fd, authorizzation); //send 0, refusing
      printf("Rejecting request, password is NOT correct.\n");
    }
    else //authenticated
    {
      // sending 1, accepting
      authorizzation = 1;
      send_short(client_fd, authorizzation);

      // waiting for request (MACROs)
      recv_short(client_fd, &request);

      printf("\nSHA256 verified, serving request of type: ");
      switch (request)
      {
        case HOOK_PEER:
          printf("HOOK_PEER\n");
          hook_peer();
          break;
        //
        // case RE_HOOK_PEER:
        //   printf("RE_HOOK_PEER\n");
        //   re_hook_peer();
        //   break;
        //
        // case HOOK_WALLET:
        //   printf("HOOK_WALLET\n");
        //   hook_wallet();
        //   break;
        //
        // case RE_HOOK_WALLET:
        //   printf("RE_HOOK_WALLET\n");
        //   re_hook_wallet();
        //   break;

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
