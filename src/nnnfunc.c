
/* Copyright (c) 1995 by G. W. Flake. */

#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

#include "nodelib/nn.h"
#include "nodelib/xalloc.h"
#include "nodelib/hash.h"

static HASH *nfhash = NULL;
static void nfinit(void);

/* linear, quadratic, xquadratic, copy, alias, diagonal,
   euclidean, pair-wise product, product */

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


void nn_register_netfunc(char *name,
			 void (*forward)(struct NN *nn, 
					 struct NN_LINK *link,
					 struct NN_LAYER *dst),
                         void (*backward)(struct NN *nn,
					  struct NN_LINK *link,
					  NN_LAYER *src),
			 void (*Rforward)(struct NN *nn, 
					  struct NN_LINK *link,
					  NN_LAYER *dst),
                         void (*Rbackward)(struct NN *nn, 
					   struct NN_LINK *link,
					   NN_LAYER *src),
			 int (*sanity)(struct NN *nn, 
				       struct NN_LAYERLIST *source,
				       struct NN_LAYERLIST *destination,
				       unsigned int *numin, 
				       unsigned int *numout, 
				       unsigned int *numaux))
{
  NN_NETFUNC nf, *nfx;

  if(nfhash == NULL) nfinit();
  nf.name = name;
  if((nfx = hash_search(nfhash, &nf)) != NULL) {
    /* Found named net func, so just change the ptrs. */
    nfx->name = name;
    nfx->forward = forward;
    nfx->backward = backward;
    nfx->Rforward = Rforward;
    nfx->Rbackward = Rbackward;
    nfx->sanity = sanity;
  }
  else {
    nfx = xmalloc(sizeof(NN_NETFUNC));
    nfx->name = name;
    nfx->forward = forward;
    nfx->backward = backward;
    nfx->Rforward = Rforward;
    nfx->Rbackward = Rbackward;
    nfx->sanity = sanity;
    hash_insert(nfhash, nfx);
  }
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

NN_NETFUNC *nn_find_netfunc(char *name)
{
  NN_NETFUNC nf;
  
  if(nfhash == NULL) nfinit();
  nf.name = name;
  return(hash_search(nfhash, &nf));
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static void free_netfunc(void *nf)
{ 
  xfree(nf);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void nn_shutdown_netfuncs(void)
{
  if(nfhash)
    hash_do_func(nfhash, free_netfunc);
  hash_destroy(nfhash);
  nfhash = NULL;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static void nflinearf(NN *nn, NN_LINK *link, NN_LAYER *dst)
{
  NN_LAYER *src;
  unsigned i, j;
  double sum;

  src = link->source->layer;
  for(i = 0; i < link->numout; i++) {
    sum = 0;
    for(j = 0; j < link->numin; j++)
      sum += link->u[i][j] * src->y[j];
    dst->x[i] += sum + link->a[i];
  }
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static void nflinearb(NN *nn, NN_LINK *link, NN_LAYER *src)
{
  NN_LAYER *dst;
  unsigned i, j;
  double sum;

  dst = link->dest->layer;
  if(link->need_grads || nn->need_all_grads)
    for(i = 0; i < link->numout; i++) {
      for(j = 0; j < link->numin; j++)
	link->du[i][j] = dst->dx[i] * src->y[j];
      link->da[i] = dst->dx[i];
    }
  if(src->need_grads || nn->need_all_grads)
    for(j = 0; j < link->numin; j++) {
      sum = 0;
      for(i = 0; i < link->numout; i++)
	sum += dst->dx[i] * link->u[i][j];
      src->dy[j] += sum;
    }
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static void nflinearRf(NN *nn, NN_LINK *link, NN_LAYER *dst)
{
  NN_LAYER *src;
  unsigned i, j;
  double sum;

  src = link->source->layer;
  for(i = 0; i < link->numout; i++) {
    sum = 0;
    for(j = 0; j < link->numin; j++)
      sum += src->Ry[j] * link->u[i][j] +
	link->Ru[i][j] * src->y[j];
    dst->Rx[i] += sum + link->Ra[i];
  }
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static void nflinearRb(NN *nn, NN_LINK *link, NN_LAYER *src)
{
  NN_LAYER *dst;
  unsigned i, j;
  double sum;

  dst = link->dest->layer;
  for(i = 0; i < link->numout; i++) {
    for(j = 0; j < link->numin; j++)
      link->Rdu[i][j] = dst->Rdx[i] * src->y[j] +
	src->Ry[j] * dst->dx[i];
    link->Rda[i] = dst->Rdx[i];
  }
  for(j = 0; j < link->numin; j++) {
    sum = 0;
    for(i = 0; i < link->numout; i++)
      sum += dst->Rdx[i] * link->u[i][j] + link->Ru[i][j] * dst->dx[i];
    src->Rdy[j] += sum;
  }
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static int nflinears(NN *nn, NN_LAYERLIST *src, NN_LAYERLIST *dst,
		     unsigned *numin, unsigned *numout, unsigned *numaux)
{
  unsigned ssz, dsz;

  ssz = nn_layerlist_len(src);
  dsz = nn_layerlist_len(dst);
  if(ssz != 1 || dsz != 1)
    return(-1);
  *numin = src->layer->sz;
  *numout = dst->layer->sz;
  *numaux = 0;
  return(NN_WVECTOR | NN_WSCALAR);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static void nfdiagonalf(NN *nn, NN_LINK *link, NN_LAYER *dst)
{
  NN_LAYER *src;
  unsigned i, j;
  double sum;

  src = link->source->layer;
  for(i = 0; i < link->numout; i++) {
    sum = 0;
    for(j = 0; j < link->numin; j++)
      sum += link->u[i][j] * src->y[j] +
	link->v[i][j] * src->y[j] * src->y[j];
    dst->x[i] += sum + link->a[i];
  }
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static void nfdiagonalb(NN *nn, NN_LINK *link, NN_LAYER *src)
{
  NN_LAYER *dst;
  unsigned i, j;
  double sum;

  dst = link->dest->layer;
  if(link->need_grads || nn->need_all_grads)
    for(i = 0; i < link->numout; i++) {
      for(j = 0; j < link->numin; j++) {
	link->du[i][j] = dst->dx[i] * src->y[j];
	link->dv[i][j] = dst->dx[i] * src->y[j] * src->y[j];
      }
      link->da[i] = dst->dx[i];
    }
  if(src->need_grads || nn->need_all_grads)
    for(j = 0; j < link->numin; j++) {
      sum = 0;
      for(i = 0; i < link->numout; i++)
	sum += dst->dx[i] * 
	  (link->u[i][j] + 2.0 * link->v[i][j] * src->y[j]);
      src->dy[j] += sum;
    }
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static void nfdiagonalRf(NN *nn, NN_LINK *link, NN_LAYER *dst)
{
  NN_LAYER *src;
  unsigned i, j;
  double sum;

  src = link->source->layer;
  for(i = 0; i < link->numout; i++) {
    sum = 0;
    for(j = 0; j < link->numin; j++)
      sum += src->Ry[j] * link->u[i][j] +
	link->Ru[i][j] * src->y[j] +
	  2.0 * src->Ry[j] * src->y[j] * link->v[i][j] + 
	    src->y[j] * src->y[j] * link->Rv[i][j];
    dst->Rx[i] += sum + link->Ra[i];
  }
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static void nfdiagonalRb(NN *nn, NN_LINK *link, NN_LAYER *src)
{
  NN_LAYER *dst;
  unsigned i, j;
  double sum;

  dst = link->dest->layer;
  for(i = 0; i < link->numout; i++) {
    for(j = 0; j < link->numin; j++) {
      link->Rdu[i][j] = dst->Rdx[i] * src->y[j] +
	src->Ry[j] * dst->dx[i];
      link->Rdv[i][j] = dst->Rdx[i] * src->y[j] * src->y[j] +
	2.0 * src->Ry[j] * src->y[j] * dst->dx[i];
    }
    link->Rda[i] = dst->Rdx[i];
  }
  for(j = 0; j < link->numin; j++) {
    sum = 0;
    for(i = 0; i < link->numout; i++)
      sum += (link->u[i][j] + 2.0 * link->v[i][j] * src->y[j]) *
	dst->Rdx[i] + dst->dx[i] *
	  (2.0 * (link->Rv[i][j] * src->y[j] + link->v[i][j] *
		  src->Ry[j]) + link->Ru[i][j]);
    src->Rdy[j] += sum;
  }
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static int nfdiagonals(NN *nn, NN_LAYERLIST *src, NN_LAYERLIST *dst,
		       unsigned *numin, unsigned *numout, unsigned *numaux)
{
  unsigned ssz, dsz;

  ssz = nn_layerlist_len(src);
  dsz = nn_layerlist_len(dst);
  if(ssz != 1 || dsz != 1)
    return(-1);
  *numin = src->layer->sz;
  *numout = dst->layer->sz;
  *numaux = 0;
  return(NN_WVECTOR_1 | NN_WVECTOR_2 | NN_WSCALAR);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static void nfquadraticf(NN *nn, NN_LINK *link, NN_LAYER *dst)
{
  NN_LAYER *src;
  unsigned i, j, k;
  double sum;

  src = link->source->layer;
  for(i = 0; i < link->numout; i++) {
    sum = 0;
    for(j = 0; j < link->numin; j++) {
      for(k = 0; k < link->numin; k++)
	sum += link->A[i][j][k] * src->y[j] * src->y[k];
      sum += link->u[i][j] * src->y[j];
    }
    dst->x[i] += sum + link->a[i];
  }
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static void nfquadraticb(NN *nn, NN_LINK *link, NN_LAYER *src)
{
  NN_LAYER *dst;
  unsigned i, j, k;
  double sum1, sum2;

  dst = link->dest->layer;
  if(link->need_grads || nn->need_all_grads)
    for(i = 0; i < link->numout; i++) {
      for(j = 0; j < link->numin; j++) {
	link->du[i][j] = dst->dx[i] * src->y[j];
	for(k = 0; k < link->numin; k++)
	  link->dA[i][j][k] = dst->dx[i] * src->y[j] * src->y[k];
      }
      link->da[i] = dst->dx[i];
    }
  if(src->need_grads || nn->need_all_grads)
    for(j = 0; j < link->numin; j++) {
      sum1 = 0;
      for(i = 0; i < link->numout; i++) {
	sum2 = 0;
	for(k = 0; k < link->numin; k++)
	  sum2 += (link->A[i][j][k] + link->A[i][k][j]) * src->y[k];
	sum1 += (link->u[i][j] + sum2 - link->A[i][j][j] * src->y[j]) *
	  dst->dx[i];
      }
      src->dy[j] += sum1;
    }
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static void nfquadraticRf(NN *nn, NN_LINK *link, NN_LAYER *dst)
{
  NN_LAYER *src;
  unsigned i, j, k;
  double sum;

  src = link->source->layer;
  for(i = 0; i < link->numout; i++) {
    sum = 0;
    for(j = 0; j < link->numin; j++) {
      for(k = 0; k < link->numin; k++)
	sum += (src->y[j] * src->Ry[k] + src->Ry[j] * src->y[k]) *
	  link->A[i][j][k] + link->RA[i][j][k] * src->y[j] * src->y[k];
      sum += link->Ru[i][j] * src->y[j] + link->u[i][j] * src->Ry[j];
    }
    dst->Rx[i] += sum + link->Ra[i];
  }
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static void nfquadraticRb(NN *nn, NN_LINK *link, NN_LAYER *src)
{
  NN_LAYER *dst;
  unsigned i, j, k;
  double sum1, sum2, sum3;

  dst = link->dest->layer;
  for(i = 0; i < link->numout; i++) {
    for(j = 0; j < link->numin; j++) {
      link->Rdu[i][j] = dst->Rdx[i] * src->y[j] + dst->dx[i] * src->Ry[j];
      for(k = 0; k < link->numin; k++)
	link->RdA[i][j][k] = dst->Rdx[i] * src->y[j] * src->y[k] +
	  dst->dx[i] * (src->Ry[j] * src->y[k] + src->y[j] * src->Ry[k]);
    }
    link->Rda[i] = dst->Rdx[i];
  }
  for(j = 0; j < link->numin; j++) {
    sum1 = 0;
    for(i = 0; i < link->numout; i++) {
      sum2 = sum3 = 0;
      for(k = 0; k < link->numin; k++) {
	sum2 += src->y[k] * (link->RA[i][j][k] + link->RA[i][k][j]) +
	  src->Ry[k] * (link->A[i][j][k] + link->A[i][k][j]);
	sum3 += (link->A[i][j][k] + link->A[i][k][j]) * src->y[k];
      }
      sum1 += (link->u[i][j] + sum3 - link->A[i][j][j] * src->y[j]) *
	dst->Rdx[i] + dst->dx[i] *
	  (link->Ru[i][j] + sum2 - (link->A[i][j][j] * src->Ry[j] +
	   link->RA[i][j][j] * src->y[j]));
    }
    src->Rdy[j] += sum1;
  }
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static int nfquadratics(NN *nn, NN_LAYERLIST *src, NN_LAYERLIST *dst,
			unsigned *numin, unsigned *numout, unsigned *numaux)
{
  unsigned ssz, dsz;

  ssz = nn_layerlist_len(src);
  dsz = nn_layerlist_len(dst);
  if(ssz != 1 || dsz != 1)
    return(-1);
  *numin = src->layer->sz;
  *numout = dst->layer->sz;
  *numaux = 0;
  return(NN_WMATRIX | NN_WVECTOR | NN_WSCALAR);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static void nfeuclideanf(NN *nn, NN_LINK *link, NN_LAYER *dst)
{
  NN_LAYER *src;
  unsigned i, j;
  double sum, tmp;
  
  src = link->source->layer;
  for(i = 0; i < link->numout; i++) {
    sum = 0;
    for(j = 0; j < link->numin; j++) {
      tmp =  src->y[j] - link->u[i][j];
      sum += tmp * tmp;
    }
    dst->x[i] += sum / (2 * link->a[i] * link->a[i]);
  }
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static void nfeuclideanb(NN *nn, NN_LINK *link, NN_LAYER *src)
{
  NN_LAYER *dst;
  unsigned i, j;
  double sum;

  dst = link->dest->layer;
  if(link->need_grads || nn->need_all_grads)
    for(i = 0; i < link->numout; i++) {
      for(j = 0; j < link->numin; j++)
	link->du[i][j] = (1.0 / (link->a[i] * link->a[i])) *
	  (link->u[i][j] - src->y[j]) * dst->dx[i];
      link->da[i] = -1.0 * dst->dx[i] * dst->x[i] / link->a[i];
    }

  if(src->need_grads || nn->need_all_grads)
    for(j = 0; j < link->numin; j++) {
      sum = 0;
      for(i = 0; i < link->numout; i++)
	sum -= link->du[i][j];
      src->dy[j] += sum;
    }
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static void nfeuclideanRf(NN *nn, NN_LINK *link, NN_LAYER *dst)
{
  NN_LAYER *src;
  unsigned i, j;
  double sum1, sum2, tmp;

  src = link->source->layer;
  for(i = 0; i < link->numout; i++) {
    sum1 = sum2 = 0;
    for(j = 0; j < link->numin; j++) {
      tmp =  src->y[j] - link->u[i][j];
      sum1 += tmp * tmp;
      tmp = (src->Ry[j] - link->Ru[i][j]) * tmp;
      sum2 += tmp;
    }
    dst->Rx[i] += -link->Ra[i] * sum1 
      / (link->a[i] * link->a[i] * link->a[i]) + sum2
	/ (link->a[i] * link->a[i]);
  }
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static void nfeuclideanRb(NN *nn, NN_LINK *link, NN_LAYER *src)
{
  NN_LAYER *dst;
  unsigned i, j;
  double sum;

  dst = link->dest->layer;
  for(i = 0; i < link->numout; i++) {
    for(j = 0; j < link->numin; j++)
      link->Rdu[i][j] =  dst->Rdx[i] * (link->u[i][j] - src->y[j]) /
	(link->a[i] * link->a[i]) +  (1.0 / (link->a[i] * link->a[i])) *
	(-1.0 * link->Ra[i] * (link->u[i][j] - src->y[j]) / link->a[i] +
	 link->Ru[i][j] - src->Ry[j]) * dst->dx[i];
    
    link->Rda[i] = (1.0 / link->a[i]) *
      (-dst->Rdx[i] * dst->x[i] - dst->dx[i] * 
       (dst->Rx[i] - dst->x[i] * link->Ra[i] / link->a[i]));
  }
  for(j = 0; j < link->numin; j++) {
    sum = 0;
    for(i = 0; i < link->numout; i++)
      sum -= link->Rdu[i][j];
    src->Rdy[j] += sum;
  }
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static int nfeuclideans(NN *nn, NN_LAYERLIST *src, NN_LAYERLIST *dst,
			unsigned *numin, unsigned *numout, unsigned *numaux)
{
  unsigned ssz, dsz;

  ssz = nn_layerlist_len(src);
  dsz = nn_layerlist_len(dst);
  if(ssz != 1 || dsz != 1)
    return(-1);
  *numin = src->layer->sz;
  *numout = dst->layer->sz;
  *numaux = 0;
  return(NN_WVECTOR | NN_WSCALAR);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static void nfcopyf(NN *nn, NN_LINK *link, NN_LAYER *dst)
{
  NN_LAYER *src;
  unsigned i;

  src = link->source->layer;
  for(i = 0; i < link->numout; i++) 
    dst->x[i] += src->y[i];
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static void nfcopyb(NN *nn, NN_LINK *link, NN_LAYER *src)
{
  NN_LAYER *dst;
  unsigned i;

  dst = link->dest->layer;
  if(src->need_grads || nn->need_all_grads)
    for(i = 0; i < link->numout; i++)
      src->dy[i] += dst->dx[i];
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static void nfcopyRf(NN *nn, NN_LINK *link, NN_LAYER *dst)
{
  NN_LAYER *src;
  unsigned i;

  src = link->source->layer;
  for(i = 0; i < link->numout; i++)
    dst->Rx[i] += src->Ry[i];
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static void nfcopyRb(NN *nn, NN_LINK *link, NN_LAYER *src)
{
  NN_LAYER *dst;
  unsigned i;

  dst = link->dest->layer;

  for(i = 0; i < link->numout; i++)
    src->Rdy[i] += dst->Rdx[i];
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static int nfcopys(NN *nn, NN_LAYERLIST *src, NN_LAYERLIST *dst,
		   unsigned *numin, unsigned *numout, unsigned *numaux)
{
  unsigned ssz, dsz;

  ssz = nn_layerlist_len(src);
  dsz = nn_layerlist_len(dst);
  if(ssz != 1 || dsz != 1)
    return(-1);
  *numin = src->layer->sz;
  *numout = dst->layer->sz;
  if(*numin != *numout)
    return(-1);
  *numaux = 0;
  return(0);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static void nfkopyf(NN *nn, NN_LINK *link, NN_LAYER *dst)
{
  NN_LAYER *src;
  unsigned i;

  src = link->source->layer;
  for(i = 0; i < link->numout; i++) 
    dst->x[i] += src->y[i];
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static void nfkopyb(NN *nn, NN_LINK *link, NN_LAYER *src)
{
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static void nfkopyRf(NN *nn, NN_LINK *link, NN_LAYER *dst)
{
  NN_LAYER *src;
  unsigned i;

  src = link->source->layer;
  for(i = 0; i < link->numout; i++)
    dst->Rx[i] += src->Ry[i];
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static void nfkopyRb(NN *nn, NN_LINK *link, NN_LAYER *src)
{
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static int nfkopys(NN *nn, NN_LAYERLIST *src, NN_LAYERLIST *dst,
		   unsigned *numin, unsigned *numout, unsigned *numaux)
{
  unsigned ssz, dsz;

  ssz = nn_layerlist_len(src);
  dsz = nn_layerlist_len(dst);
  if(ssz != 1 || dsz != 1)
    return(-1);
  *numin = src->layer->sz;
  *numout = dst->layer->sz;
  if(*numin != *numout)
    return(-1);
  *numaux = 0;
  return(0);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static void nfscalarf(NN *nn, NN_LINK *link, NN_LAYER *dst)
{
  NN_LAYER *src;
  unsigned i;

  src = link->source->layer;
  for(i = 0; i < link->numout; i++)
    dst->x[i] += src->y[i] * link->a[i];
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static void nfscalarb(NN *nn, NN_LINK *link, NN_LAYER *src)
{
  NN_LAYER *dst;
  unsigned i;

  dst = link->dest->layer;
  if(link->need_grads || nn->need_all_grads)
    for(i = 0; i < link->numout; i++)
      link->da[i] = dst->dx[i] * src->y[i];
  if(src->need_grads || nn->need_all_grads)
    for(i = 0; i < link->numin; i++)
      src->dy[i] += dst->dx[i] * link->a[i];
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static void nfscalarRf(NN *nn, NN_LINK *link, NN_LAYER *dst)
{
  NN_LAYER *src;
  unsigned i;

  src = link->source->layer;
  for(i = 0; i < link->numout; i++)
    dst->Rx[i] += src->Ry[i] * link->a[i] + src->y[i] * link->Ra[i];
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static void nfscalarRb(NN *nn, NN_LINK *link, NN_LAYER *src)
{
  NN_LAYER *dst;
  unsigned i;

  dst = link->dest->layer;
  for(i = 0; i < link->numout; i++)
    link->Rda[i] = dst->Rdx[i] * src->y[i] + dst->dx[i] * src->Ry[i];
  for(i = 0; i < link->numin; i++)
    src->Rdy[i] += dst->Rdx[i] * link->a[i] + dst->dx[i] * link->Ra[i];
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static int nfscalars(NN *nn, NN_LAYERLIST *src, NN_LAYERLIST *dst,
		   unsigned *numin, unsigned *numout, unsigned *numaux)
{
  unsigned ssz, dsz;

  ssz = nn_layerlist_len(src);
  dsz = nn_layerlist_len(dst);
  if(ssz != 1 || dsz != 1)
    return(-1);
  *numin = src->layer->sz;
  *numout = dst->layer->sz;
  if(*numin != *numout)
    return(-1);
  *numaux = 0;
  return(NN_WSCALAR);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static void nfproductf(NN *nn, NN_LINK *link, NN_LAYER *dst)
{
  NN_LAYER *src1, *src2;
  unsigned i;

  src1 = link->source->layer;
  src2 = link->source->cdr->layer;
  for(i = 0; i < link->numout; i++)
    dst->x[i] += src1->y[i] * src2->y[i];
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static void nfproductb(NN *nn, NN_LINK *link, NN_LAYER *src)
{
  NN_LAYER *src1, *src2, *dst;
  unsigned i;

  src1 = link->source->layer;
  src2 = link->source->cdr->layer;
  dst = link->dest->layer;

  if(src1->need_grads || src2->need_grads || nn->need_all_grads)
  for(i = 0; i < link->numin; i++) {
    src1->dy[i] += dst->dx[i] * src2->y[i];
    src2->dy[i] += dst->dx[i] * src1->y[i];
  }
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static void nfproductRf(NN *nn, NN_LINK *link, NN_LAYER *dst)
{
  NN_LAYER *src1, *src2;
  unsigned i;

  src1 = link->source->layer;
  src2 = link->source->cdr->layer;
  for(i = 0; i < link->numout; i++)
    dst->Rx[i] += src1->Ry[i] * src2->y[i] + src2->Ry[i] * src1->y[i];
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static void nfproductRb(NN *nn, NN_LINK *link, NN_LAYER *src)
{
  NN_LAYER *src1, *src2, *dst;
  unsigned i;

  src1 = link->source->layer;
  src2 = link->source->cdr->layer;
  dst = link->dest->layer;

  for(i = 0; i < link->numin; i++) {
    src1->dy[i] += dst->Rdx[i] * src2->y[i] + dst->dx[i] * src2->Ry[i];
    src2->dy[i] += dst->Rdx[i] * src1->y[i] + dst->dx[i] * src1->Ry[i];
  }
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static int nfproducts(NN *nn, NN_LAYERLIST *src, NN_LAYERLIST *dst,
		   unsigned *numin, unsigned *numout, unsigned *numaux)
{
  unsigned ssz, dsz;

  ssz = nn_layerlist_len(src);
  dsz = nn_layerlist_len(dst);
  if(ssz != 2 || dsz != 1)
    return(-1);
  *numin = src->layer->sz;
  *numout = dst->layer->sz;
  if(*numin != *numout || *numin != src->cdr->layer->sz)
    return(-1);
  *numaux = 0;
  return(0);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static void nfnormf(NN *nn, NN_LINK *link, NN_LAYER *dst)
{
  NN_LAYER *src;
  unsigned i;
  double norm;

  src = link->source->layer;
  norm = 0;
  for(i = 0; i < link->numin; i++)
    norm += src->y[i];
  for(i = 0; i < link->numout; i++)
    dst->x[i] += src->y[i] / norm;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static void nfnormb(NN *nn, NN_LINK *link, NN_LAYER *src)
{
  NN_LAYER *dst;
  unsigned i, j;
  double norm, sum;

  if(src->need_grads || nn->need_all_grads) {
    dst = link->dest->layer;
    norm = 0;
    for(i = 0; i < link->numin; i++)
      norm += src->y[i];
    for(i = 0; i < link->numin; i++) {
      sum = 0;
      for(j = 0; j < link->numout; j++)
	if(i != j) sum += dst->dx[j] * dst->x[j];
      src->dy[i] = ((1 - dst->x[i]) * dst->dx[i] - sum) / norm;
    }
  }
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static void nfnormRf(NN *nn, NN_LINK *link, NN_LAYER *dst)
{
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static void nfnormRb(NN *nn, NN_LINK *link, NN_LAYER *src)
{
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static int nfnorms(NN *nn, NN_LAYERLIST *src, NN_LAYERLIST *dst,
		     unsigned *numin, unsigned *numout, unsigned *numaux)
{
  unsigned ssz, dsz;

  ssz = nn_layerlist_len(src);
  dsz = nn_layerlist_len(dst);
  if(ssz != 1 || dsz != 1)
    return(-1);
  *numin = src->layer->sz;
  *numout = dst->layer->sz;
  if(*numin != *numout)
    return(-1);
  *numaux = 0;
  return(0);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static void nfaliasf(NN *nn, NN_LINK *link, NN_LAYER *dst)
{
  NN_LAYERLIST *l;
  NN_LAYER *src;
  unsigned i, j;
  double sum;

  for(l = link->source; l != NULL; l = l->cdr) {
    src = l->layer;
    for(i = 0; i < link->numout; i++) {
      sum = 0;
      for(j = 0; j < link->numin; j++)
	sum += link->u[i][j] * src->y[j];
      dst->x[i] += sum + link->a[i];
    }
  }
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static void nfaliasb(NN *nn, NN_LINK *link, NN_LAYER *src)
{
  NN_LAYERLIST *l;
  NN_LAYER *dst;
  unsigned i, j;
  double sum;

  for(l = link->dest; l != NULL; l = l->cdr) {
    dst = l->layer;

    if(link->need_grads || nn->need_all_grads)
      for(i = 0; i < link->numout; i++) {
	for(j = 0; j < link->numin; j++)
	  link->du[i][j] = dst->dx[i] * src->y[j];
	link->da[i] = dst->dx[i];
      }

    if(src->need_grads || nn->need_all_grads)
      for(j = 0; j < link->numin; j++) {
	sum = 0;
	for(i = 0; i < link->numout; i++)
	  sum += dst->dx[i] * link->u[i][j];
	src->dy[j] += sum;
      }
  }
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static void nfaliasRf(NN *nn, NN_LINK *link, NN_LAYER *dst)
{
  NN_LAYERLIST *l;
  NN_LAYER *src;
  unsigned i, j;
  double sum;

  for(l = link->source; l != NULL; l = l->cdr) {
    src = l->layer;
    for(i = 0; i < link->numout; i++) {
      sum = 0;
      for(j = 0; j < link->numin; j++)
	sum += src->Ry[j] * link->u[i][j] +
	  link->Ru[i][j] * src->y[j];
      dst->Rx[i] += sum + link->Ra[i];
    }
  }
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static void nfaliasRb(NN *nn, NN_LINK *link, NN_LAYER *src)
{
  NN_LAYERLIST *l;
  NN_LAYER *dst;
  unsigned i, j;
  double sum;

  for(l = link->dest; l != NULL; l = l->cdr) {
    dst = l->layer;
    for(i = 0; i < link->numout; i++) {
      for(j = 0; j < link->numin; j++)
	link->Rdu[i][j] = dst->Rdx[i] * src->y[j] +
	  src->Ry[j] * dst->dx[i];
      link->Rda[i] = dst->Rdx[i];
    }
    for(j = 0; j < link->numin; j++) {
      sum = 0;
      for(i = 0; i < link->numout; i++)
	sum += dst->Rdx[i] * link->u[i][j] + link->Ru[i][j] * dst->dx[i];
      src->Rdy[j] += sum;
    }
  }
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static int nfaliass(NN *nn, NN_LAYERLIST *src, NN_LAYERLIST *dst,
		    unsigned *numin, unsigned *numout, unsigned *numaux)
{
  NN_LAYERLIST *l;
  unsigned ssz, dsz, sz;

  ssz = nn_layerlist_len(src);
  dsz = nn_layerlist_len(dst);
  if(ssz == 1 && dsz > 1) {
    sz = dst->layer->sz;
    for(l = dst->cdr; l != NULL; l = l->cdr)
      if(l->layer->sz != sz)
	return(-1);
  }
  else if(dsz == 1 && ssz > 1) {
    sz = src->layer->sz;
    for(l = src->cdr; l != NULL; l = l->cdr)
      if(l->layer->sz != sz)
	return(-1);
  }
  else
    return(-1);

  *numin = src->layer->sz;
  *numout = dst->layer->sz;
  *numaux = 0;
  return(NN_WVECTOR | NN_WSCALAR);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static void nfunitminusf(NN *nn, NN_LINK *link, NN_LAYER *dst)
{
  NN_LAYER *src;
  unsigned i;

  src = link->source->layer;
  dst = link->dest->layer;
  for(i = 0; i < link->numout; i++) 
    dst->x[i] += (1 - src->y[i]);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static void nfunitminusb(NN *nn, NN_LINK *link, NN_LAYER *src)
{
  NN_LAYER *dst;
  unsigned i;

  dst = link->dest->layer;

  if(src->need_grads || nn->need_all_grads)
    for(i = 0; i < link->numout; i++)
      src->dy[i] -= dst->dx[i];
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static void nfunitminusRf(NN *nn, NN_LINK *link, NN_LAYER *dst)
{
  NN_LAYER *src;
  unsigned i;

  src = link->source->layer;
  dst = link->dest->layer;
  for(i = 0; i < link->numout; i++)
    dst->Rx[i] -= src->Ry[i];
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static void nfunitminusRb(NN *nn, NN_LINK *link, NN_LAYER *src)
{
  NN_LAYER *dst;
  unsigned i;

  dst = link->dest->layer;
  for(i = 0; i < link->numout; i++)
    src->Rdy[i] -= dst->Rdx[i];
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static int nfunitminuss(NN *nn, NN_LAYERLIST *src, NN_LAYERLIST *dst,
		   unsigned *numin, unsigned *numout, unsigned *numaux)
{
  unsigned ssz, dsz;

  ssz = nn_layerlist_len(src);
  dsz = nn_layerlist_len(dst);
  if(ssz != 1 || dsz != 1)
    return(-1);
  *numin = src->layer->sz;
  *numout = dst->layer->sz;
  if(*numin != *numout)
    return(-1);
  *numaux = 0;
  return(0);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static int nfcmp(const void *a, const void *b, void *obj)
{
  const NN_NETFUNC *x = a, *y = b;
  
  return((x->name[0] < y->name[0]) ? -1 :
	 (x->name[0] > y->name[0]) ? 1 : 0);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static unsigned long nfnumify(const void *a, void *obj)
{
  const NN_NETFUNC *x = a;

  return(x->name[0]);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static void nfinit(void)
{
  if(nfhash == NULL) {
    nfhash = hash_create(16, nfnumify, nfcmp, NULL);
    nn_register_netfunc("alias", nfaliasf, nfaliasb,
			nfaliasRf, nfaliasRb, nfaliass);
    nn_register_netfunc("linear", nflinearf, nflinearb,
			nflinearRf, nflinearRb, nflinears);
    nn_register_netfunc("diagonal", nfdiagonalf, nfdiagonalb,
			nfdiagonalRf, nfdiagonalRb, nfdiagonals);
    nn_register_netfunc("quadratic", nfquadraticf, nfquadraticb,
			nfquadraticRf, nfquadraticRb, nfquadratics);
    nn_register_netfunc("euclidean", nfeuclideanf, nfeuclideanb,
			nfeuclideanRf, nfeuclideanRb, nfeuclideans);
    nn_register_netfunc("copy", nfcopyf, nfcopyb,
			nfcopyRf, nfcopyRb, nfcopys);
    nn_register_netfunc("kopy", nfkopyf, nfkopyb,
			nfkopyRf, nfkopyRb, nfkopys);
    nn_register_netfunc("scalar", nfscalarf, nfscalarb,
			nfscalarRf, nfscalarRb, nfscalars);
    nn_register_netfunc("product", nfproductf, nfproductb,
			nfproductRf, nfproductRb, nfproducts);
    nn_register_netfunc("norm", nfnormf, nfnormb,
			nfnormRf, nfnormRb, nfnorms);
    nn_register_netfunc("unitminus", nfunitminusf, nfunitminusb,
			nfunitminusRf, nfunitminusRb, nfunitminuss);
  }
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


