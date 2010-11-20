
/* Copyright (c) 1996 by G. W. Flake.
 *
 * NAME
 *   dsmatrix.h - DATASET_METHOD for matrix types
 * SYNOPSIS
 *   These routines define methods for accessing matrices.  With this
 *   module, a DATASET can consist of a single matrix or two matrices.
 * AUTHOR
 *   Gary William Flake (\url{\bf{gary.flake@usa.net}}{mailto:gary.flake@usa.net}).
 * SEE ALSO
 *   \bf{dsmethod}(3), and \bf{dataset}(3).
 */

#ifndef __DSMATRIX_H__
#define __DSMATRIX_H__

#include "nodelib/etc/version.h"
#include "nodelib/etc/options.h"


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* The DSM_MATRIX type specififies how a matrix should be interpreted.
   There are two possibilities that we handle.  First, there may be
   one or two matrices.  If there is just one, then the inputs reside
   on the left-hand side of the rows, and the targets reside on the right.
   In this case, \em{xcols} and \em{xcols} should sum to the total
   number of columns in the single matrix.  If there are two matrices,
   then \em{xcols} and \em{ycols} specify their respective number
   of columns.  They must have the same number of rows, however. */

typedef struct DSM_MATRIX {
  double *x, *y;
  unsigned xcols;
  unsigned ycols;
  unsigned rows;
} DSM_MATRIX;

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* Declare a single C matrix to be a source for data. */

DSM_MATRIX *dsm_c_matrix(double *x, unsigned xcols, unsigned ycols, /*\*/
                         unsigned rows);


/* Declare a two C matrices to be a source for data. */

DSM_MATRIX *dsm_c_matrices(double *x, double *y, unsigned xcols, /*\*/
                           unsigned ycols, unsigned rows);


/* Free up any memory that was allocated with one of the two
   allocation routines.  The pointers that are passed to the
   DSM_MATRIX creation routines are left intact; hence, it is your job
   to free them up. The value of \em{mtx->x} is returned. */

void *dsm_destroy_matrix(DSM_MATRIX *mtx);

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __DSMATRIX_H__ */
