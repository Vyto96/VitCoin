#ifndef BLOCKCHAIN_H
#define BLOCKCHAIN_H

#include "utils.h"
#include "hash.h"
#include "net.h"
#include "list.h"

// blockchain block
struct s_block
{
	void* info; //info to decentralize via block chain
  unsigned char id[ SHA256_DIGEST_LENGTH ]; // block id
  unsigned char prev_id[ SHA256_DIGEST_LENGTH ]; // id of the previous block
  short rand_sec; // random seconds to wait for mining simulation [5,15]
  int seq; // sequence number
  struct s_net_ent creator; // block creator

  struct s_block* side; // pointer to same sequence number block
	struct s_block* next; // pointer to next sequence number block
};
typedef struct s_block* Block;
#define BLOCK_SIZE sizeof(struct s_block)



enum prev_ttc_type { FATHER, BROTHER };
// Tail To Cut
struct s_ttc
{
  Block prev_bl; // previous block that contain the pointer (next or side) to the tail to be cut
  int prev_type; // flag that indicates type of previus block, respectively:
                 // If FATHER, the pointer to tail is "next". Else (BROTHER) is "side"
};
typedef struct s_ttc* Tail_tc;
#define TTC_SIZE sizeof(struct s_ttc)



struct s_bchain
{
  Block genesis;      // pointer to genesis block
  Block max_tail;     // pointer to tail with max rand_sec
  List tails_to_cut;  // list of tails waiting to be recreated
  int size;           // number of blocks in blockchain (not sequence number)
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
// init a new empty bchain
Bchain create_bchain();

// Block constructor
Block generate_block(
  void *info,
  char not_hash_id[BUFFLEN],
  hash_t prev_id,
  short rand_sec, int seq,
  struct s_net_ent creator
);

// initialize the genesis block and add it to the bchain
void init_genesis(Bchain bc, struct s_net_ent creator, char* seed);

//--------------------------------------------------------------------GET METHOD
// get sequence number of max tail block
int get_last_seq(Bchain bc);

// like the previous but for the id
hash_t get_last_id(Bchain bc);

// search the (first) block with the given sequence number
Block get_seq_block(Bchain bc, int seq);

// search the block with maximum waiting time for a given sequence number
Block get_seq_max_block(Bchain bc, int seq);

/* get the id of the block with sequence number previous to the one requested
  and that has maximum rand_sec value */
bool get_prev_id(Bchain bc, unsigned char prev_id[SHA256_DIGEST_LENGTH], int seq);

// get a list of info that are related to the searched info
List get_related_info(Bchain bc, void* related_info, COMPARE_BLOCK_INFO);


//----------------------------------------------------------------CONTROL METHOD
bool is_bchain_empty(Bchain bc);
bool has_multi_tail(Bchain bc);
bool is_block_in_bchain(Bchain bc, Block bl);
bool is_info_in_bchain(Bchain bc, void* info, COMPARE_BLOCK_INFO);

//----------------------------------------------------------------UTILITY METHOD
// calculation of a hash dependent on the previous hash
hash_t id_hashing(hash_t prev_hash_id, char not_hash_id[BUFFLEN]);

// save a copy of not max tails (in this moment) in tails_to_cut list
void save_not_max_tails(Bchain bc);

// remove a tail from bchain by his previous (FATHER or BROTHER) block
void remove_tail(Tail_tc tail_prev);

//------------------------------------------------------------------VISIT METHOD
void visit_block(Block bl, VISIT_BLOCK_INFO);
void visit_tails_to_cut(Bchain bc, VISIT_BLOCK_INFO);
void visit_genesis(Bchain bc);
void visit_side_block(Block bl, int ti, VISIT_BLOCK_INFO);
void visit_bchain(Bchain bc, VISIT_BLOCK_INFO);

//--------------------------------------------------------------------ADD METHOD
// add new (next or tail) block
bool add_block(Bchain bc, Block b);

// add a recreated block and remove the corresponding old tail
bool add_recreated_tail(Bchain bc, Block recreated, hash_t old_tail_id);

#endif
