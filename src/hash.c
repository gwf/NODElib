
/* Copyright (c) 1995 by G. W. Flake.
 *
 * NOTES
 *   + The big question is: Do we really need to sort the collision lists?
 *     After some thought, I decided that the answer is "no".  The goal
 *     of this package is to provide hash routines that can be used on
 *     any data type, and be *adaptable* to unknown circumstances.  The
 *     key feature is the auto-table-expansion.  If the tables had sorted
 *     collision lists, then they would have to be resorted on expansion.
 *     Another point is that during insertion we need to check for
 *     duplicate data items (items in which the compare function states
 *     that they are equal).  If the lists were sorted then the compare
 *     function would have to be called on every node in the list
 *     until a suitable location was found.  This is a big loose.  With
 *     unsorted lists, we can first test for identical keys and only if
 *     two items have identical keys do we invoke the compare function.
 *     This is a big win.
 *   + I never bother to check if the size is so large as to make the
 *     mask bits insignificant.  If this is the case, then chances are
 *     that you require a hash table as large as addressable memory.  If
 *     this is the case, then I can't help you.  You need something
 *     to hash your data externally, ie. to disk.
 *   + The idea for this came from a post from Chris Torek in c.l.c.
 * HISTORY
 *   peyote - Dec  9, 1992: Created.
 *   peyote - Apr  8, 1993: Added hash_first() so that I could step
 *     through a hash table and free up memory associated with any
 *     element.
 *   peyote - Apr 23, 1993: Added hash_iterate() to do the job better.
 *   peyote - Apr 23, 1993: V1.0.
 *   peyote - May 21, 1993: Bug report from roux@cenaath.cena.dgac.fr.
 *                          Index problem in hash_iterate().
 *   peyote - Oct  7, 1993: Added hash_do_func().
 *   peyote - Oct  8, 1993: Added hash_free_all().
 *   flake  - Nov  8, 1996: Added several casts contributed by WEB.
 */


#include <math.h>
#include <limits.h>

#undef XALLOC_DEBUG

#include "nodelib/xalloc.h"
#include "nodelib/hash.h"


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* Efficiently and destructively returns the least power of two greater
   than or equal to X.  I lifted this from the GPERF sources. */

#define HASH_POW(X) \
  ((!X)?1:(X-=1,X|=X>>1,X|=X>>2,X|=X>>4,X|=X>>8,X|=X>>16,(++X)))


/* Knuth suggests this value for the multiplicative hashing method,
   which is approximately (sqrt(5) - 1) / 2.  Who am I to argue? */

#define HASH_MAGIC 0.6180339887


/* Translate an integer key to the range of [0 ... (range - 1)] */

#define HASH_FUNC(key) \
  (unsigned)(floor(UINT_MAX * fmod(key * HASH_MAGIC, 1.0)))


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#undef HASH_FUNC

INLINE unsigned HASH_FUNC(unsigned key)
{
  double x, y;
  unsigned long a;

  x = HASH_MAGIC * key;
  a = x;
  y = x - a;
  return UINT_MAX * y;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* Create a hash table.  This will round up SIZE to the smallest power
   of two, greater than or equal to SIZE. */

HASH *hash_create(unsigned size,
		  unsigned long (*numify)(const void *elem, void *obj),
		  int (*compare)(const void *a, const void *b, void *obj),
		  void *obj)
{
  HASH *hash;
  int   newsize, i;

  newsize = size;
  HASH_POW(newsize);
  hash = (HASH *)xmalloc(sizeof(HASH));
  hash->table = (HASH_NODE **)xmalloc(sizeof(HASH_NODE *) * newsize);
  hash->obj = obj;
  hash->numify = numify;
  hash->compare = compare;
  hash->size = newsize;
  hash->used = 0;
  hash->mask = newsize - 1;
  hash->collisions = 0;
  hash->load = 0.5;
  /* cast added   WEB */
  hash->limit = (unsigned long)(hash->size * hash->load);
  for(i = 0; i < newsize; i++)
    hash->table[i] = NULL;
  return(hash);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* We need this so that that LIMIT will be recomputed. Otherwise, we
   have to do a float multiplication on each insertion. */

void hash_load(HASH *hash, double load)
{
  hash->load = load;
  /* cast added   WEB */
  hash->limit = (unsigned long)(hash->size * load);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void hash_clear(HASH *hash)
{
  unsigned i, sz;
  HASH_NODE *node, *next;

  sz = hash->size;
  for(i = 0; i < sz; i++) {
    node = hash->table[i];
    while(node) {
      next = node->next;
      xfree(node);
      node = next;
    }
    hash->table[i] = NULL;
  }
  hash->used = 0;
  hash->collisions = 0;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void hash_destroy(HASH *hash)
{
  unsigned i, sz;
  HASH_NODE *node, *next;

  sz = hash->size;
  for(i = 0; i < sz; i++) {
    node = hash->table[i];
    while(node) {
      next = node->next;
      xfree(node);
      node = next;
    }
  }
  xfree(hash->table);
  xfree(hash);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* Allocates a new table, double the size of the old, and reinserts the
   nodes from the old table into the new, with the new mask.  The old
   table is freed, of course. */

static void hash_expand(HASH *hash)
{
  HASH_NODE **table, *node, *next, **tmp;
  unsigned newsize, oldsize, key, newmask, cols, i;
  
  oldsize = hash->size;
  newsize = 2 * oldsize;
  newmask = newsize - 1;
  tmp = table = (HASH_NODE **)xmalloc(sizeof(HASH_NODE *) * newsize);
  for(i = 0; i < newsize; i++)
    *tmp++ = NULL;
  cols = 0;
  tmp = hash->table;
  for(i = 0; i < oldsize; i++) {
    node = *tmp++;
    while(node) {
      key = node->key & newmask;
      next = node->next;
      node->next = table[key];
      table[key] = node;
      if(node->next)
	cols++;
      node = next;
    }
  }
  xfree(hash->table);
  hash->table = table;
  hash->size = newsize;
  hash->mask = newmask;
  hash->collisions = cols;
  /* cast added   WEB */
  hash->limit = (unsigned long)(hash->load * newsize);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* Calls hash_expand() if needed, inserts the node, and notes collisions. */

void hash_insert(HASH *hash, void *data)
{
  HASH_NODE *node, **head;
  unsigned key;
  int num;

  if(hash->used >= hash->limit)
    hash_expand(hash);
  num = (*hash->numify)(data, hash->obj);
  key = HASH_FUNC(num);
  head = &hash->table[key & hash->mask];
  node = *head;
  while(node) {
    if(node->key != key)
      node = node->next;
    else if((*hash->compare)(data, node->data, hash->obj))
      node = node->next;
    else
      break;
  }
  if(node)
    node->data = data;
  else {
    node = (HASH_NODE *)xmalloc(sizeof(HASH_NODE));
    node->data = data;
    node->key = key;
    node->next = *head;
    hash->used++;
    if(*head)
      hash->collisions++;
    *head = node;
  }
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* Delete a node, update collision record, and return the data associated
   with the node.  NULL is returned if the node is not found. */

void *hash_delete(HASH *hash, void *data)
{
  HASH_NODE *node, *last, **head;
  unsigned key;
  void *save;
  int num;

  num = (*hash->numify)(data, hash->obj);
  key = HASH_FUNC(num);
  head = &hash->table[key & hash->mask];
  node = *head;
  last = NULL;
  while(node) {
    if(node->key != key) {
      last = node;
      node = node->next;
    }
    else if((*hash->compare)(data, node->data, hash->obj)) {
      last = node;
      node = node->next;
    }
    else
      break;
  }
  save = NULL;
  if(node) {
    save = node->data;
    if(last == NULL)
      *head = node->next;
    else
      last->next = node->next;
    hash->used--;
    if(*head)
      hash->collisions--;
    xfree(node);
  }
  return(save);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* This function should be called with INDEX and NODE pointing to
   zero and NULL, respectively, on the first invokation.  They will
   be updated on each call, and they are used to keep track of a search
   position in the hash.  Called repeatidly, you can retrieve each
   element in the hash table exactly once.  You can check that you are
   done in three ways:  1) if the data never has NULL values the check
   for that;  2) You can loop for the number of items in the hash table;
   or 3) You can check if index = 0 and node = NULL (from the caller
   side) which will be the case, since after the last element the
   values are reset.  Nota Bene: index = 0 should be sufficient. */

void *hash_iterate(HASH *hash, int *index, HASH_NODE **node)
{
  HASH_NODE *n;
  unsigned   i;

  /* Empty table: the trivial case. */
  if(hash->used == 0) {
    *index = 0, *node = NULL;
    return(NULL);
  }
  i = *index, n = *node;

  /* Check to see if we already ran off the end. */
  if(i >= hash->size)
    i = 0, n = NULL;

  /* If 'n' points to something and it has a valid next pointer
   * then return that one.
   */
  if(n && n->next) {
    *node = n->next;
    return(n->next->data);
  }
  
  /* We ran to the end of this collision list. so go down one. */
  else if(n)
    i++, n = NULL;

  /* Search the table from index to the end, looking for the next
   * entry with something in it.
   *
   * Bug report from roux@cenaath.cena.dgac.fr.  Index below could have
   * been out of bounds.  Fixed.
   */
  while(i < hash->size && !(n = hash->table[i]))
    i++;


  /* If we got something, return it, otherwise, reset the pointers
   * and return NULL.
   */
  if(n) {
    *index = i, *node = n;
    return(n->data);
  }
  else {
    *index = 0, *node = NULL;
    return(NULL);
  }
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* Call 'func' on the data portion of every entry in 'hash'. */

void hash_do_func(HASH *hash, void (*func)(void *))
{
  int index = 0, i, sz;
  void *data;
  HASH_NODE *node = NULL;

  sz = hash->used;
  for(i = 0; i < sz; i++) {
    data = hash_iterate(hash, &index, &node);
    (*func)(data);
  }
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* Returns the data portion of a node, if found, NULL if not. */

void *hash_search(HASH *hash, void *data)
{
  HASH_NODE *node;
  unsigned key;
  int num;

  num = (*hash->numify)(data, hash->obj);
  key = HASH_FUNC(num);
  node = hash->table[key & hash->mask];
  while(node) {
    if(node->key != key)
      node = node->next;
    else if((*hash->compare)(data, node->data, hash->obj))
      node = node->next;
    else
      break;
  }
  return(node ? node->data : NULL);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* Efficiently translate a string to an integer.  Note that this function
   looses when strlen(str) >> sizeof(long) * 4. */

unsigned long hash_string_numify(char *str)
{
  unsigned long i;

  i = 0;
  while(*str)
    i = (i << 2) ^ *str++;
  return(i);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


