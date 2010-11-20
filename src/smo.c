
/* Copyright (c) 1998 by G. W. Flake. */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <limits.h>

#include "nodelib/xalloc.h"
#include "nodelib/misc.h"
#include "nodelib/dsmethod.h"
#include "nodelib/series.h"
#include "nodelib/dsfile.h"


#define SVM_OWNER
#include "nodelib/svm.h"
#undef SVM_OWNER

#define MAX(a,b) (((a)>(b))?(a):(b))
#define MIN(a,b) (((a)<(b))?(a):(b))
#define SGN(a)   (((a)<0)?-1:((a)>0)?1:0)

#define NODE_INDEX(SMORCH, DATA) \
  ((unsigned)(((double *)(DATA)) - (SMORCH)->alpha))

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static SMORCH_CACHE *smorch_cache_create(unsigned size, unsigned limit)
{
  unsigned i, j, step;
  SMORCH_CACHE *cache;
  double *vals;
  char *chars;

  cache = xmalloc(sizeof(SMORCH_CACHE));

  cache->vals = xmalloc(sizeof(double *) * (size - 1));
  cache->status = xmalloc(sizeof(char *) * (size - 1));

  cache->index = xmalloc(sizeof(unsigned) * limit);
  cache->invindex = xmalloc(sizeof(unsigned) * size);
  cache->nodes = xmalloc(sizeof(LIST_NODE) * size);

  vals = xmalloc(sizeof(double) * size * (size - 1) / 2);
  chars = xmalloc(sizeof(char) * size * (size - 1) / 2);

  cache->size = size;
  cache->limit = limit;
  cache->used = 0;
  cache->notickle = 0;

  step = 0;
  for (i = 0; i < size - 1; i++) {
    step += i;
    cache->vals[i] = vals + step;
    cache->status[i] = chars + step;
    for (j = 0; j < i + 1; j++)
      cache->status[i][j] = 0;
  }
  
  cache->order = list_create();
  cache->free = list_create();
  for (i = 0; i < size; i++) {
    cache->nodes[i].next = cache->nodes[i].prev = NULL;
    cache->nodes[i].data = &cache->invindex[i];
    list_insert_tail(cache->free, &cache->nodes[i]);
  }

  for (i = 0; i < limit; i++)
    cache->index[i] = limit;
  for (i = 0; i < size; i++)
    cache->invindex[i] = limit;

  return cache;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static void smorch_cache_destroy(SMORCH_CACHE *cache)
{
  xfree(cache->vals[0]);
  xfree(cache->status[0]);

  xfree(cache->vals);
  xfree(cache->status);
  xfree(cache->index);
  xfree(cache->invindex);
  xfree(cache->nodes);

  list_destroy(cache->order);
  list_destroy(cache->free);
  xfree(cache);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* Return values: -1 = not in and shouldn't be, 0 = should be but not
   present, 1 = should be and is. */

static int smorch_cache_query(SMORCH_CACHE *cache, unsigned i, unsigned j)
{
  unsigned ci, cj, swap;

  if (i == j)
    return -1;
  else if (cache->index[i] < cache->limit &&
	   cache->index[j] < cache->limit) {
    ci = cache->index[i];
    cj = cache->index[j];
    if (cj > ci) {
      swap = cj; cj = ci; ci = swap;
    }
    if (cache->status[ci - 1][cj])
      return 1;
    else
      return 0;
  }
  else
    return -1;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* Assumes that i or j is not already in the cache. */

static void smorch_cache_insert(SMORCH_CACHE *cache, unsigned i, unsigned j,
			     double value)
{
  unsigned k, ci, cj, swap;
  LIST_NODE *node;

  if (cache->index[i] == cache->limit) { /* not present. */
    swap = 0;
    if (cache->used < cache->size) {
      node = list_remove_head(cache->free);
      (cache->used)++;
    }
    else {
      node = list_remove_head(cache->order);
      swap = 1;
    }
    ci = (unsigned) (((unsigned *)node->data) - cache->invindex);
    /* ci is the location in the cache that is to be overwritten. */
    for (k = 0; k < ci; k++)
      cache->status[ci - 1][k] = 0;
    for (k = ci + 1; k < cache->size; k++)
      cache->status[k - 1][ci] = 0;
    if (swap) cache->index[cache->invindex[ci]] = cache->limit;
    cache->index[i] = ci;
    cache->invindex[ci] = i;
    list_insert_tail(cache->order, node);
  }
  else {
    ci = cache->index[i];
    list_remove_node(cache->order, &cache->nodes[ci]);
    list_insert_tail(cache->order, &cache->nodes[ci]);
  }

  if (cache->index[j] == cache->limit) { /* not present. */
    swap = 0;
    if (cache->used < cache->size) {
      node = list_remove_head(cache->free);
      (cache->used)++;
    }
    else {
      node = list_remove_head(cache->order);
      swap = 1;
    }
    cj = (unsigned) (((unsigned *)node->data) - cache->invindex);
    /* cj is the location in the cache that is to be overwritten. */
    for (k = 0; k < cj; k++)
      cache->status[cj - 1][k] = 0;
    for (k = cj + 1; k < cache->size; k++)
      cache->status[k - 1][cj] = 0;
    if (swap) cache->index[cache->invindex[cj]] = cache->limit;
    cache->index[j] = cj;
    cache->invindex[cj] = j;
    list_insert_tail(cache->order, node);
  }
  else {
    cj = cache->index[j];
    list_remove_node(cache->order, &cache->nodes[cj]);
    list_insert_tail(cache->order, &cache->nodes[cj]);
  }

  /* ci and cj now refer to the cache entries that should be written. */
  if (cj > ci) {
    swap = cj; cj = ci; ci = swap;
  }
  cache->status[ci - 1][cj] = 1;
  cache->vals[ci - 1][cj] = value;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* Assumes i != j and that the value is in the cache.  Returns the value
   but doesn't re-prioritize indices. */

static double smorch_cache_simple_access(SMORCH_CACHE *cache, unsigned i,
				      unsigned j)
{
  unsigned ci, cj, swap;

  ci = cache->index[i];
  cj = cache->index[j];
  if (cj > ci) {
    swap = cj; cj = ci; ci = swap;
  }
  return cache->vals[ci - 1][cj];  
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* Assumes i != j and that the value is in the cache.  Returns the value
   and re-prioritizes the ordering. */

static double smorch_cache_access(SMORCH_CACHE *cache, unsigned i, unsigned j)
{
  unsigned ci, cj, swap;

  ci = cache->index[i];
  cj = cache->index[j];
  if (cj > ci) {
    swap = cj; cj = ci; ci = swap;
  }
  if (!cache->notickle) {
    list_remove_node(cache->order, &cache->nodes[ci]);
    list_insert_tail(cache->order, &cache->nodes[ci]);
    list_remove_node(cache->order, &cache->nodes[cj]);
    list_insert_tail(cache->order, &cache->nodes[cj]);
  }
  return cache->vals[ci - 1][cj];  
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static double smorch_kernel_value(SMORCH *smorch, unsigned i, unsigned j, 
			       LIST_NODE **snode, int donttickle)
{
  volatile double val = 0.0;
  int status;
  
  if (i == j) {
    (smorch->cache_hit)++;
    val = smorch->kii[i];
  }
  else if (smorch->cache == NULL) {
    if (!smorch->safe_ds_pointers) {
      dataset_x_copy(smorch->data, i, smorch->temp);
      val = smorch->kernel(smorch->temp, dataset_x(smorch->data, j),
			smorch->aux, smorch->xdim);
    }
    else
      val = smorch->kernel(dataset_x(smorch->data, i), dataset_x(smorch->data, j),
			smorch->aux, smorch->xdim);
    (smorch->cache_avoid)++;
  }
  else if (smorch->examine_all ||
	   (smorch->subset_size == 0 && smorch->cache->size < smorch->sz &&
	    (snode[i] == NULL && snode[j] == NULL)) ||
	   smorch->cache->size < smorch->subset_size) {
    if (smorch_cache_query(smorch->cache, i, j) == 1) {
      (smorch->cache_hit)++;
      val = smorch_cache_simple_access(smorch->cache, i, j);
    }
    else {
      if (!smorch->safe_ds_pointers) {
	dataset_x_copy(smorch->data, i, smorch->temp);
	val = smorch->kernel(smorch->temp, dataset_x(smorch->data, j),
			  smorch->aux, smorch->xdim);
      }
      else
	val = smorch->kernel(dataset_x(smorch->data, i), dataset_x(smorch->data, j),
			  smorch->aux, smorch->xdim);
      (smorch->cache_avoid)++;
    }
  }
  else {
    status = smorch_cache_query(smorch->cache, i, j);
    if (status == 1) {
      (smorch->cache_hit)++;
      if (donttickle)
	val = smorch_cache_simple_access(smorch->cache, i, j);
      else
	val = smorch_cache_access(smorch->cache, i, j);
    }
    else {
      (smorch->cache_miss)++;
      if (!smorch->safe_ds_pointers) {
	dataset_x_copy(smorch->data, i, smorch->temp);
	val = smorch->kernel(smorch->temp, dataset_x(smorch->data, j),
			  smorch->aux, smorch->xdim);
      }
      else
	val = smorch->kernel(dataset_x(smorch->data, i), dataset_x(smorch->data, j),
			  smorch->aux, smorch->xdim);
      if (!donttickle)
	smorch_cache_insert(smorch->cache, i, j, val);
    }
  }
  return val;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static double kkt_badness(SMORCH *smorch, double y, double f, double a)
{
  double r = f - y, e, aa;
  double C = smorch->C;

  if (!smorch->regression) {
    r = r * y;
    if (r < -smorch->tol && a < C)
      return -r + (C - a) / C;
    else if (r > smorch->tol && a > 0)
      return r + a / C;
  }
  else {
    r = fabs(r);
    e = (r < smorch->regeps) ? 0 : r - smorch->regeps;
    aa = fabs(a);
    if (r < smorch->regeps - smorch->tol && aa > 0)
      return smorch->regeps - r + aa / C;
    else if (r > smorch->regeps + smorch->tol && aa < C)
      return r - smorch->regeps + (C - aa) / C;
  }
  return 0.0;
}


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static void smorch_update_alpha(SMORCH *smorch, unsigned index, double alpha,
				LIST *slist, LIST_NODE **snode, double newb)
{
  unsigned old_nonbound, new_nonbound, old_nonzero, new_nonzero;
  double old_alpha;
  double C = smorch->C;
  LIST_NODE *node;

  old_alpha = smorch->alpha[index];

  if (!smorch->regression) {
    old_nonzero = (old_alpha > 0) ? 1 : 0;
    new_nonzero = (alpha > 0) ? 1 : 0;

    old_nonbound = (old_nonzero && old_alpha < C) ? 1 : 0;
    new_nonbound = (new_nonzero && alpha < C) ? 1 : 0;
  }
  else {
    old_nonzero = (old_alpha != 0.0) ? 1 : 0;
    new_nonzero = (alpha != 0.0) ? 1 : 0;

    old_nonbound = (old_nonzero && old_alpha < C
		    && old_alpha > -C) ? 1 : 0;
    new_nonbound = (new_nonzero && alpha < C
		    && alpha > -C) ? 1 : 0;
  }

  if (smorch->ultra_clever) {
    smorch->delta_alpha[smorch->time % smorch->sz] = alpha - old_alpha;
    smorch->alpha_index[smorch->time % smorch->sz] = index;
    smorch->store_b[smorch->time % smorch->sz] = smorch->b;
    (smorch->time)++;
  }
  else {
    LIST *list;
    LIST_NODE *node;
    unsigned i;
    double kout;
    
    (smorch->time)++;
    list = slist ? slist : smorch->nonbound;
    smorch->total_output_kern += list->count;
    if (!smorch->regression) {
      for (node = list->head; node != NULL; node = node->next) {
	i = NODE_INDEX(smorch, node->data);
	if (smorch->update_time[i] != smorch->time - 1) continue;
	kout = smorch_kernel_value(smorch, index, i, 
				(snode ? snode : smorch->nonbound_node), 0);
	smorch->out[i] += smorch->y[index] * kout * (alpha - old_alpha) +
	  smorch->b - newb;
	smorch->update_time[i] = smorch->time;
      }
    }
    else {
      for (node = list->head; node != NULL; node = node->next) {
	i = NODE_INDEX(smorch, node->data);
	if (smorch->update_time[i] != smorch->time - 1) continue;
	kout = smorch_kernel_value(smorch, index, i, 
				(snode ? snode : smorch->nonbound_node), 0);
	smorch->out[i] += kout * (alpha - old_alpha) + smorch->b - newb;
	smorch->update_time[i] = smorch->time;
      }
    }
  }

  smorch->alpha[index] = alpha;
  
  /* Handle nonzero list. */
  if (old_nonzero < new_nonzero) {
    /* Need insert into list. */
    if((node = list_remove_head(smorch->freelist)))
      node->data = &smorch->alpha[index];
    else
      node = list_node_create(&smorch->alpha[index]);
    list_insert_tail(smorch->nonzero, node);
    smorch->nonzero_node[index] = node;
  }
  else if (old_nonzero > new_nonzero) {
    /* Need removal from list. */
    node = smorch->nonzero_node[index];
    list_remove_node(smorch->nonzero, node);
    smorch->nonzero_node[index] = NULL;
    list_insert_head(smorch->freelist, node);
  }

  /* Handle nonbound list and slist. */
  if (old_nonbound < new_nonbound) {
    /* Need insert into list. */
    if((node = list_remove_head(smorch->freelist)))
      node->data = &smorch->alpha[index];
    else
      node = list_node_create(&smorch->alpha[index]);
    list_insert_tail(smorch->nonbound, node);
    smorch->nonbound_node[index] = node;

    if (slist) {
      if((node = list_remove_head(smorch->freelist)))
	node->data = &smorch->alpha[index];
      else
	node = list_node_create(&smorch->alpha[index]);
      list_insert_tail(slist, node);
      snode[index] = node;
    }
  }
  else if (old_nonbound > new_nonbound) {
    /* Need removal from list. */
    node = smorch->nonbound_node[index];
    list_remove_node(smorch->nonbound, node);
    smorch->nonbound_node[index] = NULL;
    list_insert_head(smorch->freelist, node);
    
    if (slist && snode[index]) {
      node = snode[index];
      list_remove_node(slist, node);
      snode[index] = NULL;
      list_insert_head(smorch->freelist, node);
    }
  }
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static double smorch_output(SMORCH *smorch, unsigned index,
			    LIST_NODE **snode)
{
  unsigned i, j, start;
  double kout;

  start = smorch->update_time[index];
  if (smorch->update_time[index] == smorch->time)
    return smorch->out[index];
  else if (smorch->ultra_clever &&
	   (smorch->time - start < smorch->nonzero->count)) {
    smorch->total_output_kern += (smorch->time - start);
    /* incremental output is faster, so do it. */
    if (!smorch->regression) {
      for (i = start; i < smorch->time; i++) {
	j = smorch->alpha_index[i % smorch->sz];
	if (smorch->delta_alpha[i % smorch->sz] == 0.0)
	  continue;
	kout = smorch_kernel_value(smorch, index, j, 
				(snode ? snode : smorch->nonbound_node), 0);
	smorch->out[index] += smorch->y[j] * kout * 
	  smorch->delta_alpha[i % smorch->sz];
      }
    }
    else {
      for (i = start; i < smorch->time; i++) {
	j = smorch->alpha_index[i % smorch->sz];
	if (smorch->delta_alpha[i % smorch->sz] == 0.0)
	  continue;
	kout = smorch_kernel_value(smorch, index, j, 
				(snode ? snode : smorch->nonbound_node), 0);
	smorch->out[index] += kout * smorch->delta_alpha[i % smorch->sz];
      }
    }
    smorch->out[index] += smorch->store_b[start % smorch->sz] - smorch->b;
  }
  else {
    /* do the hard way. */
    LIST_NODE *node;
    smorch->total_output_kern += smorch->nonzero->count;
    smorch->out[index] = 0.0;
    if (!smorch->regression) {
      for (node = smorch->nonzero->head, i = 0; node != NULL;
	   node = node->next, i++) {
	j = NODE_INDEX(smorch, node->data);
	kout = smorch_kernel_value(smorch, index, j, 
				(snode ? snode : smorch->nonbound_node), 0);
	smorch->out[index] += smorch->y[j] * smorch->alpha[j] * kout;
      }
    }
    else {
      for (node = smorch->nonzero->head, i = 0; node != NULL;
	   node = node->next, i++) {
	j = NODE_INDEX(smorch, node->data);
	kout = smorch_kernel_value(smorch, index, j, 
				(snode ? snode : smorch->nonbound_node), 0);
	smorch->out[index] += smorch->alpha[j] * kout;
      }
    }
    smorch->out[index] -= smorch->b;
  }
  smorch->update_time[index] = smorch->time;
  return smorch->out[index];
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static int smorch_worst_kkt(SMORCH *smorch, LIST *list, LIST_NODE **snode)
{
  int i, worsti = -1;
  double worstkkt = 0, kkt, y, alpha, E, f, fabsalpha;
  LIST_NODE *node;

  for (node = list->head; node != NULL; node = node->next) {
    i = NODE_INDEX(smorch, node->data);
    y = smorch->y[i];
    alpha = smorch->alpha[i];
    fabsalpha = fabs(alpha);
    f = smorch_output(smorch, i, snode);
    E = f - y;
    kkt = kkt_badness(smorch, y, f, smorch->alpha[i]);
    if (kkt > worstkkt) {
      worstkkt = kkt;
      worsti = i;
    }
  }
  return worsti;    
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static int smorch_calculate_step(SMORCH *smorch, int i1, int i2, LIST *slist,
				 LIST_NODE **snode, double *_newa1,
				 double *_newa2, double *_newb, double *_dobj)
{
  double a1, a2 = 0, alpha1, alpha2, y1, y2, E1, E2, s, k11, k12, k22;
  double b1, b2, L, H, Lobj, Hobj, eta, oldb, newb;
  double newu1, newu2, newE1, newE2;
  double f1, f2;
  
  if (i1 == i2) return 0;
  
  alpha1 = smorch->alpha[i1];
  f1 = smorch_output(smorch, i1, snode);
  y1 = smorch->y[i1];
  E1 = f1 - y1;
  
  alpha2 = smorch->alpha[i2];
  f2 = smorch_output(smorch, i2, snode);
  y2 = smorch->y[i2];
  E2 = f2 - y2;
  
  k11 = smorch_kernel_value(smorch, i1, i1,
			 (snode ? snode : smorch->nonbound_node), 0);
  k12 = smorch_kernel_value(smorch, i1, i2,
			 (snode ? snode : smorch->nonbound_node), 0);
  k22 = smorch_kernel_value(smorch, i2, i2,
			 (snode ? snode : smorch->nonbound_node), 0);

  eta = k11 + k22 - 2 * k12;

  if (!smorch->regression) {
    double C = smorch->C;
    s = y1 * y2;
    if (y1 != y2) {
      L = MAX(0, alpha2 - alpha1);
      H = MIN(C, C + alpha2 - alpha1);
    }
    else {
      L = MAX(0, alpha2 + alpha1 - C);
      H = MIN(C, alpha2 + alpha1);
    }

    if (eta > 0) {
      a2 = alpha2 + y2 * (E1 - E2) / eta;
      if (a2 < L) a2 = L;
      else if (a2 > H) a2 = H;
    }
    else {
      double f1, f2, L1, H1;
      f1 = y1 * (E1 + smorch->b) - alpha1 * k11 - s * alpha2 * k12;
      f2 = y2 * (E2 + smorch->b) - s * alpha1 * k12 - alpha2 * k22;
      L1 = alpha1 + s * (alpha2 - L);
      H1 = alpha1 + s * (alpha2 - H);
      Lobj = L1 * f1 + L * f2 + 0.5 * L1 * L1 * k11 + 0.5 * L * L * k22
	+ s * L * L1 * k12;
      Hobj = H1 * f1 + H * f2 + 0.5 * H1 * H1 * k11 + 0.5 * H * H * k22
	+ s * H * H1 * k12;
      if (Lobj < Hobj - smorch->eps) a2 = L;
      else if (Lobj > Hobj + smorch->eps) a2 = H;
      else a2 = alpha2;
    }
    
    if (fabs(a2 - alpha2) < smorch->eps * (a2 + alpha2 + smorch->eps))
      return 0;
    
    a1 = alpha1 + s * (alpha2 - a2);
    
    b1 = E1 + y1 * (a1 - alpha1) * k11 + y2 * (a2 - alpha2) * k12 + smorch->b;
    b2 = E2 + y1 * (a1 - alpha1) * k12 + y2 * (a2 - alpha2) * k22 + smorch->b;
    
    oldb = smorch->b;
    newb = 0.5 * (b1 + b2);
    
    newu1 = y1 + b1 - newb;
    newu2 = y2 + b2 - newb;
    
    *_dobj =
      (a1 * y1 * (newu1 - 0.5 * y1 * a1 * k11 + newb) +
       a2 * y2 * (newu2 - 0.5 * y2 * a2 * k22 + newb) -
       s * a1 * a2 * k12 - a1 - a2)
      -
      (alpha1 * y1 * (E1 + y1 - 0.5 * y1 * alpha1 * k11 + oldb) +
       alpha2 * y2 * (E2 + y2 - 0.5 * y2 * alpha2 * k22 + oldb) -
       s * alpha1 * alpha2 * k12 - alpha1 - alpha2) ;

    *_newa1 = a1;
    *_newa2 = a2;
    *_newb = newb;
    return 1;
  }

#define STEP(x) (((x)<0.0)?0.0:1.0)

  /* Begin code for regression .... */

  else if (eta > 0) {
    double dobj = 0, C = smorch->C;
    double sum = alpha1 + alpha2;
    double delta = 2 * smorch->regeps / eta;

    a2 = alpha2 + (E1 - E2) / eta;
    a1 = sum - a2;
    if (a1 * a2 < 0) {
      if (fabs(a2) >= delta || fabs(a1) >= delta)
	a2 = a2 - SGN(a2) * delta;
      else
	a2 = STEP(fabs(a2) - fabs(a1)) * sum;
    }
    L = MAX(sum - smorch->C, -smorch->C);
    H = MIN(C, C + sum);
    a2 = MIN(MAX(a2, L), H);
    a1 = sum - a2;

    oldb = smorch->b;

    L = MAX(sum - C, -C);
    H = MIN(C, C + sum);
    
    b1 = E1 + (a1 - alpha1) * k11 + (a2 - alpha2) * k12 + smorch->b;
    b2 = E2 + (a1 - alpha1) * k12 + (a2 - alpha2) * k22 + smorch->b;
    newb = 0.5 * (b1 + b2);    
      
    newE1 = b1 - newb;
    newE2 = b2 - newb;
      
    dobj =
      (smorch->regeps * fabs(a1) + a1 * (newE1 - 0.5 * a1 * k11 + newb) +
       smorch->regeps * fabs(a2) + a2 * (newE2 - 0.5 * a2 * k22 + newb) -
       a1 * a2 * k12) 
      -
      (smorch->regeps * fabs(alpha1) +
       alpha1 * (E1 - 0.5 * alpha1 *  k11 + oldb) +
       smorch->regeps * fabs(alpha2) +
       alpha2 * (E2 - 0.5 * alpha2 * k22 + oldb) -
       alpha1 * alpha2 * k12) ;
    
    
    if (fabs(fabs(a2) - fabs(alpha2)) < 
	smorch->eps * (fabs(a2) + fabs(alpha2) + smorch->eps))
      return 0;

    *_dobj = dobj;
    *_newa1 = a1;
    *_newa2 = a2;
    *_newb = newb;
    return 1;
  }  
  else return 0;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static int smorch_step(SMORCH *smorch, int i1, int i2,
		    LIST *slist, LIST_NODE **snode)
{
  double newa1, newa2, newb, dobj;

  if (fabs(smorch->alpha[i1]) > fabs(smorch->alpha[i2])) {
    int swap = i1;
    i1 = i2;
    i2 = swap;
  }

  if (smorch_calculate_step(smorch, i1, i2, slist, snode, 
			    &newa1, &newa2, &newb, &dobj) == 1) {

    if (dobj < 0) {
      //fprintf(stderr, "succeded with (%d, %d) = %f\n", i1, i2, dobj);
      smorch_update_alpha(smorch, i1, newa1, slist, snode, newb);
      smorch->b = newb;
      smorch_update_alpha(smorch, i2, newa2, slist, snode, newb);     
      smorch->objective += dobj;
      return 1;
    }
  }
  //  fprintf(stderr, "failed with (%d, %d) = %f : %f, %f\n", i1, i2, dobj,
  //  smorch->alpha[i1], smorch->alpha[i2]);
  return 0;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static unsigned smorch_complex_second_heuristic(SMORCH *smorch, double E2, int index, 
					     LIST *slist, LIST_NODE **snode)
{
  LIST *list;
  LIST_NODE *node;
  unsigned i, besti;
  double beststep, newa1, newa2, newb, dobj;

  list = slist ? slist : smorch->nonbound;
  beststep = 0;
  besti = index;

  for (node = list->head; node != NULL; node = node->next) {
    i = NODE_INDEX(smorch, node->data);
    if (i == index) continue;

    if (fabs(smorch->alpha[i]) > fabs(smorch->alpha[index])) {
      if (smorch_calculate_step(smorch, index, i, slist, snode, 
				&newa1, &newa2, &newb, &dobj) == 1) {
	if (dobj < beststep) {
	  beststep = dobj;
	  besti = index;
	}
      }
    }
    else {
      if (smorch_calculate_step(smorch, i, index, slist, snode, 
				&newa1, &newa2, &newb, &dobj) == 1) {
	if (dobj < beststep) {
	  beststep = dobj;
	  besti = i;
	}
      }
    }
  }
  return besti;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static unsigned smorch_second_heuristic(SMORCH *smorch, double E2, int index, 
				     LIST *slist, LIST_NODE **snode)
{
  LIST *list;
  LIST_NODE *node;
  unsigned i, besti;
  double bestE1, E1, y1;

  list = slist ? slist : smorch->nonbound;
#if OLD
  if (smorch->cache && list->count <= smorch->cache->size && smorch->best_step &&
      !smorch->examine_all)
#else
  if (smorch->best_step && !smorch->examine_all)
#endif
    return smorch_complex_second_heuristic(smorch, E2, index, slist, snode);
  node = list->head;
  if ((besti = NODE_INDEX(smorch, node->data)) == index) {
    node = node->next;
    besti = NODE_INDEX(smorch, node->data);
  }
  y1 = smorch->y[besti];
  bestE1 = smorch_output(smorch, besti, snode) - y1;
  
  if (!smorch->regression) {
    if (E2 >= 0) {
      for (node = node->next; node != NULL; node = node->next) {
	i = NODE_INDEX(smorch, node->data);
	if (i == index) continue;
	y1 = smorch->y[i];
	E1 = smorch_output(smorch, i, snode) - y1;
	if(E1 < bestE1) {
	bestE1 = E1;
	besti = i;
	}
      }
    }
    else {
      for (node = node->next; node != NULL; node = node->next) {
	i = NODE_INDEX(smorch, node->data);
	if (i == index) continue;
	y1 = smorch->y[i];
	E1 = smorch_output(smorch, i, snode) - y1;
	if(E1 > bestE1) {
	  bestE1 = E1;
	  besti = i;
	}
      }
    }
    if (((E2 >= 0) && (bestE1 < 0)) || ((E2 < 0) && (bestE1 > 0)))
      return besti;
    else return index;
  }
  else {
    int sa1, sa2;
    double thisE1;
    
    sa2 = (smorch->alpha[index] == 0) ? SGN(-E2) : SGN(smorch->alpha[index]);
    sa1 = (smorch->alpha[besti] == 0) ? SGN(-bestE1) : SGN(smorch->alpha[besti]);
    bestE1 = fabs(bestE1 - E2);
    for (node = node->next; node != NULL; node = node->next) {
      i = NODE_INDEX(smorch, node->data);
      if (i == index) continue;
	y1 = smorch->y[i];
	E1 = smorch_output(smorch, i, snode) - y1;
	sa1 = (smorch->alpha[i] == 0) ? SGN(-E1) : SGN(smorch->alpha[i]);
	thisE1 = fabs(E1 - E2);
	if(thisE1 > bestE1) {
	  bestE1 = E1;
	  besti = i;
	}
    }
    return besti;
  }
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static int smorch_examine(SMORCH *smorch, int i2, unsigned ssz, unsigned *sindex,
		       LIST *slist, LIST_NODE **snode)
{
  LIST *list;
  LIST_NODE *node;
  int i, i1;
  unsigned sz;
  double y2, f2, alpha2, E2, r2, fabsa2;

  list = (ssz > 1 && ssz < smorch->sz) ? slist : smorch->nonbound;
  sz = (ssz > 1 && ssz < smorch->sz) ? ssz : smorch->sz;
  y2 = smorch->y[i2];
  alpha2 = smorch->alpha[i2];
  f2 = smorch_output(smorch, i2, snode);
  E2 = f2 - y2;
  fabsa2 = fabs(alpha2);
  if (smorch->regression)
    r2 = smorch->regeps - fabs(E2);
  else
    r2 = E2 * y2;
  if (kkt_badness(smorch, y2, f2, smorch->alpha[i2]) > 0.0) {
    if (list->count > 1) {
      i1 = smorch_second_heuristic(smorch, E2, i2, slist, snode);
      /*fprintf(stderr, "2nd choice: (%d, %d), ", i2, i1);*/
      if (smorch_step(smorch, i1, i2, slist, snode)) {
	/*fprintf(stderr, "took step\n");*/
	return 1;
      }
      /*else
	fprintf(stderr, "did not take step\n");*/
    }

    /* Create a random set of numbers from 0 to (# nonbound - 1). */
    shuffle_unsigned_indices(smorch->random_index, list->count);

    /* Grab all of the indices of nonbound alphas. */
    for (node = list->head, i = 0; node != NULL; node = node->next, i++)
      smorch->sub_index[i] = NODE_INDEX(smorch, node->data);

    /* The next loop will now loop over all nonbound alphas only,
     * and in random order (w.r.t. i1).
     */
    for (i = 0; i < list->count; i++) {
      i1 = smorch->sub_index[smorch->random_index[i]];
      if (smorch_step(smorch, i1, i2, slist, snode))
	return 1;
    }

    if (!smorch->examine_all && smorch->lazy_loop)
      return 0;

    /* Create a random set of numbers from 0 to (# alpha - 1). */
    shuffle_unsigned_indices(smorch->random_index, sz);
    /* Loop over all alphas in random order. */
    if (smorch->cache) smorch->cache->notickle = 1;
    for (i = 0; i < sz; i++) {
      i1 = (ssz > 1 && ssz < smorch->sz) ? sindex[smorch->random_index[i]] :
	smorch->random_index[i];
      if (smorch_step(smorch, i1, i2, slist, snode)) {
	if (smorch->cache) smorch->cache->notickle = 0;
	return 1;
      }
    }
    if (smorch->cache) smorch->cache->notickle = 0;
  }
  return 0;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static void smorch_main(SMORCH *smorch, unsigned ssz, unsigned *sindex,
		     LIST *slist, LIST_NODE **snode)
{
  LIST *list;
  LIST_NODE *node;
  int i, sz = 0, changed, worst_failed = 0;

  smorch->epoch = 0;
  smorch->examine_all = 1;
  smorch->num_changed = 0;
  if (smorch->hook) smorch->hook(smorch);
  list = (ssz > 1 && ssz < smorch->sz) ? slist : smorch->nonbound;
  while (smorch->num_changed > 0 || smorch->examine_all) {
    (smorch->epoch)++;
    smorch->num_changed = 0;
#if OLD
    if (smorch->examine_all || (smorch->regression && smorch->epoch % 1000 == 0)) {
#else
    if (smorch->examine_all) {
#endif
      sz = (ssz > 1 && ssz < smorch->sz) ? ssz : smorch->sz;
      for (i = 0; i < sz; i++)
	smorch->num_changed += smorch_examine(smorch, (ssz > 1 && ssz < smorch->sz) ?
					sindex[i] : i, ssz, sindex, slist,
					snode);
    }
    else if (smorch->worst_first && smorch->epoch > 1) {
      changed = 1;
      
      i = smorch_worst_kkt(smorch, list, snode);
      while (i != -1 && changed) {
	changed = smorch_examine(smorch, i, ssz, sindex, slist, snode);
	smorch->num_changed += changed;
	i = smorch_worst_kkt(smorch, list, snode);
      }
      if (i == -1)
	worst_failed = 1;
      else
	worst_failed = 0;
    }
    else {
      /* Loop over nonbound alpha.  We construct this index
       * array because the nonbound list could change in the
       * middle of the loop.
       */
      sz = list->count;
      for (i = 0, node = list->head; node != NULL; i++, node = node->next)
	smorch->sub_index[i] = NODE_INDEX(smorch, node->data);
      for (i = 0; i < sz; i++)
	if ((!smorch->regression && smorch->alpha[smorch->sub_index[i]] > 0 &&
	     smorch->alpha[smorch->sub_index[i]] < smorch->C) ||
	    (smorch->regression && smorch->alpha[smorch->sub_index[i]] != 0 &&
	     fabs(smorch->alpha[smorch->sub_index[i]]) != smorch->C))
	  smorch->num_changed += smorch_examine(smorch, smorch->sub_index[i],
					  ssz, sindex, slist, snode);
    }
    if (smorch->examine_all == 1) {
      smorch->examine_all = 0;
      smorch->prev_examine_all = 1;
    }
    else {
      smorch->prev_examine_all = 0;
      if (smorch->num_changed == 0 || worst_failed)
	smorch->examine_all = 1;
    }
    if (smorch->hook) smorch->hook(smorch);
  }
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static void smorch_init_memory(SMORCH *smorch)
{
  double *y, *x1, *x2, ave;
  unsigned i;

  smorch->xdim = dataset_x_size(smorch->data);
  smorch->sz = dataset_size(smorch->data);
  smorch->random_index = xmalloc(sizeof(unsigned) * smorch->sz);
  smorch->sub_index = xmalloc(sizeof(unsigned) * smorch->sz);
  
  if (smorch->cache_size > 0)
    smorch->cache = smorch_cache_create(smorch->cache_size, smorch->sz);

  /* NB: we initialize all alphas to 0, so all are zero (duh) and all
   * are at bound, so the two lists below should be empty.  If the
   * initialization ever changes to something ``clever'' with nonzero
   * alphas, then the lists would have to be initialized appropriately.
   */

  smorch->nonzero_node = xmalloc(sizeof(LIST_NODE *) * smorch->sz);
  smorch->nonbound_node = xmalloc(sizeof(LIST_NODE *) * smorch->sz);
  smorch->alpha = xmalloc(sizeof(double) * smorch->sz);
  for (i = 0; i < smorch->sz; i++) {
    smorch->alpha[i] = 0;
    smorch->nonzero_node[i] = smorch->nonbound_node[i] = NULL;
  }

  smorch->nonzero = list_create();
  smorch->nonbound = list_create();
  smorch->freelist = list_create();

  smorch->kii = xmalloc(sizeof(double) * smorch->sz);
  for (i = 0; i < smorch->sz; i++)
    smorch->kii[i] = smorch->kernel(dataset_x(smorch->data, i),
			      dataset_x(smorch->data, i),
			      smorch->aux, smorch->xdim);

  smorch->y = xmalloc(sizeof(double) * smorch->sz);
  ave = 0;
  for (i = 0; i < smorch->sz; i++) {
    y = dataset_y(smorch->data, i);
    smorch->y[i] = y[smorch->yindex];
    ave += smorch->y[i];
  }
  smorch->b = -ave / smorch->sz;

  smorch->out = xmalloc(sizeof(double) * smorch->sz);
  for (i = 0; i < smorch->sz; i++)
    smorch->out[i] = -smorch->b;

  smorch->temp = xmalloc(sizeof(double) * smorch->xdim);

  smorch->alpha_index = xmalloc(sizeof(unsigned) * smorch->sz);
  smorch->update_time = xmalloc(sizeof(unsigned) * smorch->sz);
  smorch->delta_alpha = xmalloc(sizeof(double) * smorch->sz);
  smorch->store_b = xmalloc(sizeof(double) * smorch->sz);
  for (i = 0; i < smorch->sz; i++)
    smorch->update_time[i] = 0;
  smorch->time = 0;

  x1 = dataset_x(smorch->data, 0);
  x2 = dataset_x(smorch->data, 1);
  if (x1 != x2)
    smorch->safe_ds_pointers = 1;
  else
    smorch->safe_ds_pointers = 0;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static void smorch_clear_memory(SMORCH *smorch)
{
  xfree(smorch->random_index);
  smorch->random_index = NULL;
  xfree(smorch->sub_index);
  smorch->sub_index = NULL;

  if (smorch->cache) {
    smorch_cache_destroy(smorch->cache);
    smorch->cache = NULL;
  }

  xfree(smorch->nonzero_node);
  smorch->nonzero_node = NULL;
  xfree(smorch->nonbound_node);
  smorch->nonbound_node = NULL;
  
  list_destroy_all(smorch->nonzero);
  smorch->nonzero = NULL;
  list_destroy_all(smorch->nonbound);
  smorch->nonbound = NULL;
  list_destroy_all(smorch->freelist);
  smorch->freelist = NULL;

  xfree(smorch->kii);
  smorch->kii = NULL;
  xfree(smorch->y);
  smorch->y = NULL;
  xfree(smorch->out);
  smorch->out = NULL;
  xfree(smorch->temp);
  smorch->temp = NULL;

  xfree(smorch->alpha_index);
  smorch->alpha_index = NULL;
  xfree(smorch->update_time);
  smorch->update_time = NULL;
  xfree(smorch->delta_alpha);
  smorch->delta_alpha = NULL;
  xfree(smorch->store_b);
  smorch->store_b = NULL;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void smorch_batch(SMORCH *smorch)
{
  unsigned *sindex, *notsindex, *kktbad, *kktgood, *sbound, *snotbound;
  unsigned *sbindex, *snindex, *kbindex;
  unsigned kgi, kbi, sbi, sni, limit;
  unsigned i, j, swap;
  LIST *slist;
  LIST_NODE **snode;

  kktbad = xmalloc(sizeof(unsigned) * smorch->sz - smorch->subset_size);
  kktgood = xmalloc(sizeof(unsigned) * smorch->sz - smorch->subset_size);
  sbound = xmalloc(sizeof(unsigned) * smorch->subset_size);
  sbindex = xmalloc(sizeof(unsigned) * smorch->subset_size);
  snindex = xmalloc(sizeof(unsigned) * smorch->subset_size);
  kbindex = xmalloc(sizeof(unsigned) * smorch->sz - smorch->subset_size);
  snotbound = xmalloc(sizeof(unsigned) * smorch->subset_size);
  sindex = xmalloc(sizeof(unsigned) * smorch->subset_size);
  notsindex = xmalloc(sizeof(unsigned) * smorch->sz - smorch->subset_size);
  snode = xmalloc(sizeof(LIST_NODE *) * smorch->sz);
  slist = list_create();
  
  shuffle_unsigned_indices(smorch->random_index, smorch->sz);
  for (i = 0; i < smorch->subset_size; i++)
    sindex[i] = smorch->random_index[i];
  for (i = 0; i < smorch->sz - smorch->subset_size; i++)
    notsindex[i] = smorch->random_index[i + smorch->subset_size];

  for (i = 0; i < smorch->sz; i++)
    snode[i] = NULL;

  while (1) {
    smorch_main(smorch, smorch->subset_size, sindex, slist, snode);
    
    /* find kkt status of (up to subset size) in smorch */
    kgi = kbi = sbi = sni = 0;
    for (i = 0; i < smorch->sz - smorch->subset_size; i++) {
      double y2, alpha2, E2, f2;
      unsigned i2;

      i2 = notsindex[i];
      y2 = smorch->y[i2];
      alpha2 = smorch->alpha[i2];
      f2 = smorch_output(smorch, i2, snode);
      E2 = f2 - y2;
      if (kkt_badness(smorch, y2, f2, alpha2) > 0.0)
	kktbad[kbi++] = i;
      else
	kktgood[kgi++] = i;
      if (kbi >= smorch->subset_size) break;
    }
    if (kbi == 0) break;

    for (i = 0; i < smorch->subset_size; i++) {
      if (snode[sindex[i]] == NULL)
	sbound[sbi++] = i;
      else
	snotbound[sni++] = i;
    }
    
    /* Fill as many bad kkt as will fit into the bound slots. */
    shuffle_unsigned_indices(kbindex, kbi);
    shuffle_unsigned_indices(sbindex, sbi);
    shuffle_unsigned_indices(snindex, sni);
    limit = MIN(smorch->subset_size, kbi);
    for (i = 0; i < limit; i++) {
      if (i < sbi)
	swap = sindex[sbound[sbindex[i]]];
      else
	swap = sindex[snotbound[snindex[i - sbi]]];
      j = notsindex[kktbad[kbindex[i]]];
      if (i < sbi)
	sindex[sbound[sbindex[i]]] = j;
      else
	sindex[snotbound[snindex[i - sbi]]] = j;
      notsindex[kktbad[kbindex[i]]] = swap;
      if (snode[swap]) {
	list_remove_node(slist, snode[swap]);
	list_insert_head(smorch->freelist, snode[swap]);
	snode[swap] = NULL;
      }
      if ((!smorch->regression && smorch->alpha[j] > 0 && smorch->alpha[j] < smorch->C) ||
	  (smorch->regression && fabs(smorch->alpha[j]) > 0 &&
	   fabs(smorch->alpha[j]) < smorch->C)) {
	LIST_NODE *node;
	if((node = list_remove_head(smorch->freelist)))
	  node->data = &smorch->alpha[j];
	else
	  node = list_node_create(&smorch->alpha[j]);
	list_insert_tail(slist, node);
	snode[j] = node;
      }
    }
  }

  xfree(kktbad);
  xfree(kktgood);
  xfree(sbindex);
  xfree(snindex);
  xfree(kbindex);
  xfree(sindex);
  xfree(notsindex);
  xfree(snode);
  list_destroy_all(slist);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

double smorch_objective(SMORCH *smorch)
{
  unsigned i, j;
  double yi, yj, sum = 0;
  LIST_NODE *ni, *nj;
  
  if (smorch->cache) smorch->cache->notickle = 1;

  if (!smorch->regression) {
    double alphasum = 0;
    for (ni = smorch->nonzero->head; ni != NULL; ni = ni->next) {
      i = NODE_INDEX(smorch, ni->data);
      yi = smorch->y[i];
      for (nj = smorch->nonzero->head; nj != NULL; nj = nj->next) {
	j = NODE_INDEX(smorch, nj->data);
	yj = smorch->y[j];
	sum += yi * yj *
	  smorch_kernel_value(smorch, i, j, smorch->nonbound_node, 1)
	  * smorch->alpha[i] * smorch->alpha[j];
      }
      alphasum += smorch->alpha[i];
    }
    sum = 0.5 * sum - alphasum;
  }
  else {
    double abssum = 0, alphasum = 0;
    for (ni = smorch->nonzero->head; ni != NULL; ni = ni->next) {
      i = NODE_INDEX(smorch, ni->data);
      yi = smorch->y[i];
      for (nj = smorch->nonzero->head; nj != NULL; nj = nj->next) {
	j = NODE_INDEX(smorch, nj->data);
	sum += smorch_kernel_value(smorch, i, j, smorch->nonbound_node, 1)
	  * smorch->alpha[i] * smorch->alpha[j];
      }
      alphasum += yi * smorch->alpha[i];
      abssum += fabs(smorch->alpha[i]);
    }
    sum = 0.5 * sum - alphasum + smorch->regeps * abssum;
  }

  if (smorch->cache) smorch->cache->notickle = 0;
  return sum;
} 

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

double smorch_constraint(SMORCH *smorch)
{
  unsigned i;
  double sum = 0;
  LIST_NODE *ni;

  if (!smorch->regression) {
    for (ni = smorch->nonzero->head; ni != NULL; ni = ni->next) {
      i = NODE_INDEX(smorch, ni->data);
      sum += smorch->y[i] * smorch->alpha[i];
    }
  }
  else {
    for (ni = smorch->nonzero->head; ni != NULL; ni = ni->next) {
      i = NODE_INDEX(smorch, ni->data);
      sum += smorch->alpha[i];
    }
  }
  return sum;
} 

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

SVM *smorch_train(SMORCH *smorch)
{
  SVM *svm;
  LIST_NODE *node;
  double *x;
  unsigned i, j, k;

  smorch_init_memory(smorch);

  if (smorch->tube && smorch->regression) {
    double tol = smorch->tol;
    int onemore = 0;
    
    while (tol > 0) {
      smorch->tol = tol;
      fprintf(stderr, "######################### tube diameter = %f\n", tol);
      if (smorch->subset_size != 0) 
	smorch_batch(smorch);
      else
	smorch_main(smorch, 0, NULL, NULL, NULL);
      if (!onemore && tol > smorch->regeps / smorch->tube_final)
	tol /= smorch->tube_rate;
      else if (!onemore && tol < smorch->regeps / smorch->tube_final) {
	tol = smorch->regeps / smorch->tube_final + smorch->eps;
	onemore = 1;
      }
      else
	tol = 0;
    }
  }
  else {
    if (smorch->subset_size != 0) 
      smorch_batch(smorch);
    else
      smorch_main(smorch, 0, NULL, NULL, NULL);
  }



#if 0
  smorch->subset_size = 0;
  smorch->ultra_clever = 0;
  smorch->best_step = 0;
  smorch->worst_first = 0;
  smorch->lazy_loop = 0;
  smorch_main(smorch, 0, NULL, NULL, NULL);
#endif

  if (smorch->finalhook) smorch->finalhook(smorch);
  
  svm = xmalloc(sizeof(SVM));
  svm->xdim = smorch->xdim;
  svm->sz = smorch->nonzero->count;
  svm->b = smorch->b;
  svm->aux = smorch->aux;
  svm->kernel = smorch->kernel;
  svm->alpha = xmalloc(sizeof(double) * svm ->sz);
  svm->y = xmalloc(sizeof(double) * svm ->sz);
  svm->x = allocate_array(2, sizeof(double), svm->sz, svm->xdim);
  svm->regression = smorch->regression;

  for (i = 0, node = smorch->nonzero->head; node != NULL;
       i++, node = node->next) {
    j = NODE_INDEX(smorch, node->data);
    svm->alpha[i] = smorch->alpha[j];
    svm->y[i] = smorch->y[j];
    x = dataset_x(smorch->data, j);
    for (k = 0; k < svm->xdim; k++) {
      svm->x[i][k] = x[k];
    }
  }  

  smorch_clear_memory(smorch);  
  return svm;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

