
/* Copyright (c) 1995 by G. W. Flake. */

#include <stdlib.h>
#include <math.h>
#include "nodelib/misc.h"
#include "nodelib/nn.h"
#include "nodelib/optimize.h"

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* Traverse NN in feedforward order.  Set a layer or slab flag to true
   if either an incoming link is set, or if the source to an incoming
   link is set.  If a layers is set, then all sublayers in that layer
   should also be set. */

static void traverse_all_need_grads_flags(NN *nn)
{
  unsigned i, j, need_grads;
  NN_LINKLIST *links;
  NN_LAYERLIST *layers;

  for(i = 0; i < nn->numlayers; i++) {
    need_grads = 0;
    for(links = nn->layers[i].in; links != NULL; links = links->cdr) {
      if(links->link->need_grads) {
	need_grads = 1;
	break;
      }
      for(layers = links->link->source; layers != NULL;
	  layers = layers->cdr)
	if(layers->layer->need_grads)  {
	  need_grads = 1;
	  break;
	}
      if(need_grads) break;
    }

    nn->layers[i].need_grads = need_grads;

    if(need_grads) {
      for(j = 0; j < nn->layers[i].numslabs; j++)
	nn->layers[i].slabs[j].need_grads = 1;
    }
    else {
      for(j = 0; j < nn->layers[i].numslabs; j++) {
	need_grads = 0;
	for(links = nn->layers[i].slabs[j].in; links != NULL;
	    links = links->cdr) {
	  if(links->link->need_grads) {
	    need_grads = 1;
	    break;
	  }
	  for(layers = links->link->source; layers != NULL;
	      layers = layers->cdr)
	    if(layers->layer->need_grads)  {
	      need_grads = 1;
	      break;
	    }
	  if(need_grads) break;
	}
	nn->layers[i].slabs[j].need_grads = need_grads;
	if(need_grads) nn->layers[i].need_grads = need_grads;
      }
    }
  }
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static void update_all_need_grads_flags(NN *nn)
{
  unsigned i, j;

  /* First step: mark every layer and sublayer as not needed. */
  for(i = 0; i < nn->numlayers; i++) {
    nn->layers[i].need_grads = 0;
    for(j = 0; j < nn->layers[i].numslabs; j++)
      nn->layers[i].slabs[j].need_grads = 0;
  }

  /* Second step: traverse NN in feedforward order.  Set a layer or slab
   * flag to true if either an incoming link is set, or if the source to an
   * incoming link is set.  If a layers is set, then all sublayer in that
   * layer should also be set.
   */
  traverse_all_need_grads_flags(nn);

  /* Third step: traverse a second time in case there are recurrent
   * connections. 
   */
  traverse_all_need_grads_flags(nn);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static void update_pointer_pointers(NN *nn)
{
  unsigned i, j, sz, tot;
  double *srcw, *srcg;

  if(nn->weights) xfree(nn->weights);
  if(nn->grads) xfree(nn->grads);
  nn->weights = xmalloc(sizeof(double *) * nn->numweights);
  nn->grads = xmalloc(sizeof(double *) * nn->numweights);
  tot = 0;
  for(i = 0; i < nn->numlinks; i++)
    if(nn->links[i]->need_grads) {
      if(nn->links[i]->A) {
	srcw = &nn->links[i]->A[0][0][0];
	srcg = &nn->links[i]->dA[0][0][0];
	sz = nn->links[i]->numin * nn->links[i]->numin * nn->links[i]->numout;
	for(j = 0; j < sz; j++) {
	  nn->weights[tot + j] = &srcw[j];
	  nn->grads[tot + j] = &srcg[j];
	}
	tot += sz;
      }
      if(nn->links[i]->u) {
	srcw = &nn->links[i]->u[0][0];
	srcg = &nn->links[i]->du[0][0];
	sz = nn->links[i]->numin * nn->links[i]->numout;
	for(j = 0; j < sz; j++) {
	  nn->weights[tot + j] = &srcw[j];
	  nn->grads[tot + j] = &srcg[j];
	}
	tot += sz;
      }
      if(nn->links[i]->v) {
	srcw = &nn->links[i]->v[0][0];
	srcg = &nn->links[i]->dv[0][0];
	sz = nn->links[i]->numin * nn->links[i]->numout;
	for(j = 0; j < sz; j++) {
	  nn->weights[tot + j] = &srcw[j];
	  nn->grads[tot + j] = &srcg[j];
	}
	tot += sz;
      }
      if(nn->links[i]->w) {
	srcw = &nn->links[i]->w[0][0];
	srcg = &nn->links[i]->dw[0][0];
	sz = nn->links[i]->numaux * nn->links[i]->numout;
	for(j = 0; j < sz; j++) {
	  nn->weights[tot + j] = &srcw[j];
	  nn->grads[tot + j] = &srcg[j];
	}
	tot += sz;
      }
      if(nn->links[i]->a) {
	srcw = &nn->links[i]->a[0];
	srcg = &nn->links[i]->da[0];
	sz = nn->links[i]->numout;
	for(j = 0; j < sz; j++) {
	  nn->weights[tot + j] = &srcw[j];
	  nn->grads[tot + j] = &srcg[j];
	}
	tot += sz;
      }
      if(nn->links[i]->b) {
	srcw = &nn->links[i]->b[0];
	srcg = &nn->links[i]->db[0];
	sz = nn->links[i]->numout;
	for(j = 0; j < sz; j++) {
	  nn->weights[tot + j] = &srcw[j];
	  nn->grads[tot + j] = &srcg[j];
	}
	tot += sz;
      }
    }
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void nn_lock_link(NN *nn, unsigned linknum)
{
  if(nn->links[linknum]->need_grads != 0) {
    nn->links[linknum]->need_grads = 0;
    if(nn->links[linknum]->A)
      nn->numweights -= nn->links[linknum]->numin *
	nn->links[linknum]->numin * nn->links[linknum]->numout;
    if(nn->links[linknum]->u)
      nn->numweights -= nn->links[linknum]->numin * nn->links[linknum]->numout;
    if(nn->links[linknum]->v)
      nn->numweights -= nn->links[linknum]->numin * nn->links[linknum]->numout;
    if(nn->links[linknum]->w)
      nn->numweights -= nn->links[linknum]->numaux *
	nn->links[linknum]->numout;
    if(nn->links[linknum]->a)
      nn->numweights -= nn->links[linknum]->numout;
    if(nn->links[linknum]->b)
      nn->numweights -= nn->links[linknum]->numout;
  }
  update_all_need_grads_flags(nn);
  update_pointer_pointers(nn);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void nn_unlock_link(NN *nn, unsigned linknum)
{
  if(nn->links[linknum]->need_grads != 1) {
    nn->links[linknum]->need_grads = 1;
    if(nn->links[linknum]->A)
      nn->numweights += nn->links[linknum]->numin *
	nn->links[linknum]->numin * nn->links[linknum]->numout;
    if(nn->links[linknum]->u)
      nn->numweights += nn->links[linknum]->numin * nn->links[linknum]->numout;
    if(nn->links[linknum]->v)
      nn->numweights += nn->links[linknum]->numin * nn->links[linknum]->numout;
    if(nn->links[linknum]->w)
      nn->numweights += nn->links[linknum]->numaux *
	nn->links[linknum]->numout;
    if(nn->links[linknum]->a)
      nn->numweights += nn->links[linknum]->numout;
    if(nn->links[linknum]->b)
      nn->numweights += nn->links[linknum]->numout;
  }
  update_all_need_grads_flags(nn);
  update_pointer_pointers(nn);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void nn_get_weights(NN *nn, double *w)
{
  unsigned i, j, sz;
  double *src;

  for(i = 0; i < nn->numlinks; i++)
    if(nn->links[i]->need_grads) {
      if(nn->links[i]->A) {
	src = &nn->links[i]->A[0][0][0];
	sz = nn->links[i]->numin * nn->links[i]->numin * nn->links[i]->numout;
	for(j = 0; j < sz; j++)
	  *w++ = *src++;
      }
      if(nn->links[i]->u) {
	src = &nn->links[i]->u[0][0];
	sz = nn->links[i]->numin * nn->links[i]->numout;
	for(j = 0; j < sz; j++)
	  *w++ = *src++;
      }
      if(nn->links[i]->v) {
	src = &nn->links[i]->v[0][0];
	sz = nn->links[i]->numin * nn->links[i]->numout;
	for(j = 0; j < sz; j++)
	  *w++ = *src++;
      }
      if(nn->links[i]->w) {
	src = &nn->links[i]->w[0][0];
	sz = nn->links[i]->numaux * nn->links[i]->numout;
	for(j = 0; j < sz; j++)
	  *w++ = *src++;
      }
      if(nn->links[i]->a) {
	src = &nn->links[i]->a[0];
	sz = nn->links[i]->numout;
	for(j = 0; j < sz; j++)
	  *w++ = *src++;
      }
      if(nn->links[i]->b) {
	src = &nn->links[i]->b[0];
	sz = nn->links[i]->numout;
	for(j = 0; j < sz; j++)
	  *w++ = *src++;
      }
    }
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void nn_get_grads(NN *nn, double *g)
{
  unsigned i, j, sz;
  double *src;

  for(i = 0; i < nn->numlinks; i++)
    if(nn->links[i]->need_grads) {
      if(nn->links[i]->A) {
	src = &nn->links[i]->dA[0][0][0];
	sz = nn->links[i]->numin * nn->links[i]->numin * nn->links[i]->numout;
	for(j = 0; j < sz; j++)
	  *g++ = *src++;
      }
      if(nn->links[i]->u) {
	src = &nn->links[i]->du[0][0];
	sz = nn->links[i]->numin * nn->links[i]->numout;
	for(j = 0; j < sz; j++)
	  *g++ = *src++;
      }
      if(nn->links[i]->v) {
	src = &nn->links[i]->dv[0][0];
	sz = nn->links[i]->numin * nn->links[i]->numout;
	for(j = 0; j < sz; j++)
	  *g++ = *src++;
      }
      if(nn->links[i]->w) {
	src = &nn->links[i]->dw[0][0];
	sz = nn->links[i]->numaux * nn->links[i]->numout;
	for(j = 0; j < sz; j++)
	  *g++ = *src++;
      }
      if(nn->links[i]->a) {
	src = &nn->links[i]->da[0];
	sz = nn->links[i]->numout;
	for(j = 0; j < sz; j++)
	  *g++ = *src++;
      }
      if(nn->links[i]->b) {
	src = &nn->links[i]->db[0];
	sz = nn->links[i]->numout;
	for(j = 0; j < sz; j++)
	  *g++ = *src++;
      }
    }
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void nn_set_weights(NN *nn, double *w)
{
  unsigned i, j, sz;
  double *dst;

  for(i = 0; i < nn->numlinks; i++)
    if(nn->links[i]->need_grads) {
      if(nn->links[i]->A) {
	dst = &nn->links[i]->A[0][0][0];
	sz = nn->links[i]->numin * nn->links[i]->numin * nn->links[i]->numout;
	for(j = 0; j < sz; j++)
	  *dst++ = *w++;
      }
      if(nn->links[i]->u) {
	dst = &nn->links[i]->u[0][0];
	sz = nn->links[i]->numin * nn->links[i]->numout;
	for(j = 0; j < sz; j++)
	  *dst++ = *w++;
      }
      if(nn->links[i]->v) {
	dst = &nn->links[i]->v[0][0];
	sz = nn->links[i]->numin * nn->links[i]->numout;
	for(j = 0; j < sz; j++)
	  *dst++ = *w++;
      }
      if(nn->links[i]->w) {
	dst = &nn->links[i]->w[0][0];
	sz = nn->links[i]->numaux * nn->links[i]->numout;
	for(j = 0; j < sz; j++)
	  *dst++ = *w++;
      }
      if(nn->links[i]->a) {
	dst = &nn->links[i]->a[0];
	sz = nn->links[i]->numout;
	for(j = 0; j < sz; j++)
	  *dst++ = *w++;
      }
      if(nn->links[i]->b) {
	dst = &nn->links[i]->b[0];
	sz = nn->links[i]->numout;
	for(j = 0; j < sz; j++)
	  *dst++ = *w++;
      }
    }
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void nn_set_grads(NN *nn, double *g)
{
  unsigned i, j, sz;
  double *dst;

  for(i = 0; i < nn->numlinks; i++)
    if(nn->links[i]->need_grads) {
      if(nn->links[i]->A) {
	dst = &nn->links[i]->dA[0][0][0];
	sz = nn->links[i]->numin * nn->links[i]->numin * nn->links[i]->numout;
	for(j = 0; j < sz; j++)
	  *dst++ = *g++;
      }
      if(nn->links[i]->u) {
	dst = &nn->links[i]->du[0][0];
	sz = nn->links[i]->numin * nn->links[i]->numout;
	for(j = 0; j < sz; j++)
	  *dst++ = *g++;
      }
      if(nn->links[i]->v) {
	dst = &nn->links[i]->dv[0][0];
	sz = nn->links[i]->numin * nn->links[i]->numout;
	for(j = 0; j < sz; j++)
	  *dst++ = *g++;
      }
      if(nn->links[i]->w) {
	dst = &nn->links[i]->dw[0][0];
	sz = nn->links[i]->numaux * nn->links[i]->numout;
	for(j = 0; j < sz; j++)
	  *dst++ = *g++;
      }
      if(nn->links[i]->a) {
	dst = &nn->links[i]->da[0];
	sz = nn->links[i]->numout;
	for(j = 0; j < sz; j++)
	  *dst++ = *g++;
      }
      if(nn->links[i]->b) {
	dst = &nn->links[i]->db[0];
	sz = nn->links[i]->numout;
	for(j = 0; j < sz; j++)
	  *dst++ = *g++;
      }
    }
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void nn_set_Rweights(NN *nn, double *Rw)
{
  unsigned i, j, sz;
  double *dst;

  for(i = 0; i < nn->numlinks; i++)
    if(nn->links[i]->need_grads) {
      if(nn->links[i]->RA) {
	dst = &nn->links[i]->RA[0][0][0];
	sz = nn->links[i]->numin * nn->links[i]->numin * nn->links[i]->numout;
	for(j = 0; j < sz; j++)
	  *dst++ = *Rw++;
      }
      if(nn->links[i]->Ru) {
	dst = &nn->links[i]->Ru[0][0];
	sz = nn->links[i]->numin * nn->links[i]->numout;
	for(j = 0; j < sz; j++)
	  *dst++ = *Rw++;
      }
      if(nn->links[i]->Rv) {
	dst = &nn->links[i]->Rv[0][0];
	sz = nn->links[i]->numin * nn->links[i]->numout;
	for(j = 0; j < sz; j++)
	  *dst++ = *Rw++;
      }
      if(nn->links[i]->Rw) {
	dst = &nn->links[i]->Rw[0][0];
	sz = nn->links[i]->numaux * nn->links[i]->numout;
	for(j = 0; j < sz; j++)
	  *dst++ = *Rw++;
      }
      if(nn->links[i]->Ra) {
	dst = &nn->links[i]->Ra[0];
	sz = nn->links[i]->numout;
	for(j = 0; j < sz; j++)
	  *dst++ = *Rw++;
      }
      if(nn->links[i]->Rb) {
	dst = &nn->links[i]->Rb[0];
	sz = nn->links[i]->numout;
	for(j = 0; j < sz; j++)
	  *dst++ = *Rw++;
      }
    }
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void nn_set_Rgrads(NN *nn, double *Rg)
{
  unsigned i, j, sz;
  double *dst;

  for(i = 0; i < nn->numlinks; i++)
    if(nn->links[i]->need_grads) {
      if(nn->links[i]->RA) {
	dst = &nn->links[i]->RdA[0][0][0];
	sz = nn->links[i]->numin * nn->links[i]->numin * nn->links[i]->numout;
	for(j = 0; j < sz; j++)
	  *dst++ = *Rg++;
      }
      if(nn->links[i]->Ru) {
	dst = &nn->links[i]->Rdu[0][0];
	sz = nn->links[i]->numin * nn->links[i]->numout;
	for(j = 0; j < sz; j++)
	  *dst++ = *Rg++;
      }
      if(nn->links[i]->Rv) {
	dst = &nn->links[i]->Rdv[0][0];
	sz = nn->links[i]->numin * nn->links[i]->numout;
	for(j = 0; j < sz; j++)
	  *dst++ = *Rg++;
      }
      if(nn->links[i]->Rw) {
	dst = &nn->links[i]->Rdw[0][0];
	sz = nn->links[i]->numaux * nn->links[i]->numout;
	for(j = 0; j < sz; j++)
	  *dst++ = *Rg++;
      }
      if(nn->links[i]->Ra) {
	dst = &nn->links[i]->Rda[0];
	sz = nn->links[i]->numout;
	for(j = 0; j < sz; j++)
	  *dst++ = *Rg++;
      }
      if(nn->links[i]->Rb) {
	dst = &nn->links[i]->Rdb[0];
	sz = nn->links[i]->numout;
	for(j = 0; j < sz; j++)
	  *dst++ = *Rg++;
      }
    }
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void nn_get_Rweights(NN *nn, double *Rw)
{
  unsigned i, j, sz;
  double *src;

  for(i = 0; i < nn->numlinks; i++)
    if(nn->links[i]->need_grads) {
      if(nn->links[i]->RA) {
	src = &nn->links[i]->RA[0][0][0];
	sz = nn->links[i]->numin * nn->links[i]->numin * nn->links[i]->numout;
	for(j = 0; j < sz; j++)
	  *Rw++ = *src++;
      }
      if(nn->links[i]->Ru) {
	src = &nn->links[i]->Ru[0][0];
	sz = nn->links[i]->numin * nn->links[i]->numout;
	for(j = 0; j < sz; j++)
	  *Rw++ = *src++;
      }
      if(nn->links[i]->Rv) {
	src = &nn->links[i]->Rv[0][0];
	sz = nn->links[i]->numin * nn->links[i]->numout;
	for(j = 0; j < sz; j++)
	  *Rw++ = *src++;
      }
      if(nn->links[i]->Rw) {
	src = &nn->links[i]->Rw[0][0];
	sz = nn->links[i]->numaux * nn->links[i]->numout;
	for(j = 0; j < sz; j++)
	  *Rw++ = *src++;
      }
      if(nn->links[i]->Ra) {
	src = &nn->links[i]->Ra[0];
	sz = nn->links[i]->numout;
	for(j = 0; j < sz; j++)
	  *Rw++ = *src++;
      }
      if(nn->links[i]->Rb) {
	src = &nn->links[i]->Rb[0];
	sz = nn->links[i]->numout;
	for(j = 0; j < sz; j++)
	  *Rw++ = *src++;
      }
    }
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void nn_get_Rgrads(NN *nn, double *Rg)
{
  unsigned i, j, sz;
  double *src;

  for(i = 0; i < nn->numlinks; i++)
    if(nn->links[i]->need_grads) {
      if(nn->links[i]->RA) {
	src = &nn->links[i]->RdA[0][0][0];
	sz = nn->links[i]->numin * nn->links[i]->numin * nn->links[i]->numout;
	for(j = 0; j < sz; j++)
	  *Rg++ = *src++;
      }
      if(nn->links[i]->Ru) {
	src = &nn->links[i]->Rdu[0][0];
	sz = nn->links[i]->numin * nn->links[i]->numout;
	for(j = 0; j < sz; j++)
	  *Rg++ = *src++;
      }
      if(nn->links[i]->Rv) {
	src = &nn->links[i]->Rdv[0][0];
	sz = nn->links[i]->numin * nn->links[i]->numout;
	for(j = 0; j < sz; j++)
	  *Rg++ = *src++;
      }
      if(nn->links[i]->Rw) {
	src = &nn->links[i]->Rdw[0][0];
	sz = nn->links[i]->numaux * nn->links[i]->numout;
	for(j = 0; j < sz; j++)
	  *Rg++ = *src++;
      }
      if(nn->links[i]->Ra) {
	src = &nn->links[i]->Rda[0];
	sz = nn->links[i]->numout;
	for(j = 0; j < sz; j++)
	  *Rg++ = *src++;
      }
      if(nn->links[i]->Rb) {
	src = &nn->links[i]->Rdb[0];
	sz = nn->links[i]->numout;
	for(j = 0; j < sz; j++)
	  *Rg++ = *src++;
      }
    }
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void nn_init(NN *nn, double wmax)
{
  unsigned i;
  double *w;

  wmax = (wmax < 0) ? -wmax : wmax;
  w = allocate_array(1, sizeof(double), nn->numweights);
  for(i = 0; i < nn->numweights; i++)
    w[i] = random_range(-wmax, wmax);
  nn_set_weights(nn, w);
  deallocate_array(w);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* Check that l is a valid layer number in nn. */

int nn_check_valid_layer(NN *nn, unsigned l)
{
  if(l >= nn->numlayers) {
    ulog(ULOG_ERROR, "nn_check_valid_layer: invalid layer number (%d >= %d).",
         l, nn->numlayers);
    return(1);
  }
  return(0);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* Check that (l, s) is a valid slab in nn. */

int nn_check_valid_slab(NN *nn, unsigned l, unsigned s)
{
  if(nn_check_valid_layer(nn, l)) return(1);
  if(s >= nn->layers[l].numslabs) {
    ulog(ULOG_ERROR, "nn_check_valid_slab: invalid slab number (%d >= %d).",
         s, nn->layers[l].numslabs);
    return(1);
  }
  return(0);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

unsigned nn_layerlist_len(NN_LAYERLIST *list)
{
  unsigned sz = 0;

  while(list) {
    sz++;
    list = list->cdr;
  }
  return(sz);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

NN_LAYERLIST *nn_layerlist_cons(NN_LAYER *layer, NN_LAYERLIST *cdr)
{
  NN_LAYERLIST *result;

  result = xmalloc(sizeof(NN_LAYERLIST));
  result->layer = layer;
  result->cdr = cdr;
  return(result);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void nn_layerlist_free(NN_LAYERLIST *list)
{
  NN_LAYERLIST *l;
  
  while(list) {
    l = list->cdr;
    xfree(list);
    list = l;
  }
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

unsigned nn_linklist_len(NN_LINKLIST *list)
{
  unsigned sz = 0;

  while(list) {
    sz++;
    list = list->cdr;
  }
  return(sz);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

NN_LINKLIST *nn_linklist_cons(NN_LINK *link, NN_LINKLIST *cdr)
{
  NN_LINKLIST *result;

  result = xmalloc(sizeof(NN_LINKLIST));
  result->link = link;
  result->cdr = cdr;
  return(result);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void nn_linklist_free(NN_LINKLIST *list)
{
  NN_LINKLIST *l;
  
  while(list) {
    l = list->cdr;
    xfree(list);
    list = l;
  }
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void nn_shutdown(void)
{
  void nn_shutdown_actfuncs(void);
  void nn_shutdown_netfuncs(void);

  nn_shutdown_actfuncs();
  nn_shutdown_netfuncs();
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

double nn_lnsrch_search_then_converge(OPTIMIZER *opt, double *d, double sz)
{
  NN *nn = opt->owner;
  double alpha;
  unsigned i;

  alpha = nn->info.stc_eta_0 / (1 + opt->epoch / nn->info.stc_tau);
  for(i = 0; i < opt->size; i++)
    *opt->weights[i] += d[i] * alpha;
  opt_eval_func(opt, NULL);
  return(alpha);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
