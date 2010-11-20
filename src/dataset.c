
/* Copyright (c) 1996 by G. W. Flake. */

#include "nodelib/dataset.h"
#include "nodelib/xalloc.h"
#include "nodelib/misc.h"

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

DATASET *dataset_create(DATASET_METHOD *method, void *instance)
{
  DATASET *dataset;

  dataset = xmalloc(sizeof(DATASET));
  dataset->instance = instance;
  dataset->method = method;
  return(dataset);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void *dataset_destroy(DATASET *dataset)
{
  void *instance;

  instance = dataset->instance;
  xfree(dataset);
  return(instance);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

INLINE unsigned dataset_size(DATASET *dataset)
{
  return(dataset->method->size(dataset->instance));
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

INLINE unsigned dataset_x_size(DATASET *dataset)
{
  return(dataset->method->x_size(dataset->instance));
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

INLINE unsigned dataset_y_size(DATASET *dataset)
{
  return(dataset->method->y_size(dataset->instance));
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

INLINE double *dataset_x(DATASET *dataset, unsigned index)
{
  return(dataset->method->x(dataset->instance, index));
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

INLINE double *dataset_y(DATASET *dataset, unsigned index)
{
  return(dataset->method->y(dataset->instance, index));
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* In honor of Bharat Rao, the next two functions use pointer arithmetic.
   -- GWF  */

double *dataset_x_copy(DATASET *dataset, unsigned index, double *dst)
{
  unsigned i, n;
  double *x, *d;

  n = dataset->method->x_size(dataset->instance);
  x = dataset->method->x(dataset->instance, index);
  if(dst == NULL) dst = allocate_array(1, sizeof(double), n);
  d = dst;
  for(i = 0; i < n; i++)
    *d++ = *x++;
  return(dst);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

double *dataset_y_copy(DATASET *dataset, unsigned index, double *dst)
{
  unsigned i, n;
  double *y, *d;

  n = dataset->method->y_size(dataset->instance);
  y = dataset->method->y(dataset->instance, index);
  if(dst == NULL) dst = allocate_array(1, sizeof(double), n);
  d = dst;
  for(i = 0; i < n; i++)
    *d++ = *y++;
  return(dst);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


