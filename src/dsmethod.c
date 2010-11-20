
/* Copyright (c) 1996-1997 by G. W. Flake. */

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "nodelib/series.h"

static INLINE unsigned dsm_series_size(void *instance) {
  SERIES *series = instance; return(series_get_num_pat(series));
}

static INLINE unsigned dsm_series_x_size(void *instance) {
  SERIES *series = instance; return(series->x_width);
}

static INLINE unsigned dsm_series_y_size(void *instance) {
  SERIES *series = instance; return(series->y_width);
}

static INLINE double *dsm_series_x(void *instance, unsigned index) {
  SERIES *series = instance; return(series_get_x_pat(series, index));
}

static INLINE double *dsm_series_y(void *instance, unsigned index) {
  SERIES *series = instance; return(series_get_y_pat(series, index));
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "nodelib/dsmatrix.h"

static INLINE unsigned dsm_matrix_size(void *instance) {
  DSM_MATRIX *mtx = instance; return(mtx->rows);
}

static INLINE unsigned dsm_matrix_x_size(void *instance) {
  DSM_MATRIX *mtx = instance; return(mtx->xcols);
}

static INLINE unsigned dsm_matrix_y_size(void *instance) {
  DSM_MATRIX *mtx = instance; return(mtx->ycols);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static INLINE double *dsm_matrix_x(void *instance, unsigned index) {
  DSM_MATRIX *mtx = instance;
  unsigned inc;

  inc = mtx->y ? mtx->xcols : mtx->xcols + mtx->ycols;
  return(&mtx->x[index * inc]);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static double *dsm_matrix_y(void *instance, unsigned index) {
  DSM_MATRIX *mtx = instance;
  unsigned inc, off;
  double *ptr;

  ptr = mtx->y ? mtx->y : mtx->x;
  inc = mtx->y ? mtx->ycols : mtx->xcols + mtx->ycols;
  off = mtx->y ? 0 : mtx->xcols;
  return(&ptr[index * inc + off]);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "nodelib/dsdblptr.h"

static INLINE unsigned dsm_dblptr_size(void *instance) {
  DSM_DBLPTR *mtx = instance; return(mtx->rows);
}

static INLINE unsigned dsm_dblptr_x_size(void *instance) {
  DSM_DBLPTR *mtx = instance; return(mtx->xcols);
}

static INLINE unsigned dsm_dblptr_y_size(void *instance) {
  DSM_DBLPTR *mtx = instance; return(mtx->ycols);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static INLINE double *dsm_dblptr_x(void *instance, unsigned index) {
  DSM_DBLPTR *mtx = instance;

  return(mtx->x[index]);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static INLINE double *dsm_dblptr_y(void *instance, unsigned index) {
  DSM_DBLPTR *mtx = instance;

  if(mtx->y)
    return(mtx->y[index]);
  else
    return(&mtx->x[index][mtx->xcols]);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "nodelib/dsfile.h"

#include <sys/stat.h>
#ifndef WIN32
#include <unistd.h>
#endif

static INLINE unsigned dsm_file_size(void *instance) {
  DSM_FILE *dsmf = instance; return(dsmf->numpat);
}

static INLINE unsigned dsm_file_x_size(void *instance) {
  DSM_FILE *dsmf = instance; return(dsmf->x_width);
}

static INLINE unsigned dsm_file_y_size(void *instance) {
  DSM_FILE *dsmf = instance; return(dsmf->y_width);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "nodelib/dssubset.h"

static INLINE unsigned dsm_subset_size(void *instance) {
  DSM_SUBSET *subset = instance; return(subset->len);
}

static INLINE unsigned dsm_subset_x_size(void *instance) {
  DSM_SUBSET *subset = instance; return(dataset_x_size(subset->dset));
}

static INLINE unsigned dsm_subset_y_size(void *instance) {
  DSM_SUBSET *subset = instance; return(dataset_y_size(subset->dset));
}

static INLINE double *dsm_subset_x(void *instance, unsigned index) {
  DSM_SUBSET *subset = instance;
  return(dataset_x(subset->dset, subset->start + index * subset->skip));
}

static INLINE double *dsm_subset_y(void *instance, unsigned index) {
  DSM_SUBSET *subset = instance;
  return(dataset_y(subset->dset, subset->start + index * subset->skip));
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "nodelib/dsisubset.h"

static INLINE unsigned dsm_isubset_size(void *instance) {
  DSM_ISUBSET *isubset = instance; return(isubset->len);
}

static INLINE unsigned dsm_isubset_x_size(void *instance) {
  DSM_ISUBSET *isubset = instance; return(dataset_x_size(isubset->dset));
}

static INLINE unsigned dsm_isubset_y_size(void *instance) {
  DSM_ISUBSET *isubset = instance; return(dataset_y_size(isubset->dset));
}

static INLINE double *dsm_isubset_x(void *instance, unsigned index) {
  DSM_ISUBSET *isubset = instance;
  return(dataset_x(isubset->dset, isubset->index[index]));
}

static INLINE double *dsm_isubset_y(void *instance, unsigned index) {
  DSM_ISUBSET *isubset = instance;
  return(dataset_y(isubset->dset, isubset->index[index]));
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "nodelib/dsfifo.h"

static INLINE unsigned dsm_fifo_size(void *instance) {
  DSM_FIFO *fifo = instance; return(fifo->used);
}

static INLINE unsigned dsm_fifo_x_size(void *instance) {
  DSM_FIFO *fifo = instance; return(fifo->xsz);
}

static INLINE unsigned dsm_fifo_y_size(void *instance) {
  DSM_FIFO *fifo = instance; return(fifo->ysz);
}

static INLINE double *dsm_fifo_x(void *instance, unsigned index) {
  DSM_FIFO *fifo = instance;
  return(fifo->x[(index + fifo->first) % fifo->sz]);
}

static INLINE double *dsm_fifo_y(void *instance, unsigned index) {
  DSM_FIFO *fifo = instance;
  return(fifo->y[(index + fifo->first) % fifo->sz]);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "nodelib/dsunion.h"

static unsigned dsm_union_size(void *instance) {
  DSM_UNION *dsmunion = instance;
  unsigned i, n, sz = 0;

  n = dsm_union_count(dsmunion);
  for(i = 0; i < n; i++)
    sz += dataset_size(dsm_union_elem(dsmunion, i));
  return(sz);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static INLINE unsigned dsm_union_x_size(void *instance) {
  DSM_UNION *dsmunion = instance;
  return(dataset_x_size(dsm_union_elem(dsmunion, 0)));
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static INLINE unsigned dsm_union_y_size(void *instance) {
  DSM_UNION *dsmunion = instance;
  return(dataset_y_size(dsm_union_elem(dsmunion, 0)));
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static double *dsm_union_x(void *instance, unsigned index) {
  DSM_UNION *dsmunion = instance;
  unsigned i, n, lsz, sz = 0;

  n = dsm_union_count(dsmunion);
  for(i = 0; i < n; i++) {
    lsz = dataset_size(dsm_union_elem(dsmunion, i));
    sz += lsz;
    if(index < sz)
      return(dataset_x(dsm_union_elem(dsmunion, i), index + lsz - sz));
  }
  return(NULL);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static double *dsm_union_y(void *instance, unsigned index) {
  DSM_UNION *dsmunion = instance;
  unsigned i, n, lsz, sz = 0;

  n = dsm_union_count(dsmunion);
  for(i = 0; i < n; i++) {
    lsz = dataset_size(dsm_union_elem(dsmunion, i));
    sz += lsz;
    if(index < sz)
      return(dataset_y(dsm_union_elem(dsmunion, i), index + lsz - sz));
  }
  return(NULL);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#define OWNER
#include "nodelib/dsmethod.h"
#undef OWNER

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
