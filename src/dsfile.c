
/* Copyright (c) 1996 by G. W. Flake. */

#include "nodelib/dsfile.h"
#include "nodelib/xalloc.h"
#include "nodelib/ulog.h"

#include <string.h>

#include <stdio.h>
#include <sys/stat.h>
#ifndef WIN32
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#endif

#ifdef SUNOS
#include <sys/mman.h>
int munmap(caddr_t, int);
#endif

static DSM_FILE default_dsm_file = {
  1, 1, 1, 1, 1, 1, 0, NULL, NULL, NULL, SL_NULL,
  NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, NULL
};

int dsm_file_force_stdio = 0;

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

DSM_FILE *dsm_file(char *fname)
{
  DSM_FILE *dsmf;
  struct stat fpstat;
  FILE *fp;
#ifndef WIN32
  int fd;
#endif

  if(stat(fname, &fpstat)) {
    ulog(ULOG_WARN, "dsm_file: bad file '%s': %m.", fname);
    return(NULL);
  }
  dsmf = xmalloc(sizeof(DSM_FILE));
  *dsmf = default_dsm_file;
  dsmf->size = fpstat.st_size;

#ifndef WIN32
#ifndef MAP_FAILED
#define MAP_FAILED ((void *)-1)
#endif

  /* Stdio and low-level I/O are a deadly combination, so I am bending
   * over backwards trying to not to mix the two.
   */

  if(!dsm_file_force_stdio) {
    if((fd = open(fname, O_RDONLY, 0)) < 0) {
      ulog(ULOG_WARN, "dsm_file: unable to open '%s': %m.", fname);
      xfree(dsmf);
      return(NULL);
    }
    if((dsmf->mmapptr = mmap(0, fpstat.st_size, PROT_READ,
			     MAP_SHARED, fd, 0)) == MAP_FAILED) {
      ulog(ULOG_WARN, "dsm_file: unable to mmap: %m.");
      dsmf->mmapptr = NULL;
    }
    else {
      close(fd);
      return(dsmf);
    }
  }

#undef MAP_FAILED
#endif

#ifdef WIN32
  if((fp = fopen(fname, "rb")) == NULL) {
#else
  if((fp = fopen(fname, "r")) == NULL) {
#endif
    ulog(ULOG_WARN, "dsm_file: unable to open '%s': %m.", fname);
    xfree(dsmf);
    return(NULL);
  }
  dsmf->fp = fp;
  return(dsmf);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

int dsm_file_initiate(DSM_FILE *dsmf)
{
  if(dsmf->fp && dsmf->xbuf) xfree(dsmf->xbuf);
  if(dsmf->fp && dsmf->ybuf) xfree(dsmf->ybuf);
  if(dsmf->xdata) xfree(dsmf->xdata);
  if(dsmf->ydata) xfree(dsmf->ydata);

  dsmf->patsz = dsmf->x_read_width + dsmf->offset + dsmf->y_read_width;
  if(dsmf->size < dsmf->patsz + dsmf->skip)
    dsmf->numpat = 0;
  else
    dsmf->numpat = (dsmf->size - dsmf->patsz - dsmf->skip) / dsmf->step + 1;

  if(dsmf->fp) {
    dsmf->xbuf = xmalloc(sizeof(char) * dsmf->x_read_width);
    dsmf->ybuf = xmalloc(sizeof(char) * dsmf->y_read_width);
  }
  else {
    dsmf->xbuf = NULL;
    dsmf->ybuf = NULL;
  }

  dsmf->xdata = xmalloc(sizeof(double) * dsmf->x_width);
  dsmf->ydata = xmalloc(sizeof(double) * dsmf->y_width);

  if(dsmf->fp) fseek(dsmf->fp, dsmf->skip + 0L, SEEK_SET);
  dsmf->initp = 1;
  return(0);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void dsm_destroy_file(DSM_FILE *dsmf)
{
  if(dsmf->fp) fclose(dsmf->fp);
#ifndef WIN32
  else munmap(dsmf->mmapptr, dsmf->size);
#endif
  if(dsmf->fp && dsmf->xbuf) xfree(dsmf->xbuf);
  if(dsmf->fp && dsmf->ybuf) xfree(dsmf->ybuf);
  if(dsmf->xdata) xfree(dsmf->xdata);
  if(dsmf->ydata) xfree(dsmf->ydata);
  xfree(dsmf);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* The following code segment is used very many times in the function
   below.  So I am making it one big define to save some typing. */

#define ELSE_IF_DATA_IS(_TYPE, _TYPE_ENUM)                         \
  else if((dsmf->type == _TYPE_ENUM) &&                            \
	  (dsmf->x_read_width == dsmf->x_width * sizeof(_TYPE))) { \
    src = dsmf->xbuf;                                              \
    dst = dsmf->xdata;                                             \
    for(i = 0; i < dsmf->x_width; i++) {                           \
      *dst = *((_TYPE *)src);                                      \
      dst++;                                                       \
      src += sizeof(_TYPE);                                        \
    }                                                              \
  }

double *dsm_file_x(void *instance, unsigned index)
{
  char *src;
  double *dst;
  unsigned i;
  DSM_FILE *dsmf = instance;

  if(index >= dsmf->numpat) return(NULL);

  if(dsmf->fp) {
    if(fseek(dsmf->fp, (long)(dsmf->skip + dsmf->step * index),
	     SEEK_SET))
      goto IO_ERROR;
    if(fread(dsmf->xbuf, 1, dsmf->x_read_width, dsmf->fp) !=
       dsmf->x_read_width)
      goto IO_ERROR;
  }
  else {
    dsmf->xbuf = dsmf->mmapptr + dsmf->step * index + dsmf->skip;
  }

  if(dsmf->convert_x) {
    if(dsmf->convert_x(dsmf->xbuf, dsmf->xdata, dsmf->obj))
      return(NULL);
    else
      return(dsmf->xdata);
  }
  /*
   * The lines below should not have a ';' in them!
   */
  ELSE_IF_DATA_IS(unsigned char, SL_U_CHAR)
  ELSE_IF_DATA_IS(signed char, SL_S_CHAR)
  ELSE_IF_DATA_IS(unsigned short, SL_U_SHORT)
  ELSE_IF_DATA_IS(signed short, SL_S_SHORT)
  ELSE_IF_DATA_IS(unsigned int, SL_U_INT)
  ELSE_IF_DATA_IS(signed int, SL_S_INT)
  ELSE_IF_DATA_IS(unsigned long, SL_U_LONG)
  ELSE_IF_DATA_IS(signed long, SL_S_LONG)
  ELSE_IF_DATA_IS(float, SL_FLOAT)
  else if((dsmf->type == SL_DOUBLE) &&
	  (dsmf->x_read_width == dsmf->x_width * sizeof(double)))
    return((double *)dsmf->xbuf);
  else {
    ulog(ULOG_ERROR, "dsm_file_x: cannot convert x data.");
    return(NULL);
  }
  return(dsmf->xdata);

IO_ERROR:
  ulog(ULOG_ERROR, "dsm_file_x: I/O error for index = %d: %m.", (int)index);
  return(NULL);
}

#undef ELSE_IF_DATA_IS

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* The following code segment is used very many times in the function
   below.  So I am making it one big define to save some typing. */

#define ELSE_IF_DATA_IS(_TYPE, _TYPE_ENUM)                         \
  else if((dsmf->type == _TYPE_ENUM) &&                            \
	  (dsmf->y_read_width == dsmf->y_width * sizeof(_TYPE))) { \
    src = dsmf->ybuf;                                              \
    dst = dsmf->ydata;                                             \
    for(i = 0; i < dsmf->y_width; i++) {                           \
      *dst = *((_TYPE *)src);                                      \
      dst++;                                                       \
      src += sizeof(_TYPE);                                        \
    }                                                              \
  }

double *dsm_file_y(void *instance, unsigned index)
{
  char *src;
  double *dst;
  unsigned i;
  DSM_FILE *dsmf = instance;

  if(index >= dsmf->numpat) return(NULL);

  if(dsmf->fp) {
    if(fseek(dsmf->fp, (long)(dsmf->skip + dsmf->step * index +
			      dsmf->x_read_width + dsmf->offset),
	     SEEK_SET))
      goto IO_ERROR;
    if(fread(dsmf->ybuf, 1, dsmf->y_read_width, dsmf->fp) !=
       dsmf->y_read_width)
      goto IO_ERROR;
  }
  else {
    dsmf->ybuf = dsmf->mmapptr + dsmf->step * index +
      dsmf->x_read_width + dsmf->offset + dsmf->skip;
  }

  if(dsmf->convert_y) {
    if(dsmf->convert_y(dsmf->ybuf, dsmf->ydata, dsmf->obj))
      return(NULL);
    else
      return(dsmf->ydata);
  }
  /*
   * The lines below should not have a ';' in them!
   */
  ELSE_IF_DATA_IS(unsigned char, SL_U_CHAR)
  ELSE_IF_DATA_IS(signed char, SL_S_CHAR)
  ELSE_IF_DATA_IS(unsigned short, SL_U_SHORT)
  ELSE_IF_DATA_IS(signed short, SL_S_SHORT)
  ELSE_IF_DATA_IS(unsigned int, SL_U_INT)
  ELSE_IF_DATA_IS(signed int, SL_S_INT)
  ELSE_IF_DATA_IS(unsigned long, SL_U_LONG)
  ELSE_IF_DATA_IS(signed long, SL_S_LONG)
  ELSE_IF_DATA_IS(float, SL_FLOAT)
  else if((dsmf->type == SL_DOUBLE) &&
	  (dsmf->y_read_width == dsmf->y_width * sizeof(double)))
    return((double *)dsmf->ybuf);
  else {
    ulog(ULOG_ERROR, "dsm_file_y: cannot convert y data.");
    return(NULL);
  }
  return(dsmf->ydata);

IO_ERROR:
  ulog(ULOG_ERROR, "dsm_file_y: error for index = %d: %m.", (int)index);
  return(NULL);
}

#undef ELSE_IF_DATA_IS

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
