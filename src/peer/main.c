#include "peer.h"

int main(int argc, char **argv)
{
  // set seed for use a RNG
  srand(time(NULL));

  // read and set params passed by CLI
  read_cli_param(argc, argv);

  // initialize gloabal var and destructor
  init_global_var();
  atexit(destroy_global_var);

  /*SETUP FOR PEER AND WALLET REQUEST******************************************/
  //listen descriptor stuff
  struct sockaddr_in my_server_addr;
  int list_fd;
  int optval = 1;

  list_fd = Socket(AF_INET, SOCK_STREAM, 0);
  setsockopt(list_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int));
  fill_address(&my_server_addr, AF_INET, NULL, my_service_ent.port);

  Bind(list_fd, (struct sockaddr*)&my_server_addr);
  Listen(list_fd, BACKLOG);

  fd_open[list_fd] = 1;
  max_fd = list_fd;

  /*TRY TO HOOK TO NETWORK*****************************************************/
  bool connected = hook_network();

  if(!connected)
  {
    perror("\nimpossible to hook up to the network\n");
    exit(EXIT_FAILURE);
  }


  /*START SERVER-SIDE**********************************************************/

  /*select stuff*/
  fd_set rset;
  int n_ready;
  int i_fd;
  struct timespec ts;
  ts.tv_nsec = 0;

  /*signal stuff*/
  signal(SIGINT, sig_handler);
  signal(SIGUSR1, sig_handler);

  sigset_t block_sigmask, orig_sigmask;

  // setup sigmask
  sigemptyset(&block_sigmask);
  sigaddset(&block_sigmask, SIGINT);
  sigaddset(&block_sigmask, SIGTERM);
  sigaddset(&block_sigmask, SIGUSR1);

  // get the original mask
  sigprocmask(SIG_SETMASK, NULL, &orig_sigmask);

  /*thread management stuff*/
  short request;
  int *arg_fd;
  pthread_t tid;

  /*create thread for print connected peer, wallet and blockchain*/
  pthread_create( &tid, attr, peer_state_printer, NULL);

  while(1)
  {
    // blocks reception of the signals indicated by the mask to analyze them
    sigprocmask(SIG_BLOCK, &block_sigmask, NULL);

    /*Descriptor Update*/
    FD_ZERO(&rset);
    FD_SET(list_fd, &rset);

    rw_sincro_entry_section(sincro_conn_peer, READER);
    rw_sincro_entry_section(sincro_conn_wallet, READER);

     for(i_fd=0; i_fd<=max_fd; i_fd++)
       if(fd_open[i_fd])
         FD_SET(i_fd, &rset);

    rw_sincro_exit_section(sincro_conn_wallet, READER);
    rw_sincro_exit_section(sincro_conn_peer, READER);


    //wait for n_ready
    do
    {
      // reset timeout
      ts.tv_sec = 2;

      n_ready = pselect(max_fd+1, &rset , NULL , NULL , &ts, &orig_sigmask);
    }while(n_ready == 0); // zero if the timeout expires before anything interesting happens.

    if(n_ready < 0)
    {
      if(errno == EINTR)
      {
        if(exit_flag)
        {
          printf("\n\nCLOSING DETECTED!!! Bye...\n\n");
          exit(EXIT_SUCCESS);
        }
        else // thread has finished serving a request so send a SIGUSR1 and set refresh_flag
        {
          refresh_flag = false;
          sigprocmask(SIG_UNBLOCK, &block_sigmask, NULL);
          continue;
        }
      }
      else
      {
        perror("\nselect() error\n");
        exit(EXIT_FAILURE);
      }
    }

    sigprocmask(SIG_UNBLOCK, &block_sigmask, NULL);

    /*CHECK NEW CONNECTION*************************************************/
    if( FD_ISSET(list_fd, &rset) )
    {
      printf("\n\tNew connection\n");
      int conn_fd, keepalive = 1;

      // serve one of ready fd
      n_ready--;
      conn_fd = Accept(list_fd, NULL);
      setsockopt(conn_fd, SOL_SOCKET, SO_KEEPALIVE, &keepalive, sizeof(keepalive));
      //update fd_open and max_fd
      fd_open[conn_fd] = 1;
      max_fd = (conn_fd > max_fd) ? conn_fd : max_fd;
    }

    /*CHECK REMAINING n_ready FD FOR SERVE THE REQUESTS********************/
    i_fd = list_fd;

    while(n_ready)
    {
      i_fd++;

      if(!fd_open[i_fd])
       continue;

      if(FD_ISSET(i_fd, &rset))
      {
        // serve one of ready fd
        n_ready--;
        int not_received;

        /*recv_short returns a value different from 0 both in case of error, and
          in case of closure of the socket on the other side of the connection*/
        not_received = recv_short(i_fd, &request);

        /*CHECK ERROR ON SOCKET******************************************/
        if(not_received)
          request = SOCKET_ERROR;


        /*LAUNCH OF THREAD TO SERVE THE REQUEST*******************************/
        // set the descriptor to be passed to the thread
        arg_fd = (int*)obj_malloc( sizeof(int) );
        *arg_fd = i_fd;

        // no longer check the descriptor, because the thread takes care of it
        fd_open[i_fd] = 0;

        // serve the request based on the macro
        switch(request)
        {
          case HOOK_P2P:
            printf("\nSTART HOOK_P2P REQUEST\n");
            pthread_create( &tid, attr, hook_p2p, (void*) arg_fd);
            printf("\nthread: %d, created to serve HOOK_P2P request.\n", (int)tid);
            break;
          /*
          // case HOOK_W2P:
          //   printf("\nSTART HOOK_W2P REQUEST\n");
          //   pthread_create( &tid, attr, hook_w2p, (void*) arg_fd);
          //   printf("\nthread: %d, created to serve HOOK_W2P request.\n", (int)tid);
          //   break;
          //
          // case W_BALANCE:
          //   printf("\nSTART W_BALANCE REQUEST\n");
          //   pthread_create( &tid, attr, w_balance, (void*) arg_fd);
          //   printf("\nthread: %d, created to serve W_BALANCE request.\n", (int)tid);
          //   break;
          //
          // case W_TRANSACTION:
          //   printf("\nSTART W_TRANSACTION REQUEST\n");
          //   pthread_create( &tid, attr, w_transaction, (void*) arg_fd);
          //   printf("\nthread: %d, created to serve W_TRANSACTION request.\n", (int)tid);
          //   break;

          // case P_BLOCK:
          //   printf("\nSTART P_BLOCK REQUEST\n");
          //   pthread_create( &tid, attr, p_block, (void*) arg_fd);
          //   printf("\nthread: %d, created to serve P_BLOCK request.\n", (int)tid);
          //   break;

          // case SOCKET_ERROR:
          //   pthread_create(&tid, attr, socket_error, (void*) arg_fd);
          //   break;

          // TODO: decidi se tenere
          // case SHUTDOWN_NET: // ricevuto dal server
          //   printf("\nShutting down network...\n");
          //   exit(EXIT_SUCCESS);
          */
          default:
            printf("\nMACRO NOT VALID! Request Aborted.\n");
        } // switch case
      } // if FD_ISSET()
    } // while n_ready
  } // while(1)

  exit(EXIT_SUCCESS);
}
