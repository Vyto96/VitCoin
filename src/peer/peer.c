#include "peer.h"


// -----------------------------------------------------------------------------NETWORK
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
  struct s_hook_pkg hp = { my_service_ent, download_flag };

  printf("Try connecting to:");
  visit_net_ent( &(peer->ent) );

  // Connect to peer
  Connect(peer->fd, (struct sockaddr *) &peer_addr);

  // send request macro for hook
  if( send_short(peer->fd, HOOK_P2P) == -1) return NULL;

  // send hook pkg
  if( send_hook_pkg(peer->fd, &hp) == -1) return NULL;

  printf("\nhook_pkg sent\n");

  if(download_flag) // check if download the blockchain from peer
  {
    int n_block;

    // read the number of block to download
    if( recv_int(peer->fd, &n_block) == -1 ) return NULL;

    printf("\n start download of block number: %d.", n_block);

    // download one block at a time
    for(int i = 0; i < n_block; i++)
    {
      Block bl = (Block)obj_malloc(BLOCK_SIZE);
      // read Block
      if( recv_block_pkg(peer->fd, bl) == -1) return NULL;

      // add to blockchain
      add_block(block_chain, bl);

      printf("\nblock number %d received.", i);
    }
  }

  return peer;
}



bool hook_network()
{
  int server_fd;
  bool hooked = false;

  // try to access to network with hashed password
  server_fd = server_auth(server_ent, hash_psw);

  // checks if the authorization protocol has failed
  if (server_fd == -1)
    return false;

  // saving the IP of the network interface used to connect to the server
  struct s_net_ent tmp_net_ent;
  getsock_net_ent(server_fd, &tmp_net_ent);
  strncpy(my_service_ent.addr, tmp_net_ent.addr, ADDRESS_LEN);

  printf("PEER with service address:");
  visit_net_ent(&my_service_ent);
  printf("Attempt to hook to network:\n");

  //------------------------------------------------------------hooking protocol
  int n_peer;

  // send request for hook to peer-network
  send_short(server_fd, HOOK_PEER);

  // send my service ent to be added to the server list
  send_net_ent(server_fd, &my_service_ent);

  // receive the number of peers to connect to
  recv_int(server_fd, &n_peer);

  if(n_peer == 0) // if i am the first peer
  {
    char gen_id[BUFFLEN];
    sprintf(gen_id, "%d:%s", rand(), gen_time_stamp() );
    printf("I'm the first peer of p2p network.\n");
    printf("I generate the genesis block by calculating the hash of the following id [random_number:time_stamp] : \n");
    printf("%s\n", gen_id);

    // create the genesis block of blockchain
    init_genesis(block_chain, my_service_ent, gen_id);

    // check if use a fake blockchain for testing
    if(test_mode)
      test_bchain();

    // send 1 to confirm that the genesis block was created
    send_short(server_fd, 1);

    printf("Send confirm of genesis block creation to server\n");
    hooked = true;
  }
  else
  {
    // I'm not the first peer, so n_peer tells me the number of peers i need to try to connect to
    printf("\nI'm not the first peer. Number of peers that I have to try to connect: %d\n", n_peer);

    // counter of succesfull connections
    int count_succ_conn = 0;
    // response from peer
    short confirmed_conn = 0;

    struct s_net_ent tmp_ent;
    Connected_ent new_peer = NULL;

    for(int i=0; i<n_peer; i++)
    {
      // receive the net_ent of peer
      recv_net_ent(server_fd, &tmp_ent);

      printf("\nreceived peer to connect: ");
      visit_net_ent(&tmp_ent);

      /*
        download the blockchain only from the first peer, because any more
        up-to-date blocks of subsequent peers will still reach me through the
        first peer that just after having received them will also send them to me
      */
      if(count_succ_conn == 0) // if it's the first peer I connect to
        new_peer = hook_to_peer(tmp_ent, 1); // 1 for download the bchain
      else
        new_peer = hook_to_peer(tmp_ent, 0); // 0 for only hook

      if( new_peer != NULL )
      {
        // connection confirmed
        confirmed_conn = 1;
        count_succ_conn++;

        /* update fd_open and max_fd
          (critical section not necessary because at this time there are no other threads)
        */
        fd_open[new_peer->fd] = 1;
        max_fd = (new_peer->fd > max_fd) ? new_peer->fd : max_fd;

        printf("\nconnection with peer, success");

        // add new peer to peers list
        add_to_list(conn_peer, new_peer);
      }
      else
      {
        confirmed_conn = 0;
        printf("\nconnection with peer, failled\n");
      }

      // send result of connection
      send_short(server_fd, confirmed_conn);
    }

    // check if at least one connection was successful
    if(count_succ_conn > 0)
      hooked = true;
  }

  close(server_fd);
  return hooked;
}



// -----------------------------------------------------------------------------USED BY THREADS
Block sync_in_blockchain(Trns tr, Block bl_to_add)
{
  bool done, retry;
  int seq_chosen, rand_sec;
  unsigned char prev_id[SHA256_DIGEST_LENGTH];
  Block new_bl;

  done = true;
  while(done)
  {
    // start booking cycle
    retry = true;
    while(retry)
    {
      rw_sincro_entry_section(sincro_block_chain, WRITER);
        // check if any thread is already waiting to (re)create a block
        retry = flag_tid;
        if(!retry)
        {
          // save the chosen sequence number for block
          seq_chosen = get_last_seq(block_chain) + 1;
          // set to indicate that i want to (re)create a block
          flag_tid = true;
          // save my tid to check if it is still up to me to insert the block
          reserved_tid = pthread_self();
        }
      rw_sincro_exit_section(sincro_block_chain, WRITER);
    }

    // TIME FOR BLOCK "MINING"
    rand_sec = 5+rand()%11;
    if(test_mode)
      sleep(test_mode);
    else
      sleep(rand_sec);


    // (RE)CREATION BLOCK PHASE
    rw_sincro_entry_section(sincro_block_chain, WRITER);

      /* check if seq_chosen is still valid or if the arrival of a block
         from the outside has meant that I must invalidate seq_chosen
         and choose another one */
      if( pthread_equal( reserved_tid, pthread_self() )  )
      {
        // get previous id of chosen sequence number
        get_prev_id(block_chain, prev_id, seq_chosen);

        // check if adding a new block
        if(bl_to_add == NULL)
        {
          // create it
          new_bl = generate_block(
                      tr,                 // info
                      describe_trns(tr),  // not hash id
                      prev_id,            // previous hash id
                      rand_sec,           // random "mining" time
                      seq_chosen,         // sequence number of block
                      my_service_ent);    /* only I can create or recreate
                                            a block in my blockchain */
          add_block(block_chain, new_bl);
        }
        else // or if adding a block to recreate
        {
          Trns old_tr = (Trns)bl_to_add->info;
          Trns new_tr = create_transaction(old_tr->time_stamp, old_tr->amount, old_tr->src, old_tr->dst);

          // recreate it
          new_bl = generate_block(
                          new_tr,
                          describe_trns(new_tr),
                          prev_id,
                          rand_sec,
                          seq_chosen,
                          my_service_ent);

          // add in block_chain and remove from list of tails to cut
          add_recreated_tail(block_chain, new_bl, bl_to_add->id);
        }

        // blocck added:
        done = false;

        // releases the reservation for add a block
        flag_tid = false;
      }
    rw_sincro_exit_section(sincro_block_chain, WRITER);
  }

  return new_bl;
}



void flooding(void* pkg, short pkg_type, struct s_net_ent pkg_sender)
{
  List peers_to_contact = create_list();

  // get a copy of actual connected peer list
  rw_sincro_entry_section(sincro_conn_peer, READER);
    append_list(peers_to_contact, conn_peer);
  rw_sincro_exit_section(sincro_conn_peer, READER);


  while(peers_to_contact->size > 0)
  {
    // pop front
    Connected_ent peer = extract_from_list_by_index(peers_to_contact, 0);

    // check if the peer extracted is the peer that sent me the pkg
    if(compare_net_ent( &pkg_sender, &(peer->ent) ) )
      continue;

    rw_sincro_entry_section(sincro_conn_peer, READER);

      // check if the file descriptor to use is occupied by another thread
      if(fd_open[peer->fd] == 0)
      {
        rw_sincro_exit_section(sincro_conn_peer, READER);

        // push back in because it hasn't been contacted yet
        add_to_list(peers_to_contact, peer);
      }
      else // peer to send the package found
      {
        // indicates that the file descriptor is busy
        fd_open[peer->fd] = 0;

        rw_sincro_exit_section(sincro_conn_peer, READER);

        // refresh pselect
        kill(0, SIGUSR1);

        // send the macro for pkg type
        send_short(peer->fd, pkg_type);

        // cast the pointer based on the package type and send it
        if(pkg_type == P_BLOCK)
        {
          Block bl = (Block)pkg;
          send_block_pkg(peer->fd, bl);
        }
        else // P_RECREATED_BLOCK type
        {
          Recreated_pkg rp = (Recreated_pkg)pkg;
          send_recreated_pkg(peer->fd, rp);
        }

        // "free" the file descriptor
        rw_sincro_entry_section(sincro_conn_peer, WRITER);
          fd_open[peer->fd] = 1;
        rw_sincro_exit_section(sincro_conn_peer, WRITER);

        // refresh pselect
        kill(0, SIGUSR1);
      }
  }

  free(peers_to_contact);
}



void warn_wallet(Trns t)
{
  int found;
  struct s_connected_ent dst;
  Connected_ent wallet;

  /* fill a Connected_ent with the net_ent of destination wallet  of transaction,
     for search it in the list of connected wallet */
  dst.ent = t->dst;

  rw_sincro_entry_section(sincro_conn_wallet, READER);

  // return index of found Wallet
  found = search_by_info(conn_wallet, &dst, compare_connected_ent);

  if(found != -1)
  {
    wallet = search_by_index(conn_wallet, found);
    rw_sincro_exit_section(sincro_conn_wallet, READER);

    // send the transaction to wallet
    send_trns(wallet->fd, t);
  }
  else
    rw_sincro_exit_section(sincro_conn_wallet, READER);

}



void recreate_block()
{
  List blocks_to_check = create_list();

  // get a copy of actual tails to cut list
  rw_sincro_entry_section(sincro_block_chain, READER);
    append_list(blocks_to_check, block_chain->tails_to_cut);
  rw_sincro_exit_section(sincro_block_chain, READER);


  while(blocks_to_check->size > 0)
  {
    // pop front
    Tail_tc prev_bl_to_recreate = extract_from_list_by_index(blocks_to_check, 0);

    Block bl_to_recreate;

    if(prev_bl_to_recreate->prev_type == FATHER)
      bl_to_recreate = prev_bl_to_recreate->prev_bl->next;
    else // BROTHER PREV TYPE
      bl_to_recreate = prev_bl_to_recreate->prev_bl->side;

    // check if i was the creator
    if(compare_net_ent( &(bl_to_recreate->creator), &my_service_ent ) )
    {
      // init the pkg to send
      Recreated_pkg rp = (Recreated_pkg)obj_malloc(RECREATED_PKG_SIZE);
      hashcpy(rp->old_id, bl_to_recreate->id);
      rp->rec_bl = sync_in_blockchain(NULL, bl_to_recreate);

      // flooding of recreated block
      flooding(rp, P_RECREATED_BLOCK, my_service_ent);
    }
  }

}



void print_peer_state()
{

  printf("\nѶ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ\n\n");
  printf("\t\tPEER: [%s:%d]\n\n", my_service_ent.addr, my_service_ent.port);

  printf("\nѶ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ\n\n");
  printf("\t\tBLOCK-CHAIN\n");
  rw_sincro_entry_section(sincro_block_chain, READER);
    visit_bchain(block_chain, visit_trns);
  rw_sincro_exit_section(sincro_block_chain, READER);

  printf("\nѶ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ\n\n");
  printf("\t\tCONNECTED PEERS\n\n");
  rw_sincro_entry_section(sincro_conn_peer, READER);
    visit_list(conn_peer, visit_connected_ent);
  rw_sincro_exit_section(sincro_conn_peer, READER);

  printf("\nѶ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ\n\n");
  printf("\t\tCONNECTED WALLETS\n\n");
  rw_sincro_entry_section(sincro_conn_wallet, READER);
    visit_list(conn_wallet, visit_connected_ent);
  rw_sincro_exit_section(sincro_conn_wallet, READER);
  printf("\nѶ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ-Ѷ\n\n");

  printf("\nwaiting for requests...\n");
}



void refresh_state()
{
  // print new status
  sem_post(printer_sem);
  // refresh pselect
  kill(0, SIGUSR1);
}



void test_bchain()
{
  Block a_b[4]; // array of block
  struct s_net_ent ane[4] =  { // array of net_ent
    {"127.0.0.1", 1111},
    {"127.0.0.2", 2222},
    {"127.0.0.3", 3333},
    {"127.0.0.4", 4444}
  };
  unsigned char prev_id[SHA256_DIGEST_LENGTH];
  Trns a_t[4];

  printf("\nCreating fake blockchain for test...\n");

  int i;

  // ADD BLOCK WITH SEQUENCE NUMBER 1 AND 2
  for(i = 1; i < 3; i++)
  {
    // sleep for generate different time stamp
    sleep(1);

    // create transaction
    a_t[i-1] = create_transaction( gen_time_stamp(), i*100, ane[i-1], ane[i-1] );

    // get previous id of new block
    get_prev_id(block_chain, prev_id, i);

    // generate block
    a_b[i-1] = generate_block(
                a_t[i-1], // transaction
                describe_trns(a_t[i-1]), // not_hash_id
                prev_id,
                i+5, // mining_time
                i, // sequence number
                my_service_ent
            );
    // add block
    add_block(block_chain, a_b[i-1]);
  }


  // ADD BLOCK WITH SEQUENCE NUMBER 3 (tail)
  for( int j = 0; j < 2; j++, i++)
  {
    // sleep for generate different time stamp
    sleep(1);

    // create transaction
    a_t[i-1] = create_transaction( gen_time_stamp(), i*100, ane[i-1], ane[i-1] );

    // get previous id of new block
    get_prev_id(block_chain, prev_id, 3);

    // generate block
    a_b[i-1] = generate_block(
                a_t[i-1], // transaction
                describe_trns(a_t[i-1]), // not_hash_id
                prev_id,
                i+5, // mining_time
                3, // sequence number
                my_service_ent
            );
    // add block
    add_block(block_chain, a_b[i-1]);
  }

}


// -----------------------------------------------------------------------------REQUEST SERVED BY THREADS
void *peer_state_printer()
{
  while(1)
  {
    sem_wait(printer_sem);
    print_peer_state();
    if(exit_flag)
      break;
  }
  pthread_exit(NULL);
}



void *block_to_recreate_checker()
{
  while(1)
  {
    sem_wait(checker_sem);
    recreate_block();
    sem_post(printer_sem);
    if(exit_flag)
      break;
  }
  pthread_exit(NULL);
}



void *hook_p2p(void *arg)
{
  int peer_fd = *(int*)arg;
  free(arg);

  Hook_pkg hp = (Hook_pkg)obj_malloc(HOOK_PKG_SIZE);

  // read the hook pkg request;
  recv_hook_pkg(peer_fd, hp);

  if(hp->download_flag) // check if peer want download the blockchain
  {
    printf("\nstart sending the blocks to peer...");
    visit_net_ent(&hp->ent);

    rw_sincro_entry_section(sincro_block_chain, READER);

      // read the number of blocks currently present in the blockchain
      int n_block = block_chain->size;
      // get last sequence number
      int last_seq = get_last_seq(block_chain);

      // send it to the peer
      send_int(peer_fd, n_block);

      // send all block for every sequence number
      for(int i = 0; i <= last_seq; i++)
      {
        // First Block with sequence number i
        Block fb = get_seq_block(block_chain, i);

        // send block
        send_block_pkg(peer_fd, fb);

        // send all other block with sequence number i
        while(fb->side != NULL)
        {
          fb = fb->side;
          send_block_pkg(peer_fd, fb);
        }

      }

    rw_sincro_exit_section(sincro_block_chain, READER);
    printf("\nfinished sending the blocks.");
  }

  // init new peer
  Connected_ent new_peer = (Connected_ent)obj_malloc(CONNECTED_ENT_SIZE);
  new_peer->ent = hp->ent;
  new_peer->fd = peer_fd;

  rw_sincro_entry_section(sincro_conn_peer, WRITER);
    // add new peer to list
    add_to_list(conn_peer, new_peer);

    // reactivate the descriptor for the pselect
    fd_open[peer_fd] = 1;
  rw_sincro_exit_section(sincro_conn_peer, WRITER);


  printf("\npeer hooked correctly: ");
  visit_net_ent(&(hp->ent));

  refresh_state();
  pthread_exit(NULL);
}



void *hook_w2p(void *arg)
{
  int wallet_fd = *(int*)arg;
  free(arg);

  Connected_ent wallet = (Connected_ent)obj_malloc(CONNECTED_ENT_SIZE);

  // init new wallet
  wallet->fd = wallet_fd;
  getpeer_net_ent(wallet_fd, &wallet->ent);

  rw_sincro_entry_section(sincro_conn_wallet, WRITER);
    add_to_list(conn_wallet, wallet);
  rw_sincro_exit_section(sincro_conn_wallet, WRITER);

  // send confirm to wallet
  send_short(wallet_fd, 1);

  rw_sincro_entry_section(sincro_conn_wallet, WRITER);
    // reactivate the descriptor for the pselect
    fd_open[wallet_fd] = 1;
  rw_sincro_exit_section(sincro_conn_wallet, WRITER);

  refresh_state();
  pthread_exit(NULL);
}



void *w_transaction(void* arg)
{
  int wallet_fd = *(int*)arg;
  free(arg);

  Trns tr = (Trns)obj_malloc(TRNS_SIZE);

  // read the transaction
  if( recv_trns(wallet_fd, tr) != 0 )
  {
    printf("\ntransaction not received correctly by the wallet. Request aborted.");
    send_short(wallet_fd, 0);
  }
  else
  {
    // send confirmation of receipt
    send_short(wallet_fd, 1);

    // create a block for transaction received, and sync it in blockchain
    Block bl_added = sync_in_blockchain(tr, NULL);

    // send confirmation of block creation
    send_short(wallet_fd, 1);

    // flooding of new block
    flooding(bl_added, P_BLOCK, my_service_ent);

    // check if a wallet is making a transaction to another wallet
    if( !compare_net_ent( &(tr->src), &(tr->dst)) )
      warn_wallet(tr);
  }

  // wake up the thread that takes care of checking if there are blocks to be recreated
  sem_post(checker_sem);

  rw_sincro_entry_section(sincro_conn_wallet, WRITER);
    // reactivate the descriptor for the pselect
    fd_open[wallet_fd] = 1;
  rw_sincro_exit_section(sincro_conn_wallet, WRITER);

  refresh_state();
  pthread_exit(NULL);

}



void *p_block(void* arg)
{
  int peer_fd = *(int*)arg;
  free(arg);

  // read the block pkg
  Block new_bl = (Block)obj_malloc(BLOCK_SIZE);
  recv_block_pkg(peer_fd, new_bl);

  rw_sincro_entry_section(sincro_block_chain, WRITER);

  // if the block is new
  if( !is_block_in_bchain(block_chain, new_bl) )
  {
    // add the block while in the critical section on the blockchain
    add_block(block_chain, new_bl);

    /*
      change the reserved_tid with the tid of this thread,
      to make all other thread understand that it is inserting a block.
      Therefore (the eventual) thread that was reserved through "reserved_tid",
      and that was "mining"(sleep) must invalidate the block and recreate it
    */
    reserved_tid = pthread_self();

    rw_sincro_exit_section(sincro_block_chain, WRITER);

    // find the peer service_net_ent that sent you the block by fd used
    rw_sincro_entry_section(sincro_conn_peer, READER);

      struct s_connected_ent tmp;
      tmp.fd = peer_fd;
      int sender_index = search_by_info(conn_peer, &tmp, compare_connected_ent_by_fd);
      Connected_ent sender_service_ent = search_by_index(conn_peer, sender_index);

    rw_sincro_exit_section(sincro_conn_peer, READER);

    // flooding of new block
    flooding(new_bl, P_BLOCK, sender_service_ent->ent);

    // warning the dst wallet if it is hook to peer
    Trns tr = (Trns)new_bl->info;
    if( !compare_net_ent( &(tr->src), &(tr->dst)) )
      warn_wallet(tr);

    // wake up the thread that takes care of checking if there are blocks to be recreated
    sem_post(checker_sem);
  }
  else
  {
    rw_sincro_exit_section(sincro_block_chain, WRITER);
    printf("\nBlock already received: do nothing to avoid loops.\n");
  }

  rw_sincro_entry_section(sincro_conn_peer, WRITER);
    // reactivate the descriptor for the pselect
    fd_open[peer_fd] = 1;
  rw_sincro_exit_section(sincro_conn_peer, WRITER);

  refresh_state();
  pthread_exit(NULL);

}



void *p_recreated_block(void* arg)
{
  int peer_fd = *(int*)arg;
  free(arg);

  Recreated_pkg rp = (Recreated_pkg)obj_malloc(RECREATED_PKG_SIZE);
  rp->rec_bl = (Block)obj_malloc(BLOCK_SIZE);

  // read the block
  recv_recreated_pkg(peer_fd, rp);

  rw_sincro_entry_section(sincro_block_chain, WRITER);

  // if the block is new
  if( !is_block_in_bchain(block_chain, rp->rec_bl) )
  {
    // add new recreated block and delete old block from tail_to_cut list
    add_recreated_tail(block_chain, rp->rec_bl, rp->old_id);

    // explained in p_block
    reserved_tid = pthread_self();

    rw_sincro_exit_section(sincro_block_chain, WRITER);
    printf("\nRecreated block received, inserted correctly.\n");

    // find the peer service_net_ent that sent you the block from fd used
    rw_sincro_entry_section(sincro_conn_peer, READER);

      struct s_connected_ent tmp;
      tmp.fd = peer_fd;
      int sender_index = search_by_info(conn_peer, &tmp, compare_connected_ent_by_fd);
      Connected_ent sender_service_ent = search_by_index(conn_peer, sender_index);

    rw_sincro_exit_section(sincro_conn_peer, READER);

    // flooding of new recreated block
    flooding(rp->rec_bl, P_RECREATED_BLOCK, sender_service_ent->ent);
    printf("\nRecreated block received, flooded correctly..\n");

    // wake up the thread that takes care of checking if there are blocks to be recreated
    sem_post(checker_sem);
  }
  else
  {
    rw_sincro_exit_section(sincro_block_chain, WRITER);
    printf("\nRecreated block already received: do nothing to avoid loops.\n");
  }


  rw_sincro_entry_section(sincro_conn_peer, WRITER);
    // reactivate the descriptor for the pselect
    fd_open[peer_fd] = 1;
  rw_sincro_exit_section(sincro_conn_peer, WRITER);

  refresh_state();
  pthread_exit(NULL);
}



void *w_balance(void* arg)
{
  int wallet_fd = *(int*)arg;
  free(arg);

  List wallet_trns;

  // get the wallet net_ent by fd
  struct s_net_ent w_ent;
  getpeer_net_ent(wallet_fd, &w_ent);

  // create a fake trns that contain the wallet net_ent for search the blocks
  // that concern the wallet in blockchain
  struct s_trns w_trns = {
    0.0,    // amount
    w_ent,  // src
    w_ent,  // dst
    ""      // timestamp
  };

  // get the list of transactions that concern the wallet
  rw_sincro_entry_section(sincro_block_chain, READER);
    wallet_trns = get_related_info(block_chain, &w_trns, is_src_or_dst_equal);
  rw_sincro_exit_section(sincro_block_chain, READER);

  if(wallet_trns == NULL)
  {
    // no transactions refers to the wallet
    send_int(wallet_fd, 0);
  }
  else
  {
    // send the number of transactions refers to the wallet
    send_int(wallet_fd, wallet_trns->size);

    // send trns one at time
    while(wallet_trns->size > 0)
    {
      // pop front
      Trns tmp_trns = extract_from_list_by_index(wallet_trns, 0);
      // send trns
      send_trns(wallet_fd, tmp_trns);
    }

    free(wallet_trns);
  }


  rw_sincro_entry_section(sincro_conn_wallet, WRITER);
    // reactivate the descriptor for the pselect
    fd_open[wallet_fd] = 1;
  rw_sincro_exit_section(sincro_conn_wallet, WRITER);

  refresh_state();
  pthread_exit(NULL);
}



void *socket_error(void *arg)
{
  int error_fd = *(int*)arg;
  free(arg);

  int sock_error;
  socklen_t sock_error_size = sizeof(sock_error);

  printf("Error on socket detected:");

  // read error on socket
  getsockopt(error_fd, SOL_SOCKET, SO_ERROR, &sock_error, &sock_error_size);
  fprintf(stderr, "\t%s\n", strerror(sock_error));
  close(error_fd);

  // -----------------------check which connection has fallen (peer or wallet)

  Connected_ent error_ent_found = NULL;
  struct s_connected_ent error_ent;
  error_ent.fd = error_fd;

  // check in connected peer list
  rw_sincro_entry_section(sincro_conn_peer, WRITER);
    error_ent_found = (Connected_ent)extract_from_list(conn_peer, &error_ent, compare_connected_ent_by_fd);
  rw_sincro_exit_section(sincro_conn_peer, WRITER);

  // if the descriptor referred to the connection with a peer
  if( error_ent_found != NULL )
    printf("\ncrashed peer [%s:%d], removed from list \n", error_ent_found->ent.addr, error_ent_found->ent.port);
  else // the file descriptor with the error, referred to a wallet
  {
    rw_sincro_entry_section(sincro_conn_wallet, WRITER);
      error_ent_found = (Connected_ent)extract_from_list(conn_wallet, &error_ent, compare_connected_ent_by_fd);
    rw_sincro_exit_section(sincro_conn_wallet, WRITER);
    printf("\nThe wallet [%s:%d] descriptor closed and removed from list\n", error_ent_found->ent.addr, error_ent_found->ent.port);
  }
  free(error_ent_found);


  rw_sincro_entry_section(sincro_conn_peer, WRITER);
  rw_sincro_entry_section(sincro_conn_wallet, WRITER);
    if( error_fd == max_fd ) // if the closed error_fd was the maximum
    {
      // update max_fd to optimize fd_open scanning
      while(fd_open[--error_fd] == 0) ;// only for --i

      max_fd = error_fd;
    }
  rw_sincro_exit_section(sincro_conn_wallet, WRITER);
  rw_sincro_exit_section(sincro_conn_peer, WRITER);

  refresh_state();
  pthread_exit(NULL);
}


// -----------------------------------------------------------------------------UTILITY
void read_cli_param(int argc, char **argv)
{
  /*var used for argv parsing*/
  int opt;
  int flags[4] = { 0 };

  // default value for test_mode
  test_mode = 0;

  while ((opt = getopt(argc, argv, "a:b:p:s:t:h")) != -1)
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

      case 's': // Service peer Port
        my_service_ent.port = atoi(optarg);
        flags[3] = 1;
        break;

      case 't': // Test mode activated
        test_mode = atoi(optarg);
        printf("\nTEST MODE ACTIVE. Time for minining block = %d\n", test_mode);
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


void init_global_var()
{
  // setup pthread_attr structure for detach_state use
  attr = (pthread_attr_t*)obj_malloc( sizeof(pthread_attr_t) );
  pthread_attr_init( attr );
  pthread_attr_setdetachstate(attr, PTHREAD_CREATE_DETACHED);

  // semaphore "event" (set to 0) used to activate the printer and the checker thread
  printer_sem = (sem_t*)obj_malloc(sizeof(sem_t));
  checker_sem = (sem_t*)obj_malloc(sizeof(sem_t));
  sem_init(printer_sem, 0, 0);
  sem_init(checker_sem, 0, 0);

  // blockchain mangement stuff
  flag_tid = false;
  check_ttc = false;

  // Signal stuff
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

  sem_destroy(printer_sem);
  sem_destroy(checker_sem);

  empty_list(conn_peer);
  free(conn_peer);

  empty_list(conn_wallet);
  free(conn_wallet);

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
    refresh_flag = true;
  
}


void delete_from_network()
{
  int server_fd;

  // access to network with hashed password
  server_fd = server_auth(server_ent, hash_psw);

  printf("Send macro to server...\n");
  // send request for delete peer from network
  send_short(server_fd, CLOSE_PEER);

  // send my service ent for delete me from network
  send_net_ent(server_fd, &my_service_ent);
}
