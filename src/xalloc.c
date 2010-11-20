
/* Copyright (c) 1995 by G. W. Flake. */

#include <stdio.h>
#include <string.h>
/*#include <malloc.h>*/
#include <memory.h>

#undef XALLOC_DEBUG

#define OWNER

#ifdef WIN32                /* modified WEB */
#define XALLOC_GUESS_MAGIC
#endif

#include "nodelib/xalloc.h"
#undef  OWNER

#include "nodelib/ulog.h"

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* To keep track of all the memory in use. */
static size_t xalloc_total_used = 0;

/* This is so that we only call the monster macro once. */

const size_t xalloc_magic = XALLOC_MAGIC;

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void *xmalloc(size_t size)
{
  size_t *ptr;

  /* Make room to keep the size. */ 
  size += xalloc_magic;

  /* Get the memory. */
  if(!(ptr = malloc(size)))
    ulog(ULOG_FATAL, "xmalloc: failed to get %ld bytes.", (long)size);

  /* Assign the size of the segment, and keep a running total. */
  *ptr = size;
  xalloc_total_used += size;

  /* Reset the pointer to the user memory. */
  ptr = (size_t *)((char *)ptr + xalloc_magic);  
  return(ptr);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void *xrealloc(void *old_ptr, size_t size)
{
  size_t *new_ptr;

  /* If the old pointer is NULL then just do a standard malloc. */
  if(!old_ptr)
    return(xmalloc(size));

  /* If the new size request is zero then just free the old memory. */
  if(!size) {
    xfree(old_ptr);
    return(NULL);
  }

  /* Get back the pointer that was returned by either malloc() or realloc() */
  old_ptr = ((char *)old_ptr - xalloc_magic);

  /* Remove the old segment from the running total. */
  xalloc_total_used -= *((size_t *)old_ptr);

  /* Make room to keep the size. */
  size += xalloc_magic;

  /* Update the running total. */
  xalloc_total_used += size;

  /* Get the memory. */
  if(!(new_ptr = realloc(old_ptr, size)))
    ulog(ULOG_FATAL, "xrealloc: failed to get %ld bytes.", (long)size);

  /* Assign the size of the segment, and keep a running total. */
  *new_ptr = size;

  /* Reset the pointer to the user memory. */
  new_ptr = (size_t *)((char *)new_ptr + xalloc_magic);
  return((void *)new_ptr);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void xfree(void *ptr)
{
  if(ptr) {
    /* Move the pointer back to its real origin. */
    ptr = ((char *)ptr - xalloc_magic);

    /* Remove from the running total, and free it. */
    xalloc_total_used -= *((size_t *)ptr);
    free(ptr);
  }
  else
    ulog(ULOG_WARN, "xfree: passed a NULL pointer.");
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* xcalloc and friends simply call the normal version plus a memset
   if necessary. */

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void *xcalloc(size_t nelem, size_t elemsz)
{
  void *ptr;

  ptr = xmalloc(nelem * elemsz);
  (void)memset(ptr, 0, nelem * elemsz);
  return(ptr);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void *xrecalloc(void *ptr, size_t nelem, size_t elemsz)
{
  ptr = xrealloc(ptr, nelem * elemsz);
  (void)memset(ptr, 0, nelem * elemsz);
  return(ptr);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void xcfree(void *ptr)
{
  xfree(ptr);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

size_t xmemsize(void *ptr)
{
  /* Take the pointer, shift it xalloc_magic bytes to the left, cast
   * the value to a size_t, and subtract the xalloc_magic bytes from
   * the value (which is used to store the size) to get the size of
   * the requested segment.
   */
  return(*((size_t *)((char *)ptr - xalloc_magic)) - xalloc_magic);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

size_t xmemused(void)
{
  return(xalloc_total_used);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static int xalloc_initialized = 0;
static int xalloc_numalloc = 0;
static int xalloc_numrealloc = 0;
static int xalloc_numfree = 0;
static int xalloc_id = 0;

static char *xmalloc_name   = "malloc";
static char *xrealloc_name  = "realloc";
static char *xcalloc_name   = "calloc";
static char *xrecalloc_name = "recalloc";

typedef struct
{ char   *file;
  int     line;
  int     id;
  char   *name;
  void   *ptr;
  size_t  size;
} XALLOC_REC;

#include "nodelib/hash.h"

static HASH *xalloc_hash = NULL;

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* A function to turn a XALLOC_REC * into a unique integer. */

static unsigned long xalloc_numify(const void *ptr, void *obj)
{
  return((int)((XALLOC_REC *)ptr)->ptr);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* A function to compare two XALLOC_RECs */

static int xalloc_compare(const void *ptr1, const void *ptr2, void *obj)
{
  long i, j;

  i = (long)((XALLOC_REC *)ptr1)->ptr;
  j = (long)((XALLOC_REC *)ptr2)->ptr;
  return(i < j ? -1 : i == j ? 0 : 1);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static XALLOC_REC *xar_create(char *file, int line, int id, char *name,
                              void *ptr, size_t size)
{
  XALLOC_REC *rec;

  rec = xmalloc(sizeof(XALLOC_REC));
  rec->file = file;
  rec->line = line;
  rec->id = id;
  rec->name = name;
  rec->ptr = ptr;
  rec->size = size;
  return(rec);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static void xar_destroy(XALLOC_REC *rec)
{
  xfree(rec);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static void xalloc_init(void)
{
  if(xalloc_initialized)
    return;
  xalloc_initialized = 1;
  xalloc_hash = hash_create(1024, xalloc_numify, xalloc_compare, NULL);
}
  
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void xalloc_finish(void)
{
  XALLOC_REC *rec;
  HASH_NODE  *node  = NULL;
  int         index = 0;
  
  if(!xalloc_initialized)
    return;
  xalloc_initialized = 0;
  while((rec = hash_iterate(xalloc_hash, &index, &node)) != NULL) {
    hash_delete(xalloc_hash, rec);
    xar_destroy(rec);
  }
  hash_destroy(xalloc_hash);
  xalloc_hash = NULL;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* The debugging version simply call the non-debugging version while
   doing some book-keeping.  If the call is a xmalloc() or xcalloc()
   we simply call the real function and store the record for this
   call into the hash table.

   If the call is a xrealloc() or xrecalloc() we remove the old record
   for that pointer, call the appropriate function, and store a new
   record.

   The functions xfree() and cfree() will remove the records and call
   the normal version. */

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void *__xmalloc(char *file, int line, size_t size)
{
  XALLOC_REC *rec;
  void       *ptr;
  
  if(!xalloc_initialized)
    xalloc_init();
  ptr = xmalloc(size);
  xalloc_numalloc++, xalloc_id++;
  rec = xar_create(file, line, xalloc_id, xmalloc_name, ptr, size);
  hash_insert(xalloc_hash, rec);
  return(ptr);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void *__xcalloc(char *file, int line, size_t nelem, size_t elemsz)
{
  XALLOC_REC *rec;
  void       *ptr;
  
  if(!xalloc_initialized)
    xalloc_init();
  ptr = xcalloc(nelem, elemsz);
  xalloc_numalloc++, xalloc_id++;
  rec = xar_create(file, line, xalloc_id, xcalloc_name, ptr, nelem * elemsz);
  hash_insert(xalloc_hash, rec);
  return(ptr);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void *__xrealloc(char *file, int line, void * ptr, size_t size)
{
  XALLOC_REC *rec, srch;
  
  if(!xalloc_initialized)
    xalloc_init();
  if(ptr) {
    srch.ptr = ptr;
    if((rec = hash_search(xalloc_hash, &srch)) == NULL) {
      ulog(ULOG_INFO, "(%s, %d): xrealloc: attempted to realloc "
	   "from bad pointer.\n", file, line);
      return(NULL);
    }
    hash_delete(xalloc_hash, rec);
    xar_destroy(rec);
  }
  ptr = xrealloc(ptr, size);
  xalloc_numrealloc++, xalloc_id++;
  rec = xar_create(file, line, xalloc_id, xrealloc_name, ptr, size);
  hash_insert(xalloc_hash, rec);
  return(ptr);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void *__xrecalloc(char *file, int line, void *ptr, size_t nelem,
                  size_t elemsz)
{
  XALLOC_REC *rec, srch;
  
  if(!xalloc_initialized)
    xalloc_init();
  if(ptr) {
    srch.ptr = ptr;
    if((rec = hash_search(xalloc_hash, &srch)) == NULL) {
      ulog(ULOG_INFO, "(%s, %d): xrecalloc: attempted to recalloc "
	   "from bad pointer.\n", file, line);
      return(NULL);
    }
    hash_delete(xalloc_hash, rec);
    xar_destroy(rec);
  }
  ptr = xrecalloc(ptr, nelem, elemsz);
  xalloc_numrealloc++, xalloc_id++;
  rec = xar_create(file, line, xalloc_id, xrecalloc_name, ptr, nelem * elemsz);
  hash_insert(xalloc_hash, rec);
  return(ptr);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void __xfree(char *file, int line, void *ptr)
{
  XALLOC_REC *rec, srch;
  
  if(!xalloc_initialized)
    xalloc_init();
  if(ptr) {
    srch.ptr = ptr;
    if((rec = hash_search(xalloc_hash, &srch)) == NULL) {
      ulog(ULOG_INFO, "(%s, %d): xfree: attempted to free bad pointer.\n",
	   file, line);
      return;
    }
    hash_delete(xalloc_hash, rec);
    xar_destroy(rec);
  }
  else {
    ulog(ULOG_INFO, "(%s, %d): xfree: attempted to free NULL pointer.\n",
	   file, line);
    return;
  }
  xalloc_numfree++;
  xfree(ptr);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void __xcfree(char *file, int line, void *ptr)
{
  XALLOC_REC *rec, srch;
  
  if(!xalloc_initialized)
    xalloc_init();
  if(ptr) {
    srch.ptr = ptr;
    if((rec = hash_search(xalloc_hash, &srch)) == NULL) {
      ulog(ULOG_INFO, "(%s, %d): xcfree: attempted to free bad pointer.\n",
	   file, line);
      return;
    }
    hash_delete(xalloc_hash, rec);
    xar_destroy(rec);
  }
  else {
    ulog(ULOG_INFO, "(%s, %d): xfree: attempted to free NULL pointer.\n",
	 file, line);
    return;
  }
  xalloc_numfree++;
  xcfree(ptr);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

char *xstrdup(char *str)
{
  char *s;
  unsigned n;

  n = strlen(str);
  s = xmalloc(sizeof(char) * (n + 1));
  strcpy(s, str);
  return(s);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/*  Sample output for positioning...

    ID      FILE            LINE    SOURCE          ADDRESS         SIZE
    0000000000111111111122222222223333333333444444444455555555556666666666
    0123456789012345678901234567890123456789012345678901234567890123456789

*/

#define LEN 256

void xalloc_report(void)
{
  XALLOC_REC *rec;
  HASH_NODE *node = NULL;
  int index = 0, i;
  char line[LEN], *l;

  struct info
  { int pos; char *fmt; int offset;
  } pinfo[] =  { {  8, "%d",   offsetof(XALLOC_REC, id) },
                 { 24, "%s",   offsetof(XALLOC_REC, file) },
                 { 32, "%d",   offsetof(XALLOC_REC, line) },
                 { 48, "%s",   offsetof(XALLOC_REC, name) },
                 { 64, "0x%x", offsetof(XALLOC_REC, ptr) },
                 {  0, "%d",   offsetof(XALLOC_REC, size) }};
  

  ulog(ULOG_INFO, "xalloc-report: ");
  if(!xalloc_initialized) {
    ulog(ULOG_INFO, "nothing to report.\n");
    return;
  }
  ulog(ULOG_INFO, "\n-------------\n");
  ulog(ULOG_INFO, "\ttotal xmalloc()  & xcalloc()   : %d\n",
          xalloc_numalloc);
  ulog(ULOG_INFO, "\ttotal xrealloc() & xrecalloc() : %d\n",
          xalloc_numrealloc);
  ulog(ULOG_INFO, "\ttotal xfree()    & xcfree()    : %d\n\n",
          xalloc_numfree);
  if(hash_iterate(xalloc_hash, &index, &node)) {
    node = NULL, index = 0;
    ulog(ULOG_INFO, "ID\tFILE\t\tLINE\tSOURCE\t\tADDRESS\t\tSIZE\n");
    ulog(ULOG_INFO, "--\t----\t\t----\t------\t\t-------\t\t----\n");
    while((rec = hash_iterate(xalloc_hash, &index, &node)) != NULL) {
      l = line;
      for(i = 0; i < sizeof(pinfo) / sizeof(struct info); i++) {
	sprintf(l, pinfo[i].fmt, *(int *)((char *)rec + pinfo[i].offset));
        l = l + strlen(l);
        while(l < line + pinfo[i].pos) *l++ = ' ';
      }
      ulog(ULOG_INFO, "%s\n", line);
    }
  }
  ulog(ULOG_INFO, "\n\n");
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
