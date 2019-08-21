#include "wallet.h"

// ----------------------------------------------------------------------NETWORK

bool hook_to_peer()
{
  int server_fd;

  // try to access to network with hashed password
  server_fd = server_auth(server_ent, hash_psw);

  // checks if the authorization protocol has failed
  if (server_fd == -1)
    return false;

  // send the request to receive the peer to whom to hook
  send_short(server_fd, HOOK_WALLET);

  // read the number of peer in network
  int n_peer;
  recv_int(server_fd, &n_peer);

  // check if there isn't at least one peer in the network
  if(n_peer == 0)
  {
    printf("\nNewtwork isn't UP, so there are no peers to hook!\n");
    return false;
  }

  // read the Net ent of peer
  recv_net_ent(server_fd, &peer_ent);

  // close connection with server
  close(server_fd);


  // START HOOKING TO PEER
  struct sockaddr_in peer_addr;
  fill_address(&peer_addr, AF_INET, peer_ent.addr, peer_ent.port);
  peer_fd = Socket(AF_INET, SOCK_STREAM, 0);
  Connect(peer_fd, (struct sockaddr *)&peer_addr);

  printf("\ntry to connect to the following peer received from the server:");
  visit_net_ent(&peer_ent);

  short response;

  // send request for hook
  send_short(peer_fd, HOOK_W2P);

  // read the response
  recv_short(peer_fd, &response);

  // check response
  if(!response)
  {
    fprintf(stderr, "Peer refused my connection\n");
    return false;
  }
  else
  {
    getsock_net_ent(peer_fd, &my_ent);
    printf("Correctly hooked\n");
  }


  return true;
}


bool request_transaction(Trns trns)
{
  short confirm = 0;

  // send MACRO FOR WTRANSACTION REQUEST
  send_short(peer_fd, W_TRANSACTION);

  // send transaction
  send_trns(peer_fd, trns);

  printf("\nWaiting for confirmation of receipt\n");
  recv_short(peer_fd, &confirm);

  if(!confirm)
  {
    fprintf(stderr, "Transaction not validate, aborting operation\n");
    printf("\nReturning to main menu\n");
    free(trns);
    return false;
  }

  printf("\nConfirmation of receipt, received. Waiting for mining of block that concern the transaction\n");
  recv_short(peer_fd, &confirm);

  if(confirm == 0)
  {
    fprintf(stderr, "Transaction not validate, aborting operation\n");
    return false;
  }


  printf("Transaction complete!\n");
  return true;

}


//---------------------------------------------------------WALLET MENU FUNCTIONS
void request_balance(bool print_balance)
{
  int n_trns = 0;

  // send macro for get Wallet balance
  send_short(peer_fd, W_BALANCE);
  printf("\nStart wallet balance request.\n");

  // read the number of transactions that concern wallet
  recv_int(peer_fd, &n_trns);


  if(n_trns == 0)
  {
    wallet_amount = 0.0;
    printf("\nYou have not yet made/received transactions\n");
    return;
  }

  printf("\nDownload start of %d blocks for the calculation of the balance.\n", n_trns);

  List wallet_trns = create_list();
  Trns tmp_trns;

  // DOWNLOAD ALL WALLET'S TRANSACTION
  for(int i = 0; i < n_trns; i++)
  {
    tmp_trns = (Trns)obj_malloc(TRNS_SIZE);
    recv_trns(peer_fd, tmp_trns);
    add_to_list(wallet_trns, tmp_trns);
  }

  printf("Download of transactions end.");

  printf("\nCalculating refreshed balance...\n\n");
  double tmp_amount = 0.0;

  if(print_balance)
  {
    for(int i = 0; i < wallet_trns->size; i++)
    {
      tmp_trns = search_by_index(wallet_trns, i);

      // check if tmp_trns is a negative trns
      if( compare_net_ent(&my_ent, &(tmp_trns->src)) &&
          !compare_net_ent(&my_ent, &(tmp_trns->dst)) )
      {
        tmp_amount -= tmp_trns->amount;
        printf("\t-%5.2f\n", tmp_trns->amount);
      }
      else // else is a positive trns
      {
        tmp_amount += tmp_trns->amount;
        printf("\t+%5.2f\n", tmp_trns->amount);
      }
    }
  }
  else // calculate without print balance trns
  {
    for(int i = 0; i < wallet_trns->size; i++)
    {
      tmp_trns = search_by_index(wallet_trns, i);

      if( compare_net_ent(&my_ent, &(tmp_trns->src)) && !compare_net_ent(&my_ent, &(tmp_trns->dst)) )
        tmp_amount -= tmp_trns->amount;
      else // else is a positive trns
        tmp_amount += tmp_trns->amount;
    }
  }

  printf("\nRefreshed balance correctly calculated.");
  printf("\nReturning to main menu\n\n");


  wallet_amount = tmp_amount;
  empty_list(wallet_trns);
  free(wallet_trns);
  free(tmp_trns);
}


void add_coin()
{
  float vtc_to_buy = 0.0;
  char buffer[BUFFLEN];

  printf("\nHow many ViTCoin do you want to \'mine\'?\n");
  fgets(buffer, BUFFLEN, stdin);
  sscanf(buffer, "%f", &vtc_to_buy);

  Trns trns = create_transaction(gen_time_stamp(), vtc_to_buy, my_ent, my_ent);

  if( request_transaction(trns) )
    wallet_amount = wallet_amount + vtc_to_buy;

  free(trns);

  printf("\nReturning to main menu\n");
}


void send_coin()
{
  // check if there is money in wallet
  if(wallet_amount == 0)
  {
    printf("\nWallet balance is 0.\nIn order to make a transaction you must first buy ViTCoins!!\n");
    return;
  }

  // var used for read the user's input
  char  buffer[BUFFLEN],
        choice = 'n';

  // var used for construction of transaction
  Trns trns;
  float vtc_to_send = 0.0;
  struct s_net_ent dst;

  while (choice != 'y')
  {
    // read the IPv4 address
    printf("\nInsert a valid IPv4 address: ");
    fgets(buffer, BUFFLEN, stdin);
    buffer[strcspn(buffer, "\n")] = '\0';
    strncpy(dst.addr, buffer, ADDRESS_LEN);

    // read the port
    printf("Insert a valid port address: ");
    fgets(buffer, BUFFLEN, stdin);
    sscanf(buffer, " %hud", &dst.port);

    bool retry = true;
    while(retry) // check if there is enough money in the wallet
    {
      printf("How many ViTCoin do you want to send?\n");
      fgets(buffer, BUFFLEN, stdin);
      sscanf(buffer, "%f", &vtc_to_send);

      if(vtc_to_send > wallet_amount)
        fprintf(stderr, "Sending more money than your actual fund...\nRETRY\n");
      else
        retry = false;
    }

    printf("are those info correct? [y]\n");
    fgets(buffer, BUFFLEN, stdin);
    sscanf(buffer, " %c", &choice);
  }


  printf("Are you sure want to make this payment? [y]\n");
  fgets(buffer, BUFFLEN, stdin);
  sscanf(buffer, " %c", &choice);

  if(choice == 'y')
  {
    char *timestamp = gen_time_stamp();
    trns = create_transaction(timestamp, vtc_to_send, my_ent, dst);

    if( request_transaction(trns) )
      wallet_amount = wallet_amount - vtc_to_send;

    free(trns);
  }
  else
    printf("\ntransaction cancelled!\n");

  printf("\nReturning to main menu\n");
}
// ----------------------------------------------------------------------UTILITY
void read_cli_param(int argc, char **argv)
{
  /*var used for argv parsing*/
  int opt;
  int flags[4] = { 0 };

  while ((opt = getopt(argc, argv, "a:b:p:h")) != -1)
  {
    switch(opt)
    {
      case 'a': // server Address
        strncpy(server_ent.addr, optarg, ADDRESS_LEN);
        flags[0] = 1;
        break;

      case 'b': // server Port
        server_ent.port = (unsigned short)atoi(optarg);
        flags[1] = 1;
        break;

      case 'p': // Password to encrypt with Sha256
        hash_psw = calculate_hash(optarg);
        flags[2] = 1;
        break;

      case 'h': // Help
      default:
        usage(WALLET_USAGE);
        break;
    }
  }

  // check if to use default server ip
  if(!flags[0])
  {
    printf("\nUse the default address to contact the server.");
    strcpy(server_ent.addr, DEFAULT_SERVER_ADD);
  }
  // check if to use default server port
  if(!flags[1])
  {
    printf("\nUse the default port to contact the server.");
    server_ent.port = DEFAULT_SERVER_PORT;
  }
  // check if to use default network password
  if(!flags[2])
  {
    printf("\nUsing default test password for access to network.\n");
    hash_psw = calculate_hash(DEFAULT_PSW);
  }

}


void print_menu()
{
  printf("\n\nWALLET INFO: ");
  visit_net_ent(&my_ent);
  printf("\nWALLET BALANCE: %5.2f \n", wallet_amount);

  printf("0) Exit...\n");
  printf("1) Refresh wallet balance.\n");
  printf("2) Buy more VitCoin\n" );
  printf("3) Make an exchange\n");
  printf("\nChose an operation and press ENTER to activate-->\n");
}
