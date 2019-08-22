#include "../header/blockchain.h"


//------------------------------------------------------------------------------SUPPORT METHOD
Bchain create_bchain()
{
  Bchain bc = (Bchain)obj_malloc(BCHAIN_SIZE);
  bc->genesis = NULL;
  bc->max_tail = NULL;
  bc->tails_to_cut = create_list();
  bc->size = 0;
  return bc;
}


Block generate_block(
  void *info,
  char not_hash_id[BUFFLEN],
  hash_t prev_id,
  short rand_sec, int seq,
  struct s_net_ent creator
  )
{
  Block new_block = (Block)obj_malloc(BLOCK_SIZE);

  new_block->info = info;
  hashcpy( new_block->prev_id, prev_id );

  hash_t hash_id = id_hashing(prev_id, not_hash_id);
  hashcpy(new_block->id, hash_id);

  new_block->rand_sec = rand_sec;
  new_block->seq = seq;
  new_block->creator = creator;

  new_block->side = NULL;
  new_block->next = NULL;

  return new_block;
}


void init_genesis(Bchain bc, struct s_net_ent creator, char* seed)
{
  hash_t null_prev_id = calculate_hash("NULL");

  Block gen = generate_block(
                NULL, // info
                seed, // not_hash_id
                null_prev_id, // prev_id
                0, // sequence number
                0, // random second
                creator // creator
              );

  bc->genesis = gen;
  bc->max_tail = gen;
  bc->size++;
}



//--------------------------------------------------------------------GET METHOD
int get_last_seq(Bchain bc)
{
  if(is_bchain_empty(bc))
  {
    return -1;
  }
  return bc->max_tail->seq;
}


hash_t get_last_id(Bchain bc)
{
  return bc->max_tail->id;
}


Block get_seq_block(Bchain bc, int seq)
{
  if(bc == NULL)
  {
    fprintf(stderr, "\nBlockchain pointer is NULL. Search of block with seq = %d in blockchain failed.\n", seq);
    return NULL;
  }

  if(is_bchain_empty(bc))
  {
    fprintf(stderr, "\nBlockchain is empty!\n");
    return NULL;
  }

  int ls = get_last_seq(bc); // last sequence number

  if(seq > ls || seq < 0)
  {
    fprintf(stderr, "\nSequence number not valid. Search in blockchain failed.\n");
    return NULL;
  }

  if(seq == 0)
    return bc->genesis;

  Block tmp_bl = bc->genesis->next;

  while(tmp_bl->seq != seq)
  {
    if(tmp_bl->next != NULL)
      tmp_bl = tmp_bl->next;
    else
      tmp_bl = tmp_bl->side;
  }

  return tmp_bl;
}


Block get_seq_max_block(Bchain bc, int seq)
{
  if(bc == NULL)
  {
    fprintf(stderr, "\nBlockchain pointer is NULL. Search in blockchain failed.\n");
    return NULL;
  }

  if(is_bchain_empty(bc))
  {
    fprintf(stderr, "\nBlockchain is empty!\n");
    return NULL;
  }

  int ls = get_last_seq(bc); // last sequence number

  if(seq > ls || seq < 0)
  {
    fprintf(stderr, "\nSequence number not valid. Search in blockchain failed.\n");
    return NULL;
  }

  if(seq == get_last_seq(bc))
    return bc->max_tail;

  Block tmp_bl, max_bl;

  max_bl = get_seq_block(bc, seq);
  if(max_bl == NULL)
    return NULL;


  tmp_bl = max_bl;

  /*while there is another block with the same sequence number,
    check if it is the maximum */
  while(tmp_bl->side != NULL)
  {
    tmp_bl = tmp_bl->side;

    if(tmp_bl->rand_sec > max_bl->rand_sec)
      max_bl = tmp_bl;
  }

  return max_bl;
}


bool get_prev_id(Bchain bc, unsigned char prev_id[SHA256_DIGEST_LENGTH], int seq)
{
  bool found = false;
  Block prev_block = get_seq_max_block(bc, seq-1);

  if(prev_block != NULL)
  {
    hashcpy(prev_id, prev_block->id);
    found = true;
  }

  return found;
}


List get_related_info(Bchain bc, void* searched_info, COMPARE_BLOCK_INFO)
{

  if(bc == NULL)
  {
    fprintf(stderr, "\nBlockchain pointer is NULL. Search of list of related info in blockchain failed.\n");
    return NULL;
  }

  if(searched_info == NULL)
  {
    fprintf(stderr, "\ninfo pointer to search is NULL. Search of list of related info in blockchain failed.\n");
    return NULL;
  }

  if(is_bchain_empty(bc))
  {
    fprintf(stderr, "\nBlockchain is empty!\n");
    return NULL;
  }


  // first valid block
  Block tmp_bl = bc->genesis->next;

  List found_info = create_list();

  while(tmp_bl != NULL)
  {
    // check if block info is related to the searched info
    if( compare_block_info(tmp_bl->info, searched_info) )
      add_to_list(found_info, tmp_bl->info);

    // find the max tail
    while(tmp_bl->next == NULL && tmp_bl->side != NULL)
      tmp_bl = tmp_bl->side;

    // go to next sequence block
    tmp_bl = tmp_bl->next;
  }

  if( !is_list_empty(found_info) )
    return found_info;
  else
  {
    free(found_info);
    return NULL;
  }
}



//----------------------------------------------------------------CONTROL METHOD
bool is_bchain_empty(Bchain bc)
{
  return (bc->size == 0) ? true : false;
}


bool has_multi_tail(Bchain bc)
{
  Block fbls; // First Bock with Last Sequence number
  fbls = get_seq_block(bc, get_last_seq(bc) );

  return (fbls->side != NULL) ? true : false;
}


bool is_block_in_bchain(Bchain bc, Block bl_to_search)
{
  if(bc == NULL)
  {
    fprintf(stderr, "\nBlockchain pointer is NULL. Search of block in blockchain failed.\n");
    return false;
  }

  if(bl_to_search == NULL)
  {
    fprintf(stderr, "\nBlock pointer is NULL. Search of block in blockchain failed.\n");
    return false;
  }

  if(is_bchain_empty(bc))
  {
    fprintf(stderr, "\nBlockchain is empty!\n");
    return false;
  }


  // first valid block
  Block tmp_bl = bc->genesis->next;

  bool found = false;

  while(tmp_bl != NULL && !found)
  {
    // check block id
    found = hash_equal(tmp_bl->id, bl_to_search->id);

    // find the max tail
    while(tmp_bl->next == NULL && tmp_bl->side != NULL)
      tmp_bl = tmp_bl->side;

    // go to next sequence block
    tmp_bl = tmp_bl->next;
  }

  return found;
}


bool is_info_in_bchain(Bchain bc, void* info_to_search, COMPARE_BLOCK_INFO)
{
  if(bc == NULL)
  {
    fprintf(stderr, "\n Blockchain pointer is NULL. Search of info in blockchain failed.\n");
    return false;
  }

  if(info_to_search == NULL)
  {
    fprintf(stderr, "\n info pointer to search is NULL. Search of info in blockchain failed.\n");
    return false;
  }

  if(is_bchain_empty(bc))
  {
    fprintf(stderr, "\nBlockchain is empty!\n");
    return false;
  }


  // first valid block
  Block tmp_bl = bc->genesis->next;

  bool found = false;

  while(tmp_bl != NULL && !found)
  {
    // check block info
    found = compare_block_info(tmp_bl->info, info_to_search);

    // find the max tail
    while(tmp_bl->next == NULL && tmp_bl->side != NULL)
      tmp_bl = tmp_bl->side;

    // go to next sequence block
    tmp_bl = tmp_bl->next;
  }

  return found;
}



//----------------------------------------------------------------UTILITY METHOD
hash_t id_hashing(hash_t prev_hash_id, char not_hash_id[BUFFLEN])
{
 hash_t hash_id;
 hash_t hash_cat;

 char *cat_of_hash = (char*)obj_malloc(SHA256_DIGEST_LENGTH * 2 + 2);

 hash_id = calculate_hash(not_hash_id);

 // concatenation of the two hashes
 strncpy(cat_of_hash, (const char * restrict)prev_hash_id, SHA256_DIGEST_LENGTH);
 strncpy(&cat_of_hash[SHA256_DIGEST_LENGTH], (const char *)hash_id, SHA256_DIGEST_LENGTH);
 // manual assign a endstring
 cat_of_hash[SHA256_DIGEST_LENGTH*2 + 1] = '\0';
 // calculates hash of the string obtained from the concatenation
 hash_cat = calculate_hash(cat_of_hash);

 free(cat_of_hash);
 free(hash_id);
 return hash_cat;
}


void save_not_max_tails(Bchain bc)
{
  Block prev_block = get_seq_max_block(bc, get_last_seq(bc) - 1);

  // check if the first tail is not the max tail
  if(prev_block->next != bc->max_tail)
  {
    // Tail indicated by previous block of type Father
    Tail_tc tf = (Tail_tc)obj_malloc(TTC_SIZE);
    tf->prev_bl = prev_block;
    tf->prev_type = FATHER;

    add_to_list(bc->tails_to_cut, tf);
  }

  // check the remaining side tails
  prev_block = prev_block->next;
  while(prev_block->side != NULL)
  {
    // check if the side block (other tail) is not the max tail
    if(prev_block->side != bc->max_tail)
    {
      // Tail indicated by previous block of type Brother
      Tail_tc ts = (Tail_tc)obj_malloc(TTC_SIZE);
      ts->prev_bl = prev_block;
      ts->prev_type = BROTHER;

      add_to_list(bc->tails_to_cut, ts);
    }

    prev_block = prev_block->side;
  }
  return;
}


void remove_tail(Tail_tc tail_prev)
{
  Block tmp, prev = tail_prev->prev_bl;

  if(tail_prev->prev_type == FATHER)
  {
    tmp = prev->next;
    prev->next = tmp->side;
  }
  else // prev_type == BROTHER
  {
    tmp = prev->side;
    prev->side = tmp->side;
  }

  tmp->side = NULL;
  tmp->next = NULL;

  free(tmp);
}
//------------------------------------------------------------------VISIT METHOD
void visit_block(Block bl, VISIT_BLOCK_INFO)
{
  if(bl == NULL)
  {
    fprintf(stderr, "\nBlock pointer is NULL. Visit block failed.\n");
    return;
  }

  printf("\nSEQUENCE NUMBER: {%d}\n", bl->seq);

  printf("\nPREVIOUS ID:");
  print_hash(bl->prev_id);

  printf("\nBLOCK ID:");
  print_hash(bl->id);

  printf("\nCREATOR:");
  visit_net_ent(&bl->creator);

  printf("WAITING TIME ---> %d sec\n", bl->rand_sec);

  printf("INFO-->");
  visit_block_info(bl->info);

  return;
}


void visit_tails_to_cut(Bchain bc, VISIT_BLOCK_INFO)
{
  List ttc = bc->tails_to_cut;

  for(int i=0; i<30; i++) printf("-");
  printf("\nLIST OF TAILS TO CUT\n");
  for(int i=0; i<30; i++) printf("-");

  if (is_list_empty(ttc))
  {
    fprintf(stderr, "\nList is empty\n");
    return;
  }


  Tail_tc prev_tail;
  Block bl;

  for (int i = 0; i < ttc->size; i++)
  {
    prev_tail = (Tail_tc)search_by_index(ttc, i);

    if( prev_tail->prev_type == FATHER) // prev_tail is the "father" of tail to be cut
      bl = prev_tail->prev_bl->next;
    else // prev_tail is the "brother" of tail to be cut
      bl = prev_tail->prev_bl->side;

    visit_block(bl, visit_block_info);
  }
  return;
}


void visit_genesis(Bchain bc)
{
  Block gen = bc->genesis;
  printf("\n");
  for (size_t i = 0; i < 50; i++) printf("-");
  printf("\nBLOCKCHAIN OF %d block and sequence number at %d:\n", bc->size, get_last_seq(bc));
  for (size_t i = 0; i < 50; i++) printf("-");
  printf("\nSEQUENCE NUMBER: {0} GENESIS BLOCK\n");
  printf("PREVIOUS ID (HASH OF \"NULL\"):");
  print_hash(gen->prev_id);
  printf("BLOCK ID (HASH OF SEED USED):");
  print_hash(gen->id);
  printf("CREATOR: ");
  visit_net_ent( &(gen->creator) );
  printf("RANDOM WAITING TIME: 0;\nINFO-->NULL\n");
}


void visit_side_block(Block bl, int ti, VISIT_BLOCK_INFO)
{
  if(bl == NULL)
    return;

  for(int i = 0; i < ti; i++) printf("\t");
  printf(">>>>>>>>>>>>>>>>>");

  printf("SEQUENCE NUMBER: {%d} [%d]\n", bl->seq, ti);

  for(int i = 0; i < ti; i++) printf("\t");
  printf("PREVIOUS ID:\n");
  for(int i = 0; i < ti; i++) printf("\t");
  print_hash(bl->prev_id);

  printf("\n");

  for(int i = 0; i < ti; i++) printf("\t");
  printf("BLOCK ID:\n");
  for(int i = 0; i < ti; i++) printf("\t");
  print_hash(bl->id);

  printf("\n");

  for(int i = 0; i < ti; i++) printf("\t");
  printf("CREATOR: ");
  visit_net_ent( &(bl->creator));

  for(int i = 0; i < ti; i++) printf("\t");
  printf("WAITING TIME: %d;\n", bl->rand_sec);

  for(int i = 0; i < ti; i++) printf("\t");
  printf("INFO-->");
  visit_block_info(bl->info);
  printf("\n\n");

  visit_side_block(bl->side, ++ti, visit_block_info);
  return;
}


void visit_bchain(Bchain bc, VISIT_BLOCK_INFO)
{
  if(bc == NULL)
  {
    fprintf(stderr, "\n Blockchain pointer is NULL. Visit of the blockchain failed.\n");
    return;
  }

  if(is_bchain_empty(bc))
  {
    fprintf(stderr, "\nBlockchain is empty!\n");
    return;
  }

  // visit genesis block
  visit_genesis(bc);

  // if there are no other blocks to visit besides the genesis block
  if(bc->size < 2)
    return;


  int ti = 0, // tail index
      si = 1, // sequence  index
      ls = get_last_seq(bc); // last sequence number

  // first valid block
  Block tmp_bl = bc->genesis->next;

  while(si <= ls)
  {
    for (int i = 0; i < 50; i++) printf("-");
    printf("\n");
    visit_side_block(tmp_bl, ti, visit_block_info);

    // find the right tail
    while(tmp_bl->next == NULL && tmp_bl->side != NULL)
      tmp_bl = tmp_bl->side;

    // go to next sequence block
    tmp_bl = tmp_bl->next;
    si++;
  }
  visit_tails_to_cut(bc, visit_block_info);
}

//--------------------------------------------------------------------ADD METHOD
bool add_block(Bchain bc, Block bl)
{
  if(bc == NULL || bl == NULL)
  {
    fprintf(stderr, "\npointer of Blockchain OR pointer of block to add is NULL. Addition of block to the blockchain, failed.\n");
    return false;
  }


  int ls = get_last_seq(bc); // last sequence number

  // it is possible to insert only blocks with the sequence
  // number equal to the last or equal to its next one
  if(bl->seq > ls + 1 || bl->seq < ls)
  {
    fprintf(stderr, "\nBlock sequence number (%d) not compatible with the current sequence number of the chain (%d).\n", bl->seq, get_last_seq(bc));
    return false;
  }

  // add genesis block
  if(bl->seq == 0 && ls == -1)
  {
    bc->genesis = bl;
    bc->max_tail = bl;
    bc->size++;
    return true;
  }

  // add new tail
  if( bl->seq == ls )
  {
    // find last max tail added
    Block lt = bc->max_tail;

    // check if new block has the same prev_id
    if( !hash_equal(lt->prev_id, bl->prev_id) )
    {
      fprintf(stderr, "\nprev_id of new tail not equal to prev_id of current tail.\n");
      return false;
    }

    // if max tail is not the last tail added
    while( lt->side != NULL )
      lt = lt->side;  // scroll untill find the last one

    // add the new block
    lt->side = bl;

    // update the max tail
    if(bl->rand_sec > bc->max_tail->rand_sec)
      bc->max_tail = bl;
  }
  else // add next sequence block
  {
    // last max tail
    Block lt = bc->max_tail;
    // check if new block has prev_id equal to id of max tail
    if( !hash_equal(lt->id, bl->prev_id) )
    {
      fprintf(stderr, "\nprev_id of new tail not equal to prev_id of current tail.\n");
      return false;
    }

    // check if multi-tail
    if( has_multi_tail(bc) )
    {  // add the previous block of tails that are not valid to the list of tails to be cut
      save_not_max_tails(bc);
    }

    // add the new block
    lt->next = bl;
    // update the max tail
    bc->max_tail = bl;
  }

  bc->size++;
  return true;
}


bool add_recreated_tail(Bchain bc, Block recreated, hash_t old_tail_id)
{
  if(bc == NULL || recreated == NULL)
  {
    fprintf(stderr, "\npointer of Blockchain OR pointer of recreated block to add is NULL. Addition of recreated block to the blockchain, failed.\n");
    return false;
  }


  Tail_tc tail_prev;
  int prev_ind = 0;
  bool found = false;

  while( prev_ind < bc->tails_to_cut->size && !found )
  {
    tail_prev = (Tail_tc)search_by_index(bc->tails_to_cut, prev_ind);
    Block tail = NULL;

    if(tail_prev->prev_type == FATHER)
      tail = tail_prev->prev_bl->next;
    else // BROTHER type
      tail = tail_prev->prev_bl->side;

    if(tail != NULL)
    {
      if( hash_equal(tail->id, old_tail_id) )
      {
        found = true;
        break;
      }
    }

    prev_ind++;
  }

  if(found==false)
    return false;


  // remove tail from tails_to_cut
  extract_from_list_by_index(bc->tails_to_cut, prev_ind);
  // remove tail from bchain
  remove_tail(tail_prev);
  bc->size--;
  // add the recreated block to the bchain
  add_block(bc, recreated);

  return true;
}
