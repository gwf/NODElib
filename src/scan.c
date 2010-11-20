
/* Copyright (c) 1995 by G. W. Flake. */

#include <string.h>
#include "nodelib/scan.h"

#define SCANLINELEN (16*1024)

static char *def_delimiters = "`~!@#$%^&*()-=+[]{};:'\",.<>\\/?|";
static char *def_whites = " \t\n";
static char *def_comments = "#";

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

SCAN *scan_create(int type, void *src)
{
  SCAN *s;

  s = xmalloc(sizeof(SCAN));
  s->delims = def_delimiters;
  s->whites = def_whites;
  s->comments = def_comments;
  if(type) {
    s->fp = src;
    s->buffer = xmalloc(SCANLINELEN * sizeof(char));
    s->ptr = NULL;
  }
  else {
    s->fp = NULL;
    s->buffer = src;
    s->ptr = src;
  }
  s->token = array_create(64, char);
  return(s);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

SCAN *scan_recreate(SCAN *s, int type, void *src)
{
  if(!s)
    return(scan_create(type, src));
  if(s->fp && !type)
    xfree(s->buffer);
  else if(!s->fp && type)
    s->buffer = xmalloc(SCANLINELEN * sizeof(char));
  if(type) {
    s->fp = src;
    s->ptr = NULL;
  }
  else {
    s->fp = NULL;
    s->buffer = src;
    s->ptr = src;
  }
  array_clear(s->token);
  return(s);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void scan_destroy(SCAN *s)
{
  if(s->fp)
    xfree(s->buffer);
  array_destroy(s->token);
  xfree(s);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void scan_flush(SCAN *s)
{
  if(s->fp)
    s->ptr = NULL;
  else {
    while(*s->ptr && *s->ptr != '\n')
      s->ptr++;
    if(*s->ptr)
      s->ptr++;
  }
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static char *scan_get_or_peek(SCAN *s, int get_flag)
{
  char *save;
 
  array_clear(s->token);
  while(1) {
    /* Get a new line if we are scaning a file and only if necessary.
     * The four conditions below are:
     *   1) this is a new line or a flush has been requested;
     *   2) we are looking at a comment character;
     *   3) end of line; or
     *   4) end of line (probably end of file as well)
     */    
    if(s->fp && (s->ptr == NULL || strchr(s->comments, *s->ptr) ||
		 *s->ptr == 0)) {

      /* Read a line.  If it's EOF then return an empty string */
      if((s->ptr = fgets(s->buffer, SCANLINELEN, s->fp)) == NULL) {
	array_push(s->token, 0, char);
	return(NULL);
      }

      /* Loop until we no longer have to get more lines. */
      continue;
    }
    else if(!s->fp && *s->ptr == 0) {
      array_push(s->token, 0, char);
      return(NULL);
    }
    else if(!s->fp && strchr(s->comments, *s->ptr)) {
      while(*s->ptr && *s->ptr != '\n')
	s->ptr++;
      /*s->ptr++;*/
      continue;
    }

    /* Skip white space. */
    while(*s->ptr && strchr(s->whites, *s->ptr))
      s->ptr++;

    /* Skip EOL and comments. */
    /*if(strchr(s->comments, *s->ptr) || *s->ptr == '\n' || *s->ptr == 0)*/
    if(strchr(s->comments, *s->ptr) || *s->ptr == 0)
      continue;

    /* Look for delimiters. */
    if(strchr(s->delims, *s->ptr)) {
      array_push(s->token, *s->ptr, char);
      array_push(s->token, 0, char);
      
      if(get_flag)
	s->ptr++;
      return(array_first_addr(s->token, char));
    }

    /* As long as the current characters is not a delimiter, white space,
     * comment, nor end of line, then increment the ptr.
     */
    save = s->ptr;
    /* while(*s->ptr && *s->ptr != '\n' && !strchr(s->delims, *s->ptr) 
	  && !strchr(s->whites, *s->ptr) && !strchr(s->comments, *s->ptr)) */
    while(*s->ptr && !strchr(s->delims, *s->ptr) 
	  && !strchr(s->whites, *s->ptr) && !strchr(s->comments, *s->ptr))
      s->ptr++;
    
    /* Copy everything between save and ptr into token. */
    array_append_ptr(s->token, save, s->ptr - save);
    array_append(s->token, 0, char);

    /* If this is a peek then resore the pointer. */
    if(!get_flag)
      s->ptr = save;
    return(array_first_addr(s->token, char));
} }

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

char *scan_get(SCAN *s)
{
  return(scan_get_or_peek(s, 1));
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

char *scan_peek(SCAN *s)
{
  return(scan_get_or_peek(s, 0));
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
