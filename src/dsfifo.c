
/* Copyright (c) 1997 by G. W. Flake. */

#include "nodelib/xalloc.h"
#include "nodelib/ulog.h"
#include "nodelib/dsfifo.h"
#include "nodelib/misc.h"

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

DSM_FIFO *dsm_fifo(unsigned xsz, unsigned ysz, unsigned sz)
{
  DSM_FIFO *fifo;

  fifo = xmalloc(sizeof(DSM_FIFO));
  fifo->x = allocate_array(2, sizeof(double), sz, xsz);
  fifo->y = allocate_array(2, sizeof(double), sz, ysz);
  fifo->xsz = xsz;
  fifo->ysz = ysz;
  fifo->sz = sz;
  fifo->used = 0;
  fifo->first = 0;
  return(fifo);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void dsm_fifo_new_pattern(DSM_FIFO *fifo, double *x, double *y)
{
  unsigned new, i;

  new = (fifo->first + fifo->sz - 1) % fifo->sz;

  for(i = 0; i < fifo->xsz; i++)
    fifo->x[new][i] = x[i];
  for(i = 0; i < fifo->ysz; i++)
    fifo->y[new][i] = y[i];

  fifo->first = new;
  if(fifo->used < fifo->sz) fifo->used++;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void dsm_destroy_fifo(DSM_FIFO *fifo)
{
  deallocate_array(fifo->x);
  deallocate_array(fifo->y);
  xfree(fifo);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
