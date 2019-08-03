#include "peer.h"


// ----------------------------------------------------------------------NETWORK
Connected_ent hook_to_peer(struct s_net_ent peer_ent, short download_flag)
{

  Connected_ent peer = (Connected_ent)obj_malloc(CONNECTED_ENT_SIZE);
  // set peer net ent
  peer->ent = peer_ent;

  struct sockaddr_in peer_addr;
  fill_address(&peer_addr, AF_INET, peer->ent.addr, peer->ent.port);
  // set peer fd
  peer->fd = Socket(AF_INET, SOCK_STREAM, 0);

  // set hook pkg
  struct s_hook_pkg hp = { peer_ent, download_flag };

  printf("Try connecting to:");
  visit_net_ent( &(peer->ent) );
  Connect(peer->fd, (struct sockaddr *) &peer_addr);


  // TODO controlla ritorno della comunicazione ed aggiungi il return NULL in caso d'errore
  // send request macro for hook
  send_short(peer->fd, HOOK_P2P);

  // TODO controlla ritorno della comunicazione ed aggiungi il return NULL in caso d'errore
  // send hook pkg
  send_hook_pkg(peer->fd, &hp);


  if(download_flag)
  {
    int n_block;
    // TODO controlla ritorno della comunicazione ed aggiungi il return NULL in caso d'errore
    // read the number of block to download
    recv_int(peer->fd, &n_block);

    // "download" one block at a time
    for(int i = 0; i < n_block; i++)
    {
      Block bl = (Block)obj_malloc(BLOCK_SIZE);
      // read Block
      // TODO controlla ritorno della comunicazione ed aggiungi il return NULL in caso d'errore
      recv_block_pkg(peer->fd, bl);

      // TODO controlla ritorno di add block, exit, problema nel download
      // add to blockchain
      add_block(block_chain, bl);
    }
  }

  return peer;
}


bool hook_network()
{
  int server_fd;
  bool hooked = false;

  // initialize the sockaddr_in structure for the first and only time
  fill_address(&server_add, AF_INET, server.addr, server.port);

  server_fd = Socket(AF_INET, SOCK_STREAM, 0);

  //---------------------------------------------------connect with server
  if( Connect(server_fd, (struct sockaddr *)&server_add) != 0)
  {
    close(server_fd);
    printf("\nserver not active\n");
    return false;
  }

  // saving the IP of the network interface used to connect to the server
  struct s_net_ent tmp_net_ent;
  getsock_net_ent(server_fd, &tmp_net_ent);
  strncpy(my_service_ent.addr, tmp_net_ent.addr, ADDRESS_LEN);

  printf("PEER with service address:");
  visit_net_ent(&my_service_ent);
  printf("Attempt to hook to network:\n");

  //---------------------------------------------------authorization protocol
  if(!sha_auth(server_fd, pwd))
  { // wrong password
    fprintf(stderr, "\nwrong password, retry!\n");
    exit(EXIT_FAILURE);
  }
  printf("successful connection!\n");

  //---------------------------------------------------hooking protocol
  short n_peer;

  //send request for hook to peer-network
  send_short(server_fd, HOOK_PEER);

  // send my service ent to be added to the server list
  send_net_ent(server_fd, &my_service_ent);

  //receive the number of peers to connect to
  recv_short(server_fd, &n_peer);

  if(n_peer == 0) // if i am the first peer
  {
    char gen_id[BUFFLEN];
    sprintf(gen_id, "%d:%s", rand(), gen_time_stamp() );
    printf("I'm the first peer of p2p network.\n");
    printf("I generate the genesis block by calculating the hash of the following id [random_number:time_stamp] : \n");
    printf("%s\n", gen_id);

    init_genesis(block_chain, my_service_ent, gen_id);

    //TODO: controlla se tenere la test_mode
    // if(test_mode)
    //   test_bchain();

    // send 1 to confirm that the genesis block was created
    send_short(server_fd, 1);
    hooked = true;
  }
  else // I'm not the first peer, so n_peer tells me the number of peers
  {    // i need to try to connect to

    // counter of succesfull connections
    int count_succ_conn = 0;
    // response from peer
    short confirmed_conn = 0;

    struct s_net_ent tmp_ent;
    Connected_ent conn_peer = NULL;

    for(int i=1; i<n_peer; i++)
    {
      recv_net_ent(server_fd, &tmp_ent);

      if(count_succ_conn == 0) // if is the first peer
        conn_peer = hook_to_peer(tmp_ent, 1); // 1 for download the bchain
      else
        conn_peer = hook_to_peer(tmp_ent, 0); // 0 for only hook

      if( conn_peer != NULL )
      {
        // connection confirmed
        confirmed_conn = 1;
        count_succ_conn++;
        // update fd_open and max_fd
        fd_open[conn_peer->fd] = 1;
        max_fd = (conn_peer->fd > max_fd) ? conn_peer->fd : max_fd;
        // add to peers list
        add_to_list(conn_peer, conn_peer);
      }
      send_short(server_fd, confirmed_conn);
      confirmed_conn = 0;
    }

    if(count_succ_conn > 0)
      hooked = true;
  }

  close(server_fd);
  return hooked;
}



// ----------------------------------------------------REQUEST SERVED BY THREADS
void *peer_state_printer()
{
  while(1)
  {
    sem_wait(printer);
    print_peer_state();
  }
}


void *hook_p2p(void *arg)
{
  int fd = *(int*)arg;
  free(arg);

  Hook_pkg hp = (Hook_pkg)obj_malloc(HOOK_PKG_SIZE);

  // read the hook pkg request;
  recv_hook_pkg(fd, hp); // TODO: controlla ritorno e rilascia mutex in caso d'errore e termina thread

  if (hp->donwload_flag)
  {
    rw_sincro_entry_section(sincro_block_chain, READER);

    // read the number of blocks currently present in the blockchain
    int n_block = block_chain->size;
    int last_seq = get_last_seq(block_chain);
    // send it to the peer
    send_int(fd, &n_block); // TODO: controlla ritorno e rilascia mutex in caso d'errore e termina thread

    // send all block for every sequence number
    for(int i = 0; i < last_seq; i++)
    {
      // First Block with sequence number i
      Block fb = get_seq_block(block_chain, i);

      // send block
      send_block_pkg(fd, fb);  // TODO: controlla ritorno e rilascia mutex in caso d'errore e termina thread

      while(fb->side) // send all other block with sequence number i
      {
        fb = fb->side;
        send_block_pkg(fd, fb);  // TODO: controlla ritorno e rilascia mutex in caso d'errore e termina thread
      }
    }
    rw_sincro_exit_section(sincro_block_chain, READER);
  }

  // init new peer
  Connected_ent new_peer = (Connected_ent)obj_malloc(CONNECTED_ENT_SIZE);
  new_peer->ent = hp->ent;
  new_peer->fd = fd;

  // add new peer to list
  rw_sincro_entry_section(sincro_conn_peer, WRITER);
    fd_open[fd] = 1;
    add_to_list(conn_peer, new_peer);
  rw_sincro_exit_section(sincro_conn_peer, WRITER);

  // print new status
  sem_post(printer_sem);
  // refresh pselect
  kill(0, SIGUSR1);
  pthread_exit(NULL);
}

// ----------------------------------------------------------------------UTILITY
void read_cli_param(int argc, char **argv)
{
  /*var used for argv parsing*/
  int opt;
  int flags[4] = { 0 };

  // default value for test_mode
  test_mode = false;

  while ((opt = getopt(argc, argv, "a:b:p:s:t:h")) != -1)
  {
    switch(opt)
    {
      case 'a': // server Address
        strncpy(server.addr, optarg, LEN_ADDRESS);
        flags[0] = 1;
        break;

      case 'b': // server Port
        server.port = (unsigned short)atoi(optarg);
        flags[1] = 1;
        break;

      case 'p': // Password to encrypt with Sha256
        hash_psw = calculate_hash(optarg);
        flags[2] = 1;
        break;

      case 's': // Service peer Port
        my_service_ent.port = atoi(optarg);
        flags[3] = 1;
        break;

      case 't': // Test mode activated
        debug_mode = true;
        break;

      case 'h': // Help
      default:
        usage(PEER_USAGE);
        break;
    }
  }

  // service port for peer is necessary
  if( flags[3] == 0)
    usage(PEER_USAGE);

  // check if to use default server ip
  if(!flags[0])
    strcpy(server.addr, DEFAULT_SERVER_ADD);

  // check if to use default server port
  if(!flags[1])
    server.port = DEFAULT_SERVER_PORT;

  // check if to use default network password
  if(!flags[2])
    hash_psw = calculate_hash(DEFAULT_PSW);


}


void init_global_var()
{
  // setup pthread_attr structure for detach_state use
  attr = (pthread_attr_t*)obj_malloc( sizeof(pthread_attr_t) );
  pthread_attr_init( attr );
  pthread_attr_setdetachstate(attr, PTHREAD_CREATE_DETACHED);

  // semaphore "event" (set to 0) used to activate the printer thread
  printer = (sem_t*)obj_malloc(sizeof(sem_t));
  sem_init(printer, 0, 0);

  // flag used by threads to book for inserting a block in the bchain
  flag_tid = false;
  // flag used by signal handler function
  exit_flag = false;
  refresh_flag = false;

  // List & Blockchain constructor
  conn_peer = create_list();
  conn_wallet = create_list();
  block_chain = create_bchain();

  // Sincro structure constructor
  sincro_conn_peer = rw_sincro_create(W_PRIO);
  sincro_conn_wallet = rw_sincro_create(W_PRIO);
  sincro_block_chain = rw_sincro_create(W_PRIO);

}


void destroy_global_var()
{
  pthread_attr_destroy(attr);

  sem_destroy(printer);

  empty_list(conn_peer);
  free(conn_peer);

  empty_list(conn_wallet);
  free(conn_wallet);

  // TODO: distruttore bchain personalizzato per il campo informazione passato
  // empty_bchain(block_chain);
  free(block_chain);

  rw_sincro_destroy(sincro_conn_peer);
  rw_sincro_destroy(sincro_conn_wallet);
  rw_sincro_destroy(sincro_block_chain);
}


void sig_handler(int n)
{
  if(n == SIGINT)
    exit_flag = true;

  if(n == SIGUSR1 && refresh_flag == false)
  {
    refresh_flag = true;
  }
}
