#include "../header/list.h"

List create_list()
{
  List newlist = (List)obj_malloc(LIST_SIZE);

  newlist->head = NULL;
  newlist->tail = NULL;
  newlist->size = 0;

  return newlist;
}


void empty_list(List l)
{
  Node tmp = NULL;
  while(l->head != NULL)
  {
    tmp = l->head;
    l->head = l->head->next;
    free(tmp);
  }
  l->size = 0;
}


bool add_to_list(List l, void* info)
{
  if(l == NULL || info == NULL)
  {
    fprintf(stderr, "\nList, info or both are NULL\n");
    return false;
  }

  Node newnode = (Node)obj_malloc(NODE_SIZE);
  newnode->info = info;
  newnode->next = NULL;

  if(l->head == NULL)
    l->head = newnode;
  else
    l->tail->next = newnode;

  l->tail = newnode;
  l->size++;

  return true;
}


bool is_list_empty(List l)
{
  return (l->size == 0) ? true : false;
}


//------------------------------------------------------------------------------
void visit_list(List l, VISIT_NODE_INFO)
{
  int i = 0;
  if(!is_list_empty(l))
  {
    Node tmp = l->head;
    while( tmp != NULL )
    {
      printf("\n[%d]\t", i++);
      visit_node_info(tmp->info);
      tmp = tmp->next;
    }
  }
  else
    fprintf(stderr,"\nList is empty\n");

  return;
}

// return index of given info
int search_by_info(List l, void* info, COMPARE_NODE_INFO)
{
  if (l == NULL || is_list_empty(l) || info == NULL)
    return -1;

  Node tmp = l->head;
  int i = 0;

  while(tmp != NULL)
  {
    if( compare_node_info(tmp->info, info) )
      return i;
    tmp = tmp->next;
    i++;
  }
  return -1;
}


void* search_by_index(List l, int index)
{
  if(l == NULL || is_list_empty(l) || index < 0 || index > l->size)
    return NULL;

  Node tmp = l->head;
  int i = 0;

  while( tmp != NULL && index != (i++) )
    tmp = tmp->next;

  return tmp->info;
}


bool delete_from_list(List l, void* info, COMPARE_NODE_INFO)
{
  if(l == NULL || info == NULL || is_list_empty(l))
    return false;

  Node marker = l->head;
  Node tmp;
  bool found = compare_node_info(marker->info, info);

  if(found)
  {
    l->head = marker->next;

    if(marker == l->tail)
      l->tail = l->head;

    l->size--;
    free(marker);
    return found;
  }

  while( !found && marker->next != NULL)
  {
    tmp = marker;
    marker = marker->next;
    found = compare_node_info(marker->info, info);
  }

  if(found)
  {
    tmp->next = marker->next;

    if(marker == l->tail)
      l->tail = tmp;

    l->size--;
    free(marker);
  }

  return found;
}


void* extract_from_list(List l, void* info, COMPARE_NODE_INFO)
{
  if(l == NULL || info == NULL || is_list_empty(l))
    return false;

  Node marker = l->head;
  Node tmp;
  bool found = compare_node_info(marker->info, info);

  if(found)
  {
    l->head = marker->next;

    if(marker == l->tail)
      l->tail = l->head;

    l->size--;
    return marker->info;
  }

  while( !found && marker->next != NULL)
  {
    tmp = marker;
    marker = marker->next;
    found = compare_node_info(marker->info, info);
  }

  if(found)
  {
    tmp->next = marker->next;

    if(marker == l->tail)
      l->tail = tmp;

    l->size--;
    return marker->info;
  }

  return NULL;
}


void* extract_from_list_by_index(List l, int index)
{
  if(l == NULL || is_list_empty(l) || index < 0 || index > l->size)
    return NULL;

  Node marker = l->head;
  Node tmp;

  if(index == 0)
  {
    l->head = marker->next;

    if(marker == l->tail)
      l->tail = l->head;

    l->size--;
    return marker->info;
  }

  int i = 0;
  do
  {
    i++;
    tmp = marker;
    marker = marker->next;
  }while(i != index);

  tmp->next = marker->next;
  l->size--;

  if(marker == l->tail)
    l->tail = tmp;

  return marker->info;
}
