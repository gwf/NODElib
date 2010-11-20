
/* Copyright (c) 1995 by G. W. Flake. */

#define _GNU_SOURCE
#include <stdio.h>

#ifdef PTHREADS
#include <pthread.h>
#endif

#include <errno.h>

#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>

#ifdef SUNOS
#include <varargs.h>
char *vsprintf(char *, char *, va_list);
#endif

#define OWNER
#include "nodelib/ulog.h"
#undef OWNER

#include "nodelib/array.h"
#include "nodelib/misc.h"

#define ULOG_BUFF_SZ 1024

static unsigned ulog_line_num = 0;
static char *ulog_file_name = NULL;
static int ulog_msg_code = -1;

#ifdef PTHREADS
static pthread_mutex_t ulog_mutex;
#endif


static FILE *ulog_file_ptrs[ULOG_MSG_TYPES];

static void (*ulog_rd_hooks[ULOG_MSG_TYPES]) (char *str) = {
NULL, NULL, NULL, NULL, NULL, NULL, NULL};

ARRAY *ulog_sd_hooks = NULL;

typedef void (*AFKLUDGE) (void);

#ifdef PTHREADS
void ulog_mutex_init(void) {
  pthread_mutex_init(&ulog_mutex, NULL);
}
#endif 

#undef malloc
#undef free


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* The function below is mostly lifted from (roy@mchip00.med.nyu.edu) Roy
   Smith's errprint.c package. -gwf */


static char *expand_pct_m(char *format, int msgtype)
{
  const char *err;
  char *src, *dst, *buf, *prefix = "";
  int totsz, errsz, prefsz = 0;

  if (!(err = strerror(errno)))
    err = "unknown error";
  errsz = strlen(err);

  // get the prefix first.
  if (!ulog_no_prefix && msgtype != ULOG_WARN) {
    prefix = alloca(strlen(ulog_file_name) + 30 + 10);
    sprintf(prefix, "(%s, %d) : ", ulog_file_name, ulog_line_num);
    prefsz = strlen(prefix);
  }

  // now calculate the maximum space required and get it.
  totsz = prefsz + 10;
  for (src = format; *src; src++) {
    if (*src == '%') {
      src++;
      if (*src == 'm')
	totsz += errsz + 2;
      else if (*src == 't')
	totsz += prefsz + 2;
      else
	totsz += 2;;
    }
    else
      totsz++;
  }
  if ((buf = malloc(totsz)) == NULL) {
    fprintf(stderr, "ulog: could not malloc(%d)\n", totsz);
    exit(1);
  }

  // now fill it up
  strcpy(buf, prefix);
  dst = buf + prefsz;
  src = format;
  while (*src) {
    if (*src == '%') {
      if (*(src+1) == 'm') {
	strcpy(dst, err);
	dst += errsz;
	src += 2;
      }
      else if (*(src+1) == 't') {
	if (dst > buf && *(dst-1) != '\n')
	  *dst++ = '\n';
	memset(dst, ' ', prefsz);
	dst += prefsz;
	*(dst-2) = ':';
	src += 2;
      }
      else if (*(src+1) == '%') {
	*dst++ = *src++;
	*dst++ = *src++;
      }
      else
	*dst++ = *src++;      
    }
    else
      *dst++ = *src++;    
  }
  if (dst > buf && *(dst-1) != '\n')
    *dst++ = '\n';
  *dst++ = 0;
  return buf;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static void va_ulog_slave(int msgtype, char *format, va_list args)
{
  int i;
  void ulog_generic_slave(int, char *, ...);
  char *fmt;
  char *result, *ulog_msg, *ulog_fmt = NULL;
  static int init = 0;

  if (!init) {
    for (i = 0; i < ULOG_MSG_TYPES; i++) {
      if (ulog_file_ptrs[i]) continue;
      if (i == ULOG_PRINT)
	ulog_file_ptrs[i] = stdout;
      else
	ulog_file_ptrs[i] = stderr;
    }
    init = 1;
  }

  if (msgtype < 0 || msgtype >= ULOG_MSG_TYPES) {
    ulog_generic_slave(ULOG_ERROR, "ulog_slave: invalid message type: %d",
		       msgtype);
    return;
  }
  else if ((ulog_file_ptrs[msgtype] == NULL &&
	    ulog_rd_hooks[msgtype] == NULL) || msgtype < ulog_threshold) {
    return;
  }

  if (msgtype == ULOG_DEBUG || msgtype >= ULOG_WARN) {
    ulog_fmt = expand_pct_m(format, msgtype);
    fmt = ulog_fmt;
  }
  else
    fmt = format;
  vasprintf(&result, fmt, args);
  ulog_msg = result;
  if (ulog_fmt) free(ulog_fmt);
  
  if (ulog_file_ptrs[msgtype])
    fwrite(ulog_msg, 1, strlen(ulog_msg), ulog_file_ptrs[msgtype]);

  if (ulog_rd_hooks[msgtype])
    ulog_rd_hooks[msgtype](ulog_msg);

  if (msgtype == ULOG_FATAL) {
    int sz = (ulog_sd_hooks) ? array_size(ulog_sd_hooks) : 0;
    for (i = 0; i < sz; i++)
      if (array_fast_access(ulog_sd_hooks, i, AFKLUDGE))
	array_fast_access(ulog_sd_hooks, i, AFKLUDGE) ();
    exit(1);
  }
  else if (msgtype == ULOG_ABORT) {
    for (i = 0; i < ULOG_MSG_TYPES; i++)
      if (ulog_file_ptrs[i])
	fflush(ulog_file_ptrs[i]);
    if (!ulog_never_dump)
      abort();
    else
      exit(2);
  }
  xfree(result);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void ulog_alias_slave(char *format, ...)
{
  va_list args;

  va_start(args, format);
  va_ulog_slave(ulog_msg_code, format, args);
  va_end(args);
#ifdef PTHREADS
  pthread_mutex_unlock(&ulog_mutex);
#endif
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void (*__ulog_store_alias(unsigned line, char *file, int code)) (char *, ...) {
#ifdef PTHREADS
  pthread_mutex_lock(&ulog_mutex);
#endif
  ulog_line_num = line;
  ulog_file_name = simple_basename(file);
  ulog_msg_code = code;
  return ulog_alias_slave;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void ulog_generic_slave(int msgtype, char *format, ...)
{
  va_list args;
  int code = (ulog_msg_code == -1) ? msgtype : ulog_msg_code;

  va_start(args, format);
  va_ulog_slave(code, format, args);
  va_end(args);
#ifdef PTHREADS
  pthread_mutex_unlock(&ulog_mutex);
#endif
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void (*__ulog_store_generic(unsigned line, char *file, int code))
     (int, char *, ...) {
#ifdef PTHREADS
  pthread_mutex_lock(&ulog_mutex);
#endif
  ulog_line_num = line;
  ulog_file_name = simple_basename(file);
  ulog_msg_code = code;
  return ulog_generic_slave;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void ulog_redirect_fp(int msgtype, FILE * fp)
{
  if (msgtype < 0 || msgtype >= ULOG_MSG_TYPES) {
    ulog(ULOG_ERROR, "ulog_redirect_fp: invalid message type: %d", msgtype);
    return;
  }
  ulog_file_ptrs[msgtype] = fp;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void ulog_redirect_hook(int msgtype, void (*func) (char *str))
{
  if (msgtype < 0 || msgtype >= ULOG_MSG_TYPES) {
    ulog(ULOG_ERROR, "ulog_redirect_hook: invalid message type: %d", msgtype);
    return;
  }
  ulog_rd_hooks[msgtype] = func;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void ulog_add_shutdown_hook(void (*func) (void))
{
  int i, sz;

  if (ulog_sd_hooks == NULL) {
    ulog_sd_hooks = array_create(4, AFKLUDGE);
  }
  sz = array_size(ulog_sd_hooks);
  for (i = 0; i < sz; i++)
    if (array_fast_access(ulog_sd_hooks, i, AFKLUDGE) == NULL) {
      array_fast_access(ulog_sd_hooks, i, AFKLUDGE) = func;
      return;
    }
  array_push(ulog_sd_hooks, func, AFKLUDGE);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void ulog_remove_shutdown_hook(void (*func) (void))
{
  int i, sz;

  if (ulog_sd_hooks == NULL) {
    ulog(ULOG_ERROR, "ulog_remove_shutdown: no hooks to remove.");
    return;
  }
  sz = array_size(ulog_sd_hooks);
  for (i = 0; i < sz; i++)
    if (array_fast_access(ulog_sd_hooks, i, AFKLUDGE) == func) {
      array_fast_access(ulog_sd_hooks, i, AFKLUDGE) = NULL;
      return;
    }
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
