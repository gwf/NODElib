
/* Copyright (c) 1997 by G. W. Flake.
 *
 * NAME
 *   dsisubset.h - DATASET_METHOD subset type
 * SYNOPSIS
 *   Given an existing DATASET, one can define a new subset of the
 *   first data set.  The actual subset is determined by a user
 *   specified vector of indices. This is useful for incremental
 *   techniques applied to huge amounts of data.
 * AUTHOR
 *   Gary William Flake (\url{\bf{gary.flake@usa.net}}{mailto:gary.flake@usa.net}).
 * SEE ALSO
 *   \bf{dsmethod}(3), and \bf{dataset}(3).
 */

#ifndef __DSISUBSET_H__
#define __DSISUBSET_H__

#define NOEXTERN
#include "nodelib/dataset.h"
#undef NOEXTERN

#include "nodelib/etc/version.h"
#include "nodelib/etc/options.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* A DSM_ISUBSET specifies how to reference a subset of the data in
   \em{dset}.  If you were to ask for the \em{i'th} pattern of the
   DATASET wrapped by this structure, in reality you would get the
   \em{index[i]} pattern of \em{dset.}  Also, in the subset,
   you can only reference \em{len} patterns, so the values in these
   fields should make sense for \{dset.} */

typedef struct DSM_ISUBSET {
  DATASET *dset;
  unsigned *index;
  unsigned len;
} DSM_ISUBSET;

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* Allocates a new isubset, and fills up the appropriate fields of the
   structure. */

DSM_ISUBSET *dsm_isubset(DATASET *dset, unsigned *index, unsigned len);

/* Free up any memory that was allocated with the allocation routine.
   The original DATASET that is passed to dsm_subset() is left intact;
   hence, it is your job to free them up. */

void dsm_destroy_isubset(DSM_ISUBSET *isubset);

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __DSISUBSET_H__ */
