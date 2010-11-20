
/* Copyright (c) 1997 by G. W. Flake. */

#include "nodelib/xalloc.h"
#include "nodelib/ulog.h"
#include "nodelib/dssubset.h"

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

DSM_SUBSET *dsm_subset(DATASET *dset, unsigned start, unsigned len,
		       unsigned skip)
{
  DSM_SUBSET *subset;
  unsigned sz;
 
  sz = dataset_size(dset);
  if(len == 0 || start + (len - 1) * skip >= sz) {
    ulog(ULOG_ERROR, "dsm_subset: parameters failed sanity check."
	 "%t(%d == 0 || %d + (%d - 1) * %d >= %d).",
	 len, start, len, skip, sz);
    return(NULL);
  }
  subset = xmalloc(sizeof(DSM_SUBSET));
  subset->dset = dset;
  subset->start = start;
  subset->len = len;
  subset->skip = skip;
  return(subset);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void dsm_destroy_subset(DSM_SUBSET *subset)
{
  xfree(subset);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

