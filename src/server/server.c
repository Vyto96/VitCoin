#include "server.h"

// ------------------------------------------------------------REQUEST FUNCTIONS
void hook_peer()
{
  short confirm;
  Net_ent peer_service_ent = (Net_ent) obj_malloc(NET_ENT_SIZE);

  // receive the service net_ent of peer
  recv_net_ent(client_fd, peer_service_ent);

  if( network->size == 0 ) // check if new peer is the first
  {
    printf("\nSend 0 to peer with service address: ");
    visit_net_ent(peer_service_ent);
    printf("because it is the first.\nwaiting for genesis block creation...\n");

    // send 0 to peer because it is the first
    send_int(client_fd, 0);

    // read the confirm of genesis block creation
    recv_short(client_fd, &confirm);

    if(confirm)
    {
      add_to_list(network, (void*)peer_service_ent);
      printf("Genesis block generated by peer with service address: ");
      visit_net_ent(peer_service_ent);
      printf("which is the only one in p2p network\n");
    }
    else
    {
      printf("Genesis block creation FAILED; peer not addedd to my list\n");
      return;
    }

  }
  else // network->size != 0, the new peer isn't the first
  {
    // minimum number of peers to which the new peer should connect
    int min_peers_number = network->size * MIN_PEERS_PERC;
    if(min_peers_number == 0)
      min_peers_number = 1;

    // send min_peers_number to new_peer
    send_int(client_fd, min_peers_number);

    // response from peer
    short confirmed_conn = 0;
    // counter of succesfull connections
    int count_succ_conn = 0;

    // net_ent to send to the peer for the connection attempt
    Net_ent to_send;

    // index of the peer chosen for the connection
    int i_chosen = rand()%network->size;

    // send min_peers_number net_ent to new peer
    for (int i = 0; i < min_peers_number; i++)
    {
      to_send = search_by_index(network, i_chosen);

      printf("Sending to peer with service: ");
      visit_net_ent(peer_service_ent);
      printf("this peer: ");
      visit_net_ent(to_send);

      // send peer
      send_net_ent(client_fd, to_send);

      // wait for confirm
      recv_short(client_fd, &confirmed_conn);

      // update counter
      if(confirmed_conn)
        count_succ_conn++;

      /*the next peer to be sent is chosen starting from the index previously
        chosen, to avoid sending equal peers*/
      i_chosen =(i_chosen + 1) % network->size;
    }

    // check if there is at least one connection
    if (count_succ_conn > 0)
      add_to_list(network, (void*)peer_service_ent); // add new peer to network
  }


}


void hook_wallet()
{
  int random_peer = 0;
  Net_ent to_send = NULL;

  // send the number of peer to wallet
  send_int(client_fd, network->size);

  // check if there is at least one peer in the network
  if(network->size)
  {
    random_peer = rand()%network->size;
    to_send = search_by_index(network, random_peer);

    printf("\nSending to Wallet the seguent Peer for connection: ");
    visit_net_ent(to_send);

    send_net_ent(client_fd, to_send);
  }
  else
    fprintf(stderr, "Newtwork isn't UP there isn't Peer to send to Wallet\n");

}



void close_peer()
{
  struct s_net_ent peer_service_ent;
  Net_ent peer_to_delete;
  
  // receive the service net_ent of the peer that is shutting down
  recv_net_ent(client_fd, &peer_service_ent);

  // extract peer from network list
  peer_to_delete = extract_from_list( network, &peer_service_ent, compare_net_ent );

  free(peer_to_delete);
}

// ----------------------------------------------------------------------UTILITY
// read the parameters passed by command line
void read_cli_param(int argc, char **argv, unsigned short* listen_port)
{
  int opt = 0;
  hash_psw = (hash_t) obj_malloc(SHA256_DIGEST_LENGTH);
  hash_psw = NULL;
  *listen_port = 0;

  while ( (opt = getopt(argc, argv, "l:p:h")) != -1)
  {
    switch(opt)
    {
      case 'p': // Pasword to encrypt with Sha256
        hash_psw = calculate_hash(optarg);
        printf("Created hash password with SHA256...\n");
        break;

      case 'l': // Listen_port
        *listen_port = atoi(optarg);
        printf("Listen on port %u...\n", *listen_port);
        break;

      case 'h': // Help
      default:
        usage(SERVER_USAGE);
        break;
    }
  }

  if(*listen_port == 0)
  {
    printf("Listen on default port %u...\n", DEFAULT_SERVER_PORT);
    *listen_port = DEFAULT_SERVER_PORT;
  }

  if(hash_psw == NULL)
  {
    printf("Created hash password with SHA256 using default test Password...\n");
    hash_psw = calculate_hash(DEFAULT_PSW);
  }
}


// print status of server and network list
void print_status()
{
  printf("\n\nѶ Ѷ Ѷ Ѷ Ѷ Ѷ Ѷ Ѷ Ѷ Ѷ Ѷ Ѷ Ѷ Ѷ Ѷ Ѷ Ѷ Ѷ Ѷ Ѷ Ѷ Ѷ Ѷ Ѷ Ѷ\n\n");
  printf("\tP2P NETWORK:\n\n");
  visit_list(network, visit_net_ent);
  printf("\n\nѶ Ѷ Ѷ Ѷ Ѷ Ѷ Ѷ Ѷ Ѷ Ѷ Ѷ Ѷ Ѷ Ѷ Ѷ Ѷ Ѷ Ѷ Ѷ Ѷ Ѷ Ѷ Ѷ Ѷ Ѷ\n\n");
  printf("\nWaiting for connections...\n");
}
