
/* Copyright (c) 1998 by G. W. Flake. */

#include <stdlib.h>
#include <stdio.h>

#include "nodelib/list.h"
#include "nodelib/xalloc.h"
#include "nodelib/ulog.h"

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

LIST *list_create(void)
{
  LIST *list = xmalloc(sizeof(LIST));

  list->head = list->tail = NULL;
  list->count = 0;
  return list;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void list_destroy(LIST *list)
{
  xfree(list);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void list_destroy_all(LIST *list)
{
  LIST_NODE *node, *next;

  for (node = list->head; node != NULL; node = next) {
    next = node->next;
    list_node_destroy(node);
  }
  list_destroy(list);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

LIST_NODE *list_node_create(void *data)
{
  LIST_NODE *node = xmalloc(sizeof(LIST_NODE));

  node->next = node->prev = NULL;
  node->data = data;
  return node;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void *list_node_destroy(LIST_NODE *node)
{
  void *data = node->data;
  
  xfree(node);
  return data;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void list_insert_head(LIST *list, LIST_NODE *node)
{
  if (list->count == 0) {
    list->head = list->tail = node;
    node->next = node->prev = NULL;
  }
  else {
    node->next = list->head;
    node->prev = NULL;
    list->head->prev = node;
    list->head = node;
  }
  list->count++;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void list_insert_tail(LIST *list, LIST_NODE *node)
{
  if (list->count == 0) {
    list->head = list->tail = node;
    node->next = node->prev = NULL;
  }
  else {
    node->prev = list->tail;
    node->next = NULL;
    list->tail->next = node;
    list->tail = node;
  }
  list->count++;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void list_insert_before(LIST *list, LIST_NODE *node, LIST_NODE *refnode)
{
  node->next = refnode;
  node->prev = refnode->prev;
  if (refnode->prev == NULL)
    list->head = node;
  else
    refnode->prev->next = node;
  refnode->prev = node;
  list->count++;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void list_insert_after(LIST *list, LIST_NODE *node, LIST_NODE *refnode)
{
  node->prev = refnode;
  node->next = refnode->next;
  if (refnode->next == NULL)
    list->tail = node;
  else
    refnode->next->prev = node;
  refnode->next = node;
  list->count++;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

LIST_NODE *list_remove_head(LIST *list)
{
  LIST_NODE *node;

  if (list->head == NULL) return NULL;
  node = list->head;
  if (list->count == 1)
    list->head = list->tail = NULL;
  else {
    node->next->prev = NULL;
    list->head = node->next;
  }
  node->next = node->prev = NULL;
  list->count--;
  return node;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

LIST_NODE *list_remove_tail(LIST *list)
{
  LIST_NODE *node;

  if (list->tail == NULL) return NULL;
  node = list->tail;
  if (list->count == 1)
    list->head = list->tail = NULL;
  else {
    node->prev->next = NULL;
    list->tail = node->prev;
  }
  node->next = node->prev = NULL;
  list->count--;
  return node;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void list_remove_node(LIST *list, LIST_NODE *node)
{
  if (list->tail == NULL) {
    ulog(ULOG_ERROR, "list_remove_node: tried to remove node from empty list");
    return;
  }

  if (list->count == 1) { 
    list->head = list->tail = NULL;
    list->count = 0;
  }
  else if (list->head == node) list_remove_head(list);
  else if (list->tail == node) list_remove_tail(list);
  else {
    node->prev->next = node->next;
    node->next->prev = node->prev;
    node->next = node->prev = NULL;
    list->count--;
  }
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void list_do_func(LIST *list, void (*func)(void *data))
{
  LIST_NODE *node;

  for (node = list->head; node != NULL; node = node->next)
    func(node->data);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

LIST_NODE *list_search(LIST *list, void *data)
{
  LIST_NODE *node;

  for (node = list->head; node != NULL; node = node->next)
    if (node->data == data) return node;
  return NULL;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


