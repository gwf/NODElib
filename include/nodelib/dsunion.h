
/* Copyright (c) 1997 by G. W. Flake.
 *
 * NAME
 *   dsunion.h - DATASET_METHOD union of other DATASETs
 * SYNOPSIS
 *   Given multiple existing DATASETs, one can define a new DATASET
 *   that is the union of original ones.  This is useful for when
 *   multiple data sources have to be conceptually merged (e.g., 
 *   training on multiple files as if thery were one).
 * AUTHOR
 *   Gary William Flake (\url{\bf{gary.flake@usa.net}}{mailto:gary.flake@usa.net}).
 * SEE ALSO
 *   \bf{dsmethod}(3), and \bf{dataset}(3).
 */

#ifndef __DSUNION_H__
#define __DSUNION_H__

#define NOEXTERN
#include "nodelib/dataset.h"
#undef NOEXTERN

#include "nodelib/array.h"
#include "nodelib/etc/version.h"
#include "nodelib/etc/options.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* A DSM_UNION specifies how to reference the union of the data in
   contained in several DATASETs.  The total number of patterns in the
   union is equal to the sum of the sizes.  Accessing patterns by index
   does does so in the order in that they are added to the union. */

typedef ARRAY DSM_UNION;

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* Allocates a new union with no members. */

DSM_UNION *dsm_union(void);


/* Adds \em{elem} to \em{dsmunion.} */

void dsm_union_add(DSM_UNION *dsmunion, DATASET *elem);


/* Removes the DATASET indexed by \em{index} from \em{dsmunion}.  The first
   element always has index 0. */

void dsm_union_remove(DSM_UNION *dsmunion, unsigned index);


/* Returns the total number of elements that hav been added to
   \em{dsmunion}. */

unsigned dsm_union_count(DSM_UNION *dsmunion);


/* Returns the element from \em{dsmunion} with index \em{index}. */

DATASET *dsm_union_elem(DSM_UNION *dsmunion, unsigned index);


/* Frees up any memory that was allocated with the allocation routine.
   The member DATASETs are left intact; hence, it is your job to free
   them up. */

void dsm_destroy_union(DSM_UNION *dsmunion);

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __DSUNION_H__ */
