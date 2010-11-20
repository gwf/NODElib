
/* Copyright (c) 1995 by G. W. Flake.
 *
 * NAME
 *   array.h - a generic array type that grows as needed
 * SYNOPSIS
 *   Use this for stacks, queues, or linear lists of any type.  The
 *   bounds will grow transparently, so you never need to be concerned
 *   about hard coded limits.
 * DESCRIPTION
 *   This package provides a pseudo-array type that will automatically
 *   grow as you insert more elements into it.  Most of the provided
 *   routines are macros, so you must be very careful to avoid
 *   side-effects.
 *   
 *   If you never have to access elements by index then you may
 *   be better off using a linked list implementation.  However, if
 *   you do need to access elements by an index, or if you are only
 *   interested in creating a stack or a queue, then these routines may
 *   work quite well for you.
 *
 *   In most of the macro descriptions that follow, the argument
 *   \em{type} refers to the element type.  Any macro that inserts
 *   or returns elements must be provided with a type.
 * AUTHOR
 *   Gary William Flake (\url{\bf{gary.flake@usa.net}}{mailto:gary.flake@usa.net}).
 */


#ifndef __ARRAY_H__
#define __ARRAY_H__

#include <stddef.h>
#include <stdlib.h>
#include "nodelib/ulog.h"
#include "nodelib/xalloc.h"
#include "nodelib/etc/version.h"
#include "nodelib/etc/options.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* This section should be skipped by users who don't care about the
   implementation specifics of this package.
  
   We store the elements of an ARRAY with three indices: \em{origin},
   \em{lindex}, and \em{rindex}. \em{origin} is the offset from 0
   in which \em{lindex} and \em{rindex} are defined. \em{lindex} 
   is the location of the first element in the array relative to the left
   of \em{origin}. \em{rindex} is the location of the last element
   relative to the right of \em{origin}.  As an example:

   \begin{verbatim}
            +-----+-----+-----+-----+-----+-----+-----+-----+
   memory:  |  0  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |
            |-----|-----|-----|-----|-----|-----|-----|-----|
   data:    |  F  |     |     |     |  B  |  C  |  D  |  E  |
            +-----+-----+-----+-----+-----+-----+-----+-----+
   \end{verbatim}

   Here we have an ARRAY with 5 elements in it.  We'll denote the elements
   as A-D, as if they are to be in some sorted order.  One possible set
   of values for the the structure fields is:
          \em{alloced} = 8,
          \em{origin}  = 5,
          \em{lindex}  = 1, and
          \em{rindex}  = 3.

   On \bf{array_prepend(ARRAY, A, TYPE)} we would place 'A' into memory
   location 3, while incrementing \em{lindex}. On
   \bf{array_append(ARRAY, G, TYPE)} we would place 'G' into memory
   location 1, while incrementing \em{rindex}.

   Obviously, from this example, we allow the elements to wrap around.
   Because of this we can guarantee that a queue implementation correctly
   coded as an ARRAY will never have more than twice the maximum number
   of elements in the queue of memory allocated.

   If \em{lindex} is 0 and you remove the front, then \em{origin} will
   be incremented.  If \em{rindex} is 0 and you remove the back then
   \em{origin} will be decremented.  This means that your \em{origin} may
   shift in unfavorable ways, but things should still work if you only use
   the provided macros and you heed the warning below about mixing push
   and pop calls with the other routines.

   The \em{hole} field is used for some macro trickery.  Ignore it. */

typedef struct ARRAY {
  unsigned   alloced;
  unsigned   origin;
  unsigned   lindex;
  unsigned   rindex;
  unsigned   elemsz;
  char      *data;
  char      *hole;
} ARRAY;

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* Prototypes for functions. */

/* h2man:ignore */
void   __array_grow_internal(ARRAY *);
/* h2man:ignore */
ARRAY *__array_create_internal(size_t sz, size_t elemsz);


/* Given the ARRAY, \em{array}, copy the \em{sz} elements pointed to
   by \em{ptr} into the front of \em{array}. The paramater \em{sz}
   represents the number of elements, not the number of bytes. */

void array_prepend_ptr(ARRAY *array, char *ptr, size_t sz);

/* Given the ARRAY, \em{array}, copy the \em{sz} elements pointed to
   by \em{ptr} into the rear of \em{array}. The paramater \em{sz}
   represents the number of elements, not the number of bytes. */

void array_append_ptr(ARRAY *array, char *ptr, size_t sz);

/* Given the ARRAY, \em{array}, copy all of the data to a new chunk
   of memory and return the new pointer.  The caller of this function
   "owns" the newly allocated data in the sense that it has the
   responsibility for freeing the returned pointer later on. */

void *array_to_pointer(ARRAY *array);

/* Removes the \em{index'th} item in the ARRAY, \em{a.}  This is rather
   involved, so it is written as a function. */

void array_destroy_index(ARRAY *a, unsigned index);

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* Create an ARRAY with a requested \em{size} and a given \em{type}. */

#define array_create(size, type) \
  __array_create_internal(size, sizeof(type))

/* Free up all memory used by the ARRAY \em{a}. */

void array_destroy(ARRAY *a);

/* Returns the number of elements in the ARRAY \em{a}, not the amount
   of memory allocated. */

#define array_size(a) ((a)->lindex + (a)->rindex)

/* This is a fairly safe method for accessing elements of 
   \em{array}.  The result of the macro expansion can be used as
   an lvalue, thus \bf{array_access(a, 5, int) = 36} is perfectly
   legal. The supplied \em{index} is checked for valid bounds.  If
   \em{index} is invalid, then a warning message is produced. */   

#define array_access(a, index, type) \
  ((((index) >= 0) && ((index) < array_size(a))) ? \
    array_fast_access((a), (index), type) : \
    (ulog(ULOG_ERROR, "array_access: bad index %d not in [0 .. %d].", \
	  (index), array_size(a)), ((type *)(a)->hole)[0]))

/* Old definition has a bug...
  (((type *) (a)->data)[(((index) > 0) && ((index) < array_size(a))) ? \
   (((a)->alloced + (a)->origin + (index) - (a)->lindex) % (a)->alloced) : \
   ((ulog(ULOG_ERROR, "array_access: bad index %d not in [0 .. %d].", \
   (index), array_size(a))), ((type *)(a)->hole)[0]) ])
 */

/* This macro is much faster then \bf{array_access()}, but
   \em{index} is not checked.  Only use this is you are certain of
   your index bounds. */

#define array_fast_access(a, index, type) \
  (((type *) (a)->data)[((a)->alloced + (a)->origin + (index) - \
                         (a)->lindex) % (a)->alloced])

/* This macro shoves a new element into the first position of \em{a},
   making sure that there is enough space, and wrapping indices as
   needed. */

#define array_prepend(a, elem, type) \
  do \
  { if(array_size(a) >= (a)->alloced) \
      __array_grow_internal(a); \
    ((type *)(a)->data)[((a)->alloced + (a)->origin - ++(a)->lindex) % \
                        (a)->alloced] = (elem); \
  } while (0)

/* This macro shoves a new element into the last position of \em{a},
   making sure that there is enough space, and wrapping indices as
   needed. */

#define array_append(a, elem, type) \
  do \
  { if (array_size(a) >= (a)->alloced) \
      __array_grow_internal(a); \
    ((type *)(a)->data)[((a)->alloced + (a)->origin + (a)->rindex++) % \
                        (a)->alloced] = (elem); \
  } while (0)

/* This macro will insert \em{elem} into the \em{index'th} position
   in the ARRAY \em{a} and allocate any space that is needed.  It
   calls a helper function to do the bulk of the work. */

#define array_insert_index(a, elem, type, index) \
  (((type *)(a)->hole)[0] = (type)(elem), __array_insert_index((a), (index)))

/* This macro removes the first element from the array and can be used
   as the right-hand side of an assignment to get the removed element,
   similar to a stack pop or a queue dequeue. */

#define array_remove_front(a, type) \
  (((a)->lindex > 0) ? \
   (((type *)(a)->data)[((a)->alloced + (a)->origin - (a)->lindex--) % \
                        (a)->alloced]) : \
   ((a)->rindex > 0 && (a)->origin < ((a)->alloced - 1)) ? \
   ((a)->rindex--, ((type *)(a)->data)[(a)->origin++]) : \
   ((a)->rindex > 0 && (a)->origin == ((a)->alloced - 1)) ? \
   ((a)->rindex--, (a)->origin = 0, ((type *)(a)->data)[(a)->alloced - 1]) : \
   (ulog(ULOG_ERROR, "array_remove_front: array is empty."), \
    ((type *)(a)->hole)[0]))

/* This macro removes the last element from the array and can be used
   as the right-hand side of an assignment to get the removed element,
   similar to a stack pop or a queue dequeue. */

#define array_remove_back(a, type) \
  (((a)->rindex > 0) ? \
   (((type *)(a)->data)[((a)->alloced + (a)->origin + --(a)->rindex) % \
                        (a)->alloced]) : \
   ((a)->lindex > 0 && (a)->origin > 0) ? \
   ((a)->lindex--, ((type *)(a)->data)[--(a)->origin]) : \
   ((a)->lindex > 0 && (a)->origin == 0) ? \
   ((a)->lindex--, (a)->origin = (a)->alloced - 1, ((type *)(a)->data)[0]) : \
   (ulog(ULOG_ERROR, "array_remove_back: array is empty."), \
    ((type *)(a)->hole)[0]))

/* This behaves similarly to \bf{array_destroy_index()} but it will
   return the element to destroyed as an R-value. */

#define array_remove_index(a, type, index) \
   (((type *)(a)->hole)[0] = array_fast_access(a, index, type), \
    array_destroy_index(a, index), ((type *)(a)->hole)[0])

/* This macro removes the first element similarly to
   \bf{array_remove_front()} but does not return a valid rvalue, thus
   it cannot be used to retrieve the first element.  This macro may be
   trivially faster than the "remove" version. */

#define array_destroy_front(a) \
  (((a)->lindex > 0) ? \
      ((a)->lindex--) : \
   ((a)->rindex > 0 && (a)->origin < ((a)->alloced - 1)) ? \
      ((a)->rindex--, (a)->origin++) : \
   ((a)->rindex > 0 && (a)->origin == ((a)->alloced - 1)) ? \
      ((a)->rindex--, (a)->origin = 0) : \
   (ulog(ULOG_ERROR, "array_destroy_front: array is empty.")))

/* This macro removes the last element similarly to
   \bf{array_remove_back()} but does not return to a valid rvalue, thus
   it cannot be used to retrieve the last element.  This macro may be
   trivially faster than the "remove" version. */

#define array_destroy_back(a) \
  (((a)->rindex > 0) ? \
      (--(a)->rindex) : \
   ((a)->lindex > 0 && (a)->origin > 0) ? \
      ((a)->lindex--, --(a)->origin) : \
   ((a)->lindex > 0 && (a)->origin == 0) ? \
      ((a)->lindex--, (a)->origin = (a)->alloced - 1) : \
   (ulog(ULOG_ERROR, "array_destroy_back: array is empty.")))


/* This macro will not free any memory, but it will set all of the
   array's internals to an empty state. */

#define array_clear(a)  ((a)->lindex = (a)->rindex = (a)->origin = 0)

/* Shrink the allocated array to the smallest amount possible
   considering the elements that are currently in use. */

#define array_shrink(a) \
  do \
  { char *__array_save = (a)->data; \
    (a)->data = array_to_pointer(a); \
    xfree(__array_save); \
    (a)->origin = (a)->lindex = 0; \
    (a)->rindex = (a)->alloced = array_size(a); \
  } while (0)

/* This macro expression is identical to a call to \bf{array_append()}
   except that it is much faster.  The down side is that you can only
   use this call by itself and with calls to \bf{array_pop()}.  If
   you mix these calls with any of the other array insertion and 
   deletion routines we guarantee that your program will crash.
   If you just need a stack module then this the ideal thing for
   you to use. */

#define array_push(a, elem, type) \
  (((array_size(a) >= (a)->alloced) ? \
    __array_grow_internal(a) : 0), \
   (((type *)(a)->data)[(a)->rindex++] = (elem)))\


/* This is the counterpart to \bf{array_push()}.  Underflow is not
   checked, so let the buyer beware. */

#define array_pop(a, type) \
  (((type *)(a)->data)[--(a)->rindex])


/* This macro returns the address of the first element of the array.
   Use this with caution. */

#define array_first_addr(a, type) \
  (&(array_fast_access((a), 0, type)))

/* This macro returns the address of the last element of the array.
   Use this with caution. If result of this macro is greater than 
   \bf{array_first_addr()} then you are guaranteed that the array
   does not "wrap" around. */

#define array_last_addr(a, type) \
  (&(array_fast_access((a), array_size(a) - 1, type)))

/* h2man:ignore */
void __array_insert_index(ARRAY *a, unsigned index);


#ifdef __cplusplus
}
#endif /* __cplusplus */

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#endif /* __ARRAY_H__ */




