
/* Copyright (c) 1995 by G. W. Flake. */

#include "nodelib/series.h"
#include "nodelib/ulog.h"
#include "nodelib/xalloc.h"
#include "nodelib/scan.h"

#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#define RISKY_PAT 1

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

INLINE unsigned series_get_num_pat(SERIES *ser)
{
  if(!ser->initp) series_reinitiate(ser);
  return(ser->numpats);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

SERIES *series_create(void)
{
  SERIES *ser;

  ser = xmalloc(sizeof(SERIES));
  ser->x_width = 1;
  ser->x_delta = 1;
  ser->y_width = 1;
  ser->y_delta = 1;
  ser->offset = 1;
  ser->step = 1;
  ser->data = array_create(100, double);
  ser->x_pat = array_create(10, double);
  ser->y_pat = array_create(10, double);
  ser->var_x_deltas = ser->var_y_deltas = NULL;
  ser->patsz = ser->numpats = ser->initp = 0;
  return(ser);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void series_destroy(SERIES *ser)
{
  array_destroy(ser->data);
  array_destroy(ser->x_pat);
  array_destroy(ser->y_pat);
  xfree(ser);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

INLINE void series_clear(SERIES *ser)
{
  array_clear(ser->data);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

SERIES *series_read_ascii_fp(FILE *fp)
{
  SERIES *ser;
  double data;
  SCAN *scan;
  char *ptr;
  
  /* Allows us to skip `#' lines. */
  scan = scan_create(1, fp);
  scan->delims = "";
  scan->comments = "#";
  scan->whites = " \t\n";
  ser = series_create();
  while((ptr = scan_get(scan)) != NULL) {
    data = atof(ptr);
    if(data != data) /* NaN */ {
      ulog(ULOG_WARN, "series_read_ascii_fp: invalid data encountered.");
      break;
    }
    array_push(ser->data, data, double);
  }
  scan_destroy(scan);
  return(ser);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

SERIES *series_read_ascii(char *name)
{
  FILE *fp;
  SERIES *ser;

  fp = fopen(name, "r");
  if(fp == NULL) {
    ulog(ULOG_WARN, "series_read_ascii: unable to open '%s': %m.", name);
    return(NULL);
  }
  ser = series_read_ascii_fp(fp);
  fclose(fp);
  return(ser);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void series_write_ascii_fp(SERIES *ser, FILE *fp)
{
  double *dat;
  int i, j, num, xsz, ysz;
  
  num = series_get_num_pat(ser);
  xsz = ser->x_width; ysz = ser->y_width;
  fprintf(fp, "# num = %d, xsz = %d, ysz = %d\n", num, xsz, ysz);
  for(i = 0; i < num; i++) {
    dat = series_get_x_pat(ser, i);
    for(j = 0; j < xsz; j++)
      fprintf(fp, "%f\n", dat[j]);
    dat = series_get_y_pat(ser, i);
    for(j = 0; j < ysz; j++)
      fprintf(fp, "%f\n", dat[j]);
  }
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

int series_write_ascii(SERIES *ser, char *name)
{
  FILE *fp;

  fp = fopen(name, "w");
  if(fp == NULL) {
    ulog(ULOG_WARN, "series_write_ascii: unable to open '%s': %m.", name);
    return(1);
  }
  series_write_ascii_fp(ser, fp);
  fclose(fp);
  return(0);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void series_reinitiate(SERIES *ser)
{
  unsigned sz;

  /* Compute the pattern size. */
  if(ser->var_x_deltas)
    ser->xsz = ser->var_x_deltas[ser->x_width - 2] + 1;
  else if(ser->x_width > 0)
    ser->xsz = 1 + ((ser->x_width - 1) * ser->x_delta);
  else
    ser->xsz = 0;

  if(ser->var_y_deltas)
    ser->ysz = ser->var_y_deltas[ser->y_width - 2] + 1;
  else if(ser->y_width > 0)
    ser->ysz = 1 + ((ser->y_width - 1) * ser->y_delta);
  else
    ser->ysz = 0;

  ser->patsz = ser->xsz + ser->offset + ser->ysz - 1;

  /* Compute the number of patterns. */
  sz = array_size(ser->data);
  ser->numpats = (sz < ser->patsz) ? 0 : (sz - ser->patsz) / ser->step + 1;

  ser->initp = 1;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

INLINE unsigned series_get_pat_size(SERIES *ser)
{
  if(!ser->initp) series_reinitiate(ser);
  return(ser->patsz);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

double *series_get_x_pat(SERIES *ser, unsigned pindex)
{
  double data;
  unsigned i, num_pat, base, index;

  num_pat = series_get_num_pat(ser);
  if(pindex >= num_pat) {
    ulog(ULOG_ERROR, "series_get_x_pat: series index (%d) out of"
	 " range [0...%d].", pindex, num_pat - 1);
    return(NULL);
  }
  if(!ser->var_x_deltas) {
#if RISKY_PAT
    if(ser->x_delta == 1)
      return(&array_fast_access(ser->data, ser->step * pindex, double));
#endif
    array_clear(ser->x_pat);
    for(i = 0; i < ser->x_width; i++) {
      index = ser->step * pindex + ser->x_delta * i;
      data = array_fast_access(ser->data, index, double);
      array_push(ser->x_pat, data, double);
    }
  }
  else {
    base = index = ser->step * pindex + ser->xsz - 1;
    data = array_fast_access(ser->data, index, double);
    array_clear(ser->x_pat);
    array_push(ser->x_pat, data, double);
    for(i = 0; i < ser->x_width - 1; i++) {
      index = base - ser->var_x_deltas[i];
      data = array_fast_access(ser->data, index, double);
      array_push(ser->x_pat, data, double);
    }
  }
  return(&array_fast_access(ser->x_pat, 0, double));
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

double *series_get_y_pat(SERIES *ser, unsigned pindex)
{
  double data;
  unsigned i, num_pat, base, index;

  num_pat = series_get_num_pat(ser);
  if(pindex >= num_pat) {
    ulog(ULOG_ERROR, "series_get_y_pat: series index (%d) out of "
	 "range [0...%d].", pindex, num_pat - 1);
    return(NULL);
  }
  if(!ser->var_y_deltas) {
    base = ser->step * pindex + ser->xsz - 1 + ser->offset;
#if RISKY_PAT
    if(ser->x_delta == 1)
      return(&array_fast_access(ser->data, base, double));
#endif
    array_clear(ser->y_pat);
    for(i = 0; i < ser->y_width; i++) {
      index = base + ser->y_delta * i;
      data = array_fast_access(ser->data, index, double);
      array_push(ser->y_pat, data, double);
    }
  }
  else {
    base = index = ser->step * pindex + 
      ser->xsz - 2 + ser->offset + ser->ysz;
    data = array_fast_access(ser->data, index, double);
    array_clear(ser->y_pat);
    array_push(ser->y_pat, data, double);
    for(i = 0; i < ser->y_width - 1; i++) {
      index = base - ser->var_y_deltas[i];
      data = array_fast_access(ser->data, index, double);
      array_push(ser->y_pat, data, double);
    }
  }
  return(&array_fast_access(ser->y_pat, 0, double));
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

double series_get_x(SERIES *ser, unsigned pindex, unsigned xindex)
{
  unsigned index, num_pat;

  if(xindex >= ser->x_width) {
    ulog(ULOG_ERROR, "series_get_x: index (%d) out of range [0...%d].",
	 xindex, ser->x_width - 1);
    return(0.0);
  }
  num_pat = series_get_num_pat(ser);
  if(pindex >= num_pat) {
    ulog(ULOG_ERROR, "series_get_x: series index (%d) out of range [0...%d].",
	 pindex, num_pat - 1);
    return(0.0);
  }
  if(!ser->var_x_deltas)
    index = ser->step * pindex + ser->x_delta * xindex;
  else if(xindex == 0)
    index = ser->step * pindex + ser->xsz - 1;
  else
    index = ser->step * pindex + ser->xsz - 1 - ser->var_x_deltas[xindex - 1];
  return(array_fast_access(ser->data, index, double));
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

double series_get_y(SERIES *ser, unsigned pindex, unsigned yindex)
{
  unsigned index, num_pat;

  if(yindex >= ser->y_width) {
    ulog(ULOG_ERROR, "series_get_y: index (%d) out of range [0...%d].",
	 yindex, ser->y_width - 1);
    return(0.0);
  }
  num_pat = series_get_num_pat(ser);
  if(pindex >= num_pat) {
    ulog(ULOG_ERROR, "series_get_y: series index (%d) out of range [0...%d].",
	 pindex, num_pat - 1);
    return(0.0);
  }
  if(!ser->var_y_deltas)
    index = ser->step * pindex + ser->xsz - 1 +
      ser->offset + ser->y_delta * yindex;
  else if(yindex == 0)
    index = ser->step * pindex + ser->xsz - 2 + ser->offset + ser->ysz;
  else
    index = ser->step * pindex + ser->xsz - 2 +
      ser->offset + ser->ysz - ser->var_y_deltas[yindex - 1];
  return(array_fast_access(ser->data, index, double));
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

int series_set_x(SERIES *ser, unsigned pindex, unsigned xindex, double val)
{
  unsigned index, num_pat;

  if(xindex >= ser->x_width) {
    ulog(ULOG_ERROR, "series_set_x: index (%d) out of range [0...%d].",
	 xindex, ser->x_width - 1);
    return(1);
  }
  num_pat = series_get_num_pat(ser);
  if(pindex >= num_pat) {
    ulog(ULOG_ERROR, "series_set_x: series index (%d) out of range [0...%d].",
	 pindex, num_pat - 1);
    return(1);
  }
  if(!ser->var_x_deltas)
    index = ser->step * pindex + ser->x_delta * xindex;
  else if(xindex == 0)
    index = ser->step * pindex + ser->xsz - 2;
  else
    index = ser->step * pindex + ser->xsz - 2 - ser->var_x_deltas[xindex - 1];
  array_fast_access(ser->data, index, double) = val;
  return(0);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

int series_set_y(SERIES *ser, unsigned pindex, unsigned yindex, double val)
{
  unsigned index, num_pat;

  if(yindex >= ser->y_width) {
    ulog(ULOG_ERROR, "series_set_y: index (%d) out of range [0...%d].",
	 yindex, ser->y_width - 1);
    return(1);
  }
  num_pat = series_get_num_pat(ser);
  if(pindex >= num_pat) {
    ulog(ULOG_ERROR, "series_set_y: series index (%d) out of range [0...%d].",
	 pindex, num_pat - 1);
    return(1);
  }
  if(!ser->var_y_deltas)
    index = ser->step * pindex + ser->xsz - 1 +
      ser->offset + ser->y_delta * yindex;
  else if(yindex == 0)
    index = ser->step * pindex + ser->xsz - 1 + ser->offset + ser->ysz;
  else
    index = ser->step * pindex + ser->xsz - 1 +
      ser->offset + ser->ysz - ser->var_y_deltas[yindex - 1];
  array_fast_access(ser->data, index, double) = val;
  return(0);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

INLINE void series_append_pat(SERIES *ser, double *data, unsigned sz)
{
  array_append_ptr(ser->data, (char *)data, sz);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

INLINE void series_append_val(SERIES *ser, double val)
{
  array_push(ser->data, val, double);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

double series_average(SERIES *ser)
{
  unsigned i, num;
  double sum = 0.0;

  num = array_size(ser->data);
  for(i = 0; i < num; i++)
    sum += array_fast_access(ser->data, i, double);
  return(sum / num);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

double series_stddev(SERIES *ser)
{
  unsigned i, num;
  double sum = 0.0, sumsq = 0.0, data;

  num = array_size(ser->data);
  for(i = 0; i < num; i++) {
    data = array_fast_access(ser->data, i, double);
    sum += data;
    sumsq += data * data;
  }
  return(sqrt(sumsq / num - (sum / num) * (sum / num)));
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

double series_min(SERIES *ser)
{
  unsigned i, num;
  double min = 10e50, data;

  num = array_size(ser->data);
  for(i = 0; i < num; i++) {
    data = array_fast_access(ser->data, i, double);
    if(data < min)
      min = data;
  }
  return(min);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

double series_max(SERIES *ser)
{
  unsigned i, num;
  double max = -10e50, data;

  num = array_size(ser->data);
  for(i = 0; i < num; i++) {
    data = array_fast_access(ser->data, i, double);
    if(data > max)
      max = data;
  }
  return(max);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

