#include "wallet.h"


int main(int argc, char **argv)
{
  // read and set params passed by CLI
  read_cli_param(argc, argv);

  // try to hook up to the peer
  bool connected = hook_to_peer();

  if(!connected)
  {
    fprintf(stderr, "\nimpossible to hook up to the peer\n");
    exit(EXIT_FAILURE);
  }

  printf("\nGet wallet balance from peer...\n");
  request_balance(false);
  printViTCmsg("WELCOME IN YOUT WALLET!\n");

  // var used for read the user's choice
  int choice = 0;
  char line_buffer[USER_BUFFER];

  // fset used for peer_fd and stdin
  fd_set fset;
  int n_ready = 0;

  while (1)
  {
    // initialize file descriptor set
    FD_ZERO(&fset);
    FD_SET(peer_fd, &fset); // set for the peer socket
    FD_SET(STDIN_FILENO, &fset); // set for the standard input

    print_menu();
    while((n_ready = select(peer_fd +1, &fset , NULL , NULL , NULL)) < 0 && \
      errno == EINTR);

    if(n_ready < 0)
    {
      perror("select");
      exit(EXIT_FAILURE);
    }

    if(FD_ISSET(STDIN_FILENO, &fset))
    {

      if( fgets(line_buffer, USER_BUFFER, stdin) == NULL )
      {
        printf("\nfgets return NULL\n");
        continue;
      }

      // read the choice from buffer
      sscanf(line_buffer, "%d", &choice);

      switch(choice)
      {
        case CLOSE_WALLET:
          printf("\n\nGood bye!!!\n\n");
          exit(EXIT_SUCCESS);
          break;

        case BALANCE:
          request_balance(true);
          break;

        case ADD_COIN:
          add_coin();
          break;

        case SEND_COIN:
          send_coin();
          break;

        default:
          fprintf(stderr, "Choice is not processed, retry\n");
          break;
      }
    }

    // check if peer send a transaction or if it is crushed
    if(FD_ISSET(peer_fd, &fset))
    {
      struct s_trns trns;

      int error = recv_trns(peer_fd, &trns);
      if(error != 0)
      {
        fprintf(stderr,"Socket error: connection rejected from peer.\n");
        close(peer_fd);
        fprintf(stderr,"wallet closure...\n");
        exit(EXIT_SUCCESS);
      }
      else
      { // receive transaction and add it to yours amount
        printf("\nReceived transaction! You have received %5.2f from [%s:%d]\n", \
          trns.amount, trns.src.addr, trns.src.port);
        wallet_amount += trns.amount;
        printf("\nBalance correctly update.\n");
      }
    }
  }
  exit(EXIT_SUCCESS);
}
