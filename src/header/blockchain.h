#ifndef BLOCKCHAIN_H
#define BLOCKCHAIN_H

#include "utils.h"
#include "hash.h"
#include "net.h"
#include "list.h"

struct s_block
{
	void* info; //info to decentralize via block chain
  unsigned char id[ SHA256_DIGEST_LENGTH ]; // block id
  unsigned char prev_id[ SHA256_DIGEST_LENGTH ]; // id del block precedente
  short rand_sec; // random seconds to wait [5,15]
  int seq; // sequence number
  struct s_net_ent creator;
  short confirmed; // confirmation flag

  struct s_block* side; // pointer to same sequence number block
	struct s_block* next; // pointer to next sequence number block
};
typedef struct s_block* Block;
#define BLOCK_SIZE sizeof(struct s_block)


// Tail To Cut
struct s_ttc
{
  Block prev_bl; // previous block that contain the tail to be cut
  int prev_type; // flag that indicates type of previus block:
                 // If FATHER, the pointer to tail is "next". Else (BROTHER) is "side"
};
enum prev_ttc_type { FATHER, BROTHER };
typedef struct s_ttc* Tail_tc;
#define TTC_SIZE sizeof(struct s_ttc)


struct s_bchain
{
  // pointer to genesis block
  Block genesis;
  // pointer to tail with max rand_sec
  Block max_tail;
  // list of tails waiting to be recreated
  List tails_to_cut;
  int size;
};
typedef struct s_bchain* Bchain;
#define BCHAIN_SIZE sizeof(struct s_bchain)




//------------------------------------------------------------------------------EXTERNAL IMPLEMENTATION
// to be implemented for those who use the functions of
// search / extraction / delete / visit

#define VISIT_BLOCK_INFO void(*visit_block_info)(void*)
#define COMPARE_BLOCK_INFO bool(*compare_block_info)(void*, void*)

// generic visit(print) function used for visit every block info of blockchain
void visit_block_info(void *info);
// generic compare function used for compare info of one block with another
bool compare_block_info(void* x, void* y);

//------------------------------------------------------------------------------SUPPORT METHOD
//-----------------------------------------------------------CONSTRUCTION METHOD
Bchain create_bchain();
Block generate_block(
  void *info,
  char not_hash_id[BUFFLEN],
  hash_t prev_id,
  short rand_sec, int seq,
  struct s_net_ent creator
);
void init_genesis(Bchain bc, struct s_net_ent creator, char* seed);

//--------------------------------------------------------------------GET METHOD
int get_last_seq(Bchain bc);
hash_t get_last_id(Bchain bc);
Block get_seq_block(Bchain bc, int seq);
Block get_seq_max_block(Bchain bc, int seq);
bool get_prev_id(Bchain bc, unsigned char prev_id[SHA256_DIGEST_LENGTH], int seq);
List get_related_info(Bchain bc, void* related_info, COMPARE_BLOCK_INFO);


//----------------------------------------------------------------CONTROL METHOD
bool is_bchain_empty(Bchain bc);
bool has_multi_tail(Bchain bc);
bool is_block_in_bchain(Bchain bc, Block bl);
bool is_info_in_bchain(Bchain bc, void* info, COMPARE_BLOCK_INFO);

//----------------------------------------------------------------UTILITY METHOD
hash_t id_hashing(hash_t prev_hash_id, char not_hash_id[BUFFLEN]);
void save_not_max_tails(Bchain bc);
void remove_tail(Tail_tc tail_prev);

//------------------------------------------------------------------VISIT METHOD
void visit_block(Block bl, VISIT_BLOCK_INFO);
void visit_tails_to_cut(Bchain bc, VISIT_BLOCK_INFO);
void visit_genesis(Bchain bc);
void visit_side_block(Block bl, int ti, VISIT_BLOCK_INFO);
void visit_bchain(Bchain bc, VISIT_BLOCK_INFO);

//--------------------------------------------------------------------ADD METHOD
bool add_block(Bchain bc, Block b);
bool add_recreated_tail(Bchain bc, Block recreated, hash_t old_tail_id);




#endif
