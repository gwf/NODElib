
/* Copyright (c) 1997 by G. W. Flake. */

#include "nodelib/xalloc.h"
#include "nodelib/ulog.h"
#include "nodelib/dataset.h"
#include "nodelib/dsunion.h"

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

DSM_UNION *dsm_union(void)
{
  return(array_create(8, DATASET *));
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void dsm_union_add(DSM_UNION *dsmunion, DATASET *elem)
{
  array_append((ARRAY *)dsmunion, elem, DATASET *);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void dsm_union_remove(DSM_UNION *dsmunion, unsigned index)
{
  array_destroy_index((ARRAY *)dsmunion, index);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

unsigned dsm_union_count(DSM_UNION *dsmunion)
{
  return(array_size((ARRAY *)dsmunion));
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

DATASET *dsm_union_elem(DSM_UNION *dsmunion, unsigned index)
{
  return(array_fast_access((ARRAY *)dsmunion, index, DATASET *));
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void dsm_destroy_union(DSM_UNION *dsmunion)
{
  array_destroy((ARRAY *)dsmunion);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

