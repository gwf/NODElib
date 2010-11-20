
/* Copyright (c) 1995 by G. W. Flake.
 *
 * NAME
 *   xmalloc.h - smarter memory allocation
 * SYNOPSIS
 *   This module provides memory allocation with automatic error
 *   checking.  It also keeps track of the memory used by each
 *   allocated pointer and the total used for a process.  It supports
 *   debugging with checks for valid frees and gives a summary of
 *   outstanding pointers.
 * DESCRIPTION
 *   Must of the routines in \bf{xalloc}(3) work identically
 *   To the corresponding routines in \bf{malloc}(3) with some
 *   minor differences.  The first difference is that failure to
 *   get memory is consider a fatal error for the \bf{xalloc}(3)
 *   routines.  If no memory is available \bf{ulog()} is
 *   called with an appropriate message.  Therefore, you never need
 *   to check for a \bf{NULL} return value.
 *
 *   The second difference is that the \bf{xalloc}(3) routines
 *   can keep track of the size of a pointer's memory on a pointer
 *   by pointer basis.  Therefore, at any future time you can
 *   determine the size of any memory segment that was allocated
 *   with the \bf{xalloc}(3) routines.
 *
 *   Note that you \bf{cannot} mix calls between the
 *   \bf{xalloc}(3) routines and the \bf{malloc}(3) routines:
 *   if you get memory with \bf{xmalloc()} it must be freed
 *   with \bf{xfree()}, not \bf{free()}.
 *
 *   More extensive debugging features are summarized after this section.
 * DEBUGGING ROUTINES
 *   If you define \bf{XALLOC_DEBUG} before you include \bf{xalloc.h}
 *   then all of the routines will be aliased to debugging versions.
 *   Each pointer will be stored in a hash table which contains
 *   information such as the size, file, and line location of the calls.
 *   At any time you can call \bf{xalloc_report()} to get a complete
 *   summary of all pointers.  Moreover, the freeing and reallocating
 *   routines will check to make sure that the supplied pointers are
 *   valid.
 *
 *   Because most program source is divided into several files, and
 *   memory may be allocated in one file but deallocated in another,
 *   you should normally set \bf{XALLOC_DEBUG} to either true or
 *   false for every source file in your program.  Otherwise, you
 *   could potentially have a pointers allocated with a debugging
 *   routine but freed with a non-debugging routine.  Don't do this.
 * AUTHOR
 *   Gary William Flake (\url{\bf{gary.flake@usa.net}}{mailto:gary.flake@usa.net}).
 * CREDITS
 *   Credit for some implementation ideas goes to Chris Torek and
 *   Jos Horsmeier.
 * SEE ALSO
 *   \bf{brk}(2), \bf{malloc}(3), \bf{ulog}(3).
 */


#ifndef __XALLOC_H__
#define __XALLOC_H__

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* I am including the header file below for the size_t definition.
   This should really be <stddef.h> but gcc has its own version of
   that file which does not #ifdef definitions.  Thus, I keep getting
   redefinition error for NULL.  Fortunately, <stdio.h> also defines
   size_t, but your mileage may vary. */

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include "nodelib/etc/version.h"
#include "nodelib/etc/options.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* These are the drop in replacements for the standard memory
   functions */

/* Allocates \em{sz} bytes and returns a pointer to the allocated memory.
   The memory is not cleared. */

void *xmalloc(size_t sz);

/* Allocates memory for an array of \em{num} elements with each element
   consisting of \em{sz} bytes and returns a pointer to the memory.
   The memory is cleared to zero. */

void *xcalloc(size_t num, size_t sz);

/* \bf{xrealloc()} changes the size of the memory block pointed to by
   \em{ptr} to \em{sz} bytes.  The contents will be unchanged to the
   minimum of the old and new sizes; newly allocated memory will be
   uninitialized.  If \em{ptr} is \bf{NULL}, the call is equivalent
   to \bf{xmalloc(}\em{sz}\bf{)}; if \em{sz} is equal to zero, the call
   is equivalent to \bf{xfree(}\em{ptr}\bf{)}.  Unless \em{ptr} is
   \bf{NULL}, it must have been returned by an earlier call to
   \bf{xmalloc()}, \bf{xcalloc()}, or \bf{xrealloc()}. */

void *xrealloc(void *ptr, size_t sz);

/* \bf{xrecalloc()} has no counterpart in the \bf{malloc}(3) routines,
   yet the author swears that he has found a legitimate use for it.
   \bf{xrecalloc()} behaves exactly like \bf{xrealloc()} except the
   arguments \em{num} and \em{sz} have the same meaning as in 
   \bf{xcalloc()}. */

void *xrecalloc(void *ptr, size_t num, size_t sz);

/* Frees the memory pointed to by \em{ptr} which must have been 
   allocated by \bf{xmalloc()} or \bf{xrealloc()}. */

void  xfree(void *ptr);

/* Frees the memory pointed to by \em{ptr} which must have been 
   allocated by \bf{xcalloc()} or \bf{xrecalloc()}. */

void  xcfree(void *ptr);


/* Like strdup() but uses xmalloc(). */

char *xstrdup(char *str);


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* Global declarations */

/* h2man:include The external int \bf{xalloc_fail_on_null} is set to
   1 by default, which means that all routines which return memory will
   fail with an appropriate error message and ultimately halt the program
   by calling \bf{xfatal()}.  You can prevent the halt by setting
   \bf{xalloc_fail_on_null} to 0. */


/* h2man:skipbeg */

#ifdef OWNER
#define ISOWNER(x) x
#define NOTOWNER(x)
#else
#define ISOWNER(x)
#define NOTOWNER(x) x
#endif

NOTOWNER(extern) int xalloc_fail_on_null ISOWNER(= 1);

#undef OWNER
#undef ISOWNER
#undef NOTOWNER

/* h2man:skipend */

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* Some query and book-keeping functions. */

/* Returns the number of bytes in the memory pointed to by \em{ptr},
   which must have been returned by one of the above \bf{xalloc}(3)
   routines.  Due to some programming trickery, \bf{xmemsize()} is
   a constant time operation, i.e. you can use it as often as you 
   wish with almost no additional overhead. */

size_t xmemsize(void *ptr);

/* Returns the total number of outstanding bytes used by all of the
   memory returned by the \bf{xalloc}(3) routines. */

size_t xmemused(void);

/* This function gives a summary report of each outstanding pointer
   that has not been freed. Some sample output:
   \begin{verbatim}
   -------------
          total xmalloc()  & xcalloc()   : 512
          total xrealloc() & xrecalloc() : 256
          total xfree()    & xcfree()    : 1024
   
   ID     FILE      LINE    SOURCE       ADDRESS         SIZE
   --     ----      ----    ------       -------         ----
   278    foo.c     623     xmalloc      0x12345678      256
   324    foo.c     627     xmalloc      0xabcd1234      256
   745    bar.c     118     xrealloc     0x1234abcd      256
   \end{verbatim}

   The summary states that there are three outstanding pointers.  Most of
   the information is self explanatory; however, the \bf{ID} field
   gives some indication as to when a pointer was originally allocated as
   id's are simply assigned by adding 1 to the last id assigned, with
   the first id being 0. */

void   xalloc_report(void);

/* This routine does something only if you have been using the debugging
   \bf{xalloc}(3) routines, in which case it will free up internal
   tables used to keep track of outstanding pointers. */      

void   xalloc_shutdown(void);

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* These are the debugging memory functions which should never be called
   directly by the user. */

/* h2man:skipbeg */
void *__xmalloc(char *, int, size_t);
void *__xcalloc(char *, int, size_t, size_t);
void *__xrecalloc(char *, int, void *, size_t, size_t);
void *__xrealloc(char *, int, void *, size_t);
void  __xcfree(char *, int, void *);
void  __xfree(char *, int, void *);
/* h2man:skipend */

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* The macros with the single "_" prepended are always hooked to
   the debugging versions.  They supply the file and line number
   of the caller to the debugging versions. */

/* h2man:skipbeg */
#define _xmalloc(SZ) \
  __xmalloc(__FILE__, __LINE__, SZ)
#define _xcalloc(NUM, SZ) \
  __xcalloc(__FILE__, __LINE__, NUM, SZ)
#define _xrealloc(PTR, SZ) \
  __xrealloc(__FILE__, __LINE__, PTR, SZ)
#define _xrecalloc(PTR, NUM, SZ) \
  __xrecalloc(__FILE__, __LINE__, PTR, NUM, SZ)
#define _xfree(PTR) \
  __xfree(__FILE__, __LINE__, PTR)
#define _xcfree(PTR) \
  __xcfree(__FILE__, __LINE__, PTR)
/* h2man:skipend */

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* If our little friend below is defined then let the normal names
   be synonyms for the debugging versions. */

#ifdef  XALLOC_DEBUG

/* h2man:skipbeg */
#define xmalloc(SZ) \
  __xmalloc(__FILE__, __LINE__, SZ)
#define xcalloc(NUM, SZ) \
  __xcalloc(__FILE__, __LINE__, NUM, SZ)
#define xrealloc(PTR, SZ) \
  __xrealloc(__FILE__, __LINE__, PTR, SZ)
#define xrecalloc(PTR, NUM, SZ) \
  __xrecalloc(__FILE__, __LINE__, PTR, NUM, SZ)
#define xfree(PTR) \
  __xfree(__FILE__, __LINE__, PTR)
#define xcfree(PTR) \
  __xcfree(__FILE__, __LINE__, PTR)
/* h2man:skipend */

#endif /*  XALLOC_DEBUG */

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* The value of XALLOC_MAGIC needs to be a number which is greater or
   equal to sizeof(size_t) and a multiple of the strictest alignment
   on your machine.  This is usually sizeof(double).

   If you define XALLOC_MAGIC then I'll use that.  If you that's not
   the case and you define XALLOC_GUESS_MAGIC then I'll always guess
   that the right size is sizeof(double).  Otherwise, I'll compute
   it the hard way. */

#ifndef XALLOC_MAGIC
#ifndef XALLOC_GUESS_MAGIC

/* h2man:skipbeg */

#define __alignment(TYPE) \
  ((long)((char *)&((struct{char x; TYPE y;}*)0)->y - (char *)0))

#define __max(X, Y) \
  (((X) > (Y)) ? (X) : (Y))

typedef int __function ();

/* Monster comparison:  sorry if this hoses your preprocessor.
   Fortunately, this macro is only called once in xalloc.c */

#define XALLOC_MAX_ALIGN \
  (__max(__max(__max(__alignment(long), __alignment(double)), \
               __max(__alignment(long double), __alignment(char *))), \
         __alignment(__function *)))

/* h2man:skipend */

/* I am assuming that all alignments and sizes are powers of 2.  This
   should be safe since anything else is insane.  With this assumption
   we can set XALLOC_MAGIC to the max of XALLOC_MAX_ALIGN and
   sizeof(size_t) since if the former is greater than the latter our
   requirements our met.  Otherwise sizeof(size_t) is greatest, and
   since it is a power of 2 it will also be a multiple of the strictest
   alignment, which is also cool. */

#define XALLOC_MAGIC __max(XALLOC_MAX_ALIGN, sizeof(size_t))

#else /* XALLOC_GUESS_MAGIC */


/* Just pick the obvious choice. */

#define XALLOC_MAGIC sizeof(double)

#endif /* XALLOC_GUESS_MAGIC */
#endif /* XALLOC_MAGIC */

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /*  __XALLOC_H__ */
