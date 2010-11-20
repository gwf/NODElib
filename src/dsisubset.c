
/* Copyright (c) 1997 by G. W. Flake. */

#include "nodelib/xalloc.h"
#include "nodelib/ulog.h"
#include "nodelib/dsisubset.h"

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

DSM_ISUBSET *dsm_isubset(DATASET *dset, unsigned *index, unsigned len)
{
  DSM_ISUBSET *isubset;
  unsigned sz;
 
  sz = dataset_size(dset);
  if(len == 0 || len > sz) {
    ulog(ULOG_ERROR, "DSM_ISUBSET: parameters failed sanity check."
	 "%t(%d == 0 || %d > %d).", len, len, sz);
    return(NULL);
  }
  isubset = xmalloc(sizeof(DSM_ISUBSET));
  isubset->dset = dset;
  isubset->index = index;
  isubset->len = len;
  return(isubset);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void dsm_destroy_isubset(DSM_ISUBSET *isubset)
{
  xfree(isubset);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

