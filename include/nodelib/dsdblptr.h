
/* Copyright (c) 1996 by G. W. Flake.
 *
 * NAME
 *   dsdblptr.h - DATASET_METHOD for double pointer matrix types
 * SYNOPSIS
 *   These routines define methods for accessing matrices that are
 *   dynamically allocated as pointers to pointers of doubles.  With this
 *   module, a DATASET can consist of a single matrix or two matrices.
 * AUTHOR
 *   Gary William Flake (\url{\bf{gary.flake@usa.net}}{mailto:gary.flake@usa.net}).
 * SEE ALSO
 *   \bf{dsmethod}(3), and \bf{dataset}(3).
 */

#ifndef __DSDBLPTR_H__
#define __DSDBLPTR_H__

#include "nodelib/etc/version.h"
#include "nodelib/etc/options.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* The DSM_DBLPTR type specififies how a double pointer matrix should be
   interpreted.  There are two possibilities that we handle.  First,
   there may be one or two matrices.  If there is just one, then the
   inputs reside on the left-hand side of the rows, and the targets
   reside on the right.  In this case, \em{xcols} and \em{xcols}
   should sum to the total number of columns in the single matrix.
   If there are two matrices,  then \em{xcols} and \em{ycols}
   specify their respective number of columns.  They must have the
   same number of rows, however. */

typedef struct DSM_DBLPTR {
  double **x, **y;
  unsigned xcols;
  unsigned ycols;
  unsigned rows;
} DSM_DBLPTR;

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* Declare a single C pointer matrix to be a source for data. */

DSM_DBLPTR *dsm_c_dblptr(double **x, unsigned xcols, unsigned ycols, /*\*/
                         unsigned rows);


/* Declare two C pointer matrices to be a source for data. */

DSM_DBLPTR *dsm_c_dblptrs(double **x, double **y, unsigned xcols, /*\*/
                          unsigned ycols, unsigned rows);


/* Free up any memory that was allocated with one of the two
   allocation routines.  The pointers that are passed to the
   DSM_DBLPTR creation routines are left intact; hence, it is your job
   to free them up.  The value of \em{mtx->x} is returned. */

void *dsm_destroy_dblptr(DSM_DBLPTR *mtx);

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __DSDBLPTR_H__ */
