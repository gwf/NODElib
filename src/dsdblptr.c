
/* Copyright (c) 1996 by G. W. Flake. */

#include "nodelib/xalloc.h"
#include "nodelib/dsdblptr.h"

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

DSM_DBLPTR *dsm_c_dblptr(double **x, unsigned xcols, unsigned ycols,
			 unsigned rows)
{
  DSM_DBLPTR *mtx = xmalloc(sizeof(DSM_DBLPTR));

  mtx->x = x;
  mtx->y = NULL;
  mtx->xcols = xcols;
  mtx->ycols = ycols;
  mtx->rows = rows;
  return(mtx);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

DSM_DBLPTR *dsm_c_dblptrs(double **x, double **y, unsigned xcols,
			  unsigned ycols, unsigned rows)
{
  DSM_DBLPTR *mtx = xmalloc(sizeof(DSM_DBLPTR));

  mtx->x = x;
  mtx->y = y;
  mtx->xcols = xcols;
  mtx->ycols = ycols;
  mtx->rows = rows;
  return(mtx);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void *dsm_destroy_dblptr(DSM_DBLPTR *mtx)
{
  void *ptr = mtx->x;
  xfree(mtx);
  return(ptr);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

