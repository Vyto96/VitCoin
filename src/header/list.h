#ifndef LIST_H
#define LIST_H


#include "utils.h"

struct s_node
{
  void* info;
  struct s_node* next;
};
typedef struct s_node* Node;
#define NODE_SIZE sizeof(struct s_node)

struct s_list
{
  Node head;
  Node tail;
  int size;
};
typedef struct s_list* List;
#define LIST_SIZE sizeof(struct s_list)


List create_list();
void empty_list(List l);

bool add_to_list(List l, void* info);
bool is_list_empty(List l);


//------------------------------------------------------------------------------
// generic visit(print) function used for visit every node of list
#define VISIT_NODE_INFO void(*visit_node_info)(void*)
// generic compare function used for compare info of node
#define COMPARE_NODE_INFO bool(*compare_node_info)(void*, void*)

// to be implemented for those who use the functions of
// search / extraction / delete / visit
bool compare_node_info(void* x, void* y);
void visit_node_info(void *info);

// EXAMPLES OF IMPLEMENTATION
/*bool compare_node_info(void* x, void* y)
{
  int a = *(int*)x, b = *(int*)y;
  if(a == b)
    return true;
  return false;
}


void visit_node_info(void *info)
{
  Node n = (Node)info;
  printf("Int: %d\n", *(int*)n->info);
  return;
}
*/


void visit_list(List l, VISIT_NODE_INFO);

int search_by_info(List l, void* info, COMPARE_NODE_INFO);
void* search_by_index(List l, int index);

bool delete_from_list(List l, void* info, COMPARE_NODE_INFO);

void* extract_from_list(List l, void* info, COMPARE_NODE_INFO);
void* extract_from_list_by_index(List l, int index);



#endif
