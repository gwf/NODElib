
/* Copyright (c) 1995 by G. W. Flake.
 *
 * NAME
 *   hash.h - generic but expandable hash tables
 * SYNOPSIS
 *   This module defines a hash table for objects that can be
 *   expressed as a void pointer.  The table size dynamically
 *   doubles in size if a load factor is exceeded, but without
 *   recomputation of the hash keys.  Very nifty.
 * AUTHOR
 *   Gary William Flake (\url{\bf{gary.flake@usa.net}}{mailto:gary.flake@usa.net}).
 * CREDITS
 *   The idea for this package came from a post from Chris Torek in
 *   comp.lang.c.
 */


#include <stdio.h>

#ifndef __HASH_H__
#define __HASH_H__

#include "nodelib/etc/version.h"
#include "nodelib/etc/options.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* This is the node structure that we use to resolve collisions.  The
   \em{key} is saved so that when the table expands, hash indices do not
   need to be recomputed. */

typedef struct HASH_NODE {
  struct HASH_NODE  *next;
  void              *data;
  unsigned long       key;
} HASH_NODE;


/* Our hash table structure is pretty straight forward. You should never
   need to access the structure directly.  The information here is 
   provided only for the curious. */

typedef struct HASH {
  HASH_NODE     **table;
  void           *obj;
  unsigned long   size, used, limit, mask, collisions;
  unsigned long (*numify)(const void *elem, void *obj);
  int           (*compare)(const void *a, const void *b, void *obj);
  double          load;
} HASH;

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* The create function takes a requested size, a function which
   will convert a key to an integer, a compare fuction used to
   compare two elements, a user specified pointer that is passed to
   the previous two function, and returns a pointer to a newly allocated
   HASH table. */

HASH *hash_create(unsigned sz,  /*\*/
                  unsigned long (*numify)(const void *a, void *obj),  /*\*/
                  int (*compare)(const void *a, const void *b,  /*\*/
				 void *obj), /*\*/
                  void *obj);

/* The destroy function frees all memory used by the HASH table, but will
   not free any memory associated with the hashed elements themselves.
   You must free any memory associated with the \bf{data} field
   of your nodes. */

void hash_destroy(HASH *hash);

/* This function will insert \em{elem} if no such element exists in the
   hash table already.  If \em{elem} is found (in the sense that your
   compare function says that it is identical to another entry) then the
   \em{data} field of the existing node will be replaced by elem. */

void hash_insert(HASH *hash, void *elem);

/* This function will remove the node who's data field is equal to
   \em{elem}. (in the sense of the compare function) and return
   the \em{data} field of the found node.  If the element is not
   found then NULL is returned. */

void *hash_delete(HASH *hash, void *elem);

/* This function will free all of the inserted nodes and clear the
   hash table, making it empty but still usable. */

void hash_clear(HASH *hash);

/* Returns the \em{data} field of the node which is considered
   equal to \em{elem} in the sense of the compare function. */

void *hash_search(HASH *hash, void *elem);

/* This function allows you to iterate through each element in a hash
   table. This function should be called with \em{index}  and
   \em{node} pointing to zero and NULL respectively on the first
   invokation.  The values that the paramaters point to will be 
   updated on each subsequent call, and they are used to keep track
   of the search position in the hash table.  Called repeatidly,
   you can retrieve each element in the hash table exactly once.
   You can check that you are done in three ways:
   \begin{itemize}
     \item (1) if the data never has NULL values then you can check
     for that.
     \item (2) You can loop for the number of items in the hash table.
     \item (3) You can check if \em{index} = 0 and \em{node} = NULL
        (from the caller side) which will be the case if the
        iterations are finished since after the last element
        the values are reset.
   \end{itemize}  */

void *hash_iterate(HASH *hash, int *index, HASH_NODE **node);

/* This function will call \em{func} on each node's \em{data}
   field in the supplied hash table.  The order of function calls
   in not garunteed in any way. */

void  hash_do_func(HASH *hash, void (*func)(void *elem));

/* A helper function: this will convert a NULL terminated string
   intto a big integer.  You can write your own numify function in
   terms of this function if your keys are strings. */

unsigned long hash_string_numify(char *str);

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __HASH_H__ */
