
/* Copyright (c) 1997 by G. W. Flake.
 *
 * NAME
 *   dsfifo.h - DATASET_METHOD for fixed-size FIFO.
 * SYNOPSIS
 *   A FIFO (first in, first out) allows you to hold the most current
 *   training patterns in a fixed size set.  Whenever a new pattern is
 *   added, the oldest pattern is overwritten.  This is handy for
 *   incremental learning.
 * AUTHOR
 *   Gary William Flake (\url{\bf{gary.flake@usa.net}}{mailto:gary.flake@usa.net}).
 * SEE ALSO
 *   \bf{dsmethod}(3), and \bf{dataset}(3).
 */

#ifndef __DSFIFO_H__
#define __DSFIFO_H__

#include "nodelib/etc/version.h"
#include "nodelib/etc/options.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* A DSM_FIFO defines a fixed-width sliding window of training
   patterns.  You should never manipulate this structure directly.
   The meaning of all of the fields is mostly obvious.  The \em{used}
   field contains the number (less or equal to \em{sz}) of positions
   used in the DSM_FIFO.  The \em{first} field is used to keep track
   of where the most recently added pattern is located. */

typedef struct DSM_FIFO {
  double **x, **y;
  unsigned xsz, ysz, sz;
  unsigned used, first;
} DSM_FIFO;

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* Creates a DSM_FIFO with the pattern dimensions specified by
   \em{xsz} and \em{ysz}, while \em{sz} is the number of patterns that
   can be stored in the DSM_FIFO at any time. */

DSM_FIFO *dsm_fifo(unsigned xsz, unsigned ysz, unsigned sz);


/* If \em{fifo} is not full, then the new data is copied into
   \em{fifo.}  But if \em{fifo} is full, then the oldest pattern is
   replaced with the new pattern. */

void dsm_fifo_new_pattern(DSM_FIFO *fifo, double *x, double *y);


/* Free up any memory that was allocated with the allocation routine. */

void dsm_destroy_fifo(DSM_FIFO *fifo);

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifdef __cplusplus
}{
#endif /* __cplusplus */

#endif /* __DSFIFO_H__ */
