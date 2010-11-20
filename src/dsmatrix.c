
/* Copyright (c) 1996 by G. W. Flake. */

#include "nodelib/xalloc.h"
#include "nodelib/dsmatrix.h"

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

DSM_MATRIX *dsm_c_matrix(double *x, unsigned xcols, unsigned ycols,
			 unsigned rows)
{
  DSM_MATRIX *mtx = xmalloc(sizeof(DSM_MATRIX));

  mtx->x = x;
  mtx->y = NULL;
  mtx->xcols = xcols;
  mtx->ycols = ycols;
  mtx->rows = rows;
  return(mtx);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

DSM_MATRIX *dsm_c_matrices(double *x, double *y, unsigned xcols,
			   unsigned ycols, unsigned rows)
{
  DSM_MATRIX *mtx = xmalloc(sizeof(DSM_MATRIX));

  mtx->x = x;
  mtx->y = y;
  mtx->xcols = xcols;
  mtx->ycols = ycols;
  mtx->rows = rows;
  return(mtx);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void *dsm_destroy_matrix(DSM_MATRIX *mtx)
{
  void *ptr = mtx->x;
  xfree(mtx);
  return(ptr);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

