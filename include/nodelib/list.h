
/* Copyright (c) 1998 by G. W. Flake.
 *
 * NAME
 *   list.h - generic doubly linked lists
 * SYNOPSIS
 *   This module defines a linked list for objects that can be
 *   expressed as a void pointer.  Just the basics are provided.
 * AUTHOR
 *   Gary William Flake (\url{\bf{gary.flake@usa.net}}{mailto:gary.flake@usa.net}).
 */


#include <stdio.h>

#ifndef __LIST_H__
#define __LIST_H__

#include "nodelib/etc/version.h"
#include "nodelib/etc/options.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* A list node contains pointers to next and previous nodes and a pointer
   to its data.  That's it. */

typedef struct LIST_NODE {
  struct LIST_NODE *next, *prev;
  void *data;
} LIST_NODE;


/* The list itself consists only the pointers to the head and tail. */

typedef struct LIST {
  LIST_NODE *head, *tail;
  unsigned count;
} LIST;

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* Create and empty list. */

LIST *list_create(void);


/* Destroy just the LIST structure (and not any nodes). */

void list_destroy(LIST *list);


/* Destroy the LIST structure and all of its nodes. */

void list_destroy_all(LIST *list);


/* Create a list node with \em{data} as it's data. */

LIST_NODE *list_node_create(void *data);


/* Destroy a node and return it's \em{data} value. */

void *list_node_destroy(LIST_NODE *node);


/* Insert \em{node} at the front of \em{list}. */

void list_insert_head(LIST *list, LIST_NODE *node);


/* Insert \em{node} at the rear of \em{list}. */

void list_insert_tail(LIST *list, LIST_NODE *node);


/* Insert \em{node} before \em{refnode} in \em{list}. */

void list_insert_before(LIST *list, LIST_NODE *node, LIST_NODE *refnode);


/* Insert \em{node} after \em{refnode} in \em{list}. */

void list_insert_after(LIST *list, LIST_NODE *node, LIST_NODE *refnode);


/* Remove the first node and return it. */

LIST_NODE *list_remove_head(LIST *list);


/* Remove the last node and return it. */

LIST_NODE *list_remove_tail(LIST *list);


/* Remove \em{node} from \em{list}. */

void list_remove_node(LIST *list, LIST_NODE *node);


/* Call \em{func} on every \em{data} field in \em{list}, starting
   with the head and ending with the tail.. */

void list_do_func(LIST *list, void (*func)(void *data));


/* Search for a node with that matches \em{data} and return the node. */

LIST_NODE *list_search(LIST *list, void *data);

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __LIST_H__ */
