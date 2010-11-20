
/* Copyright (c) 1995 by G. W. Flake. */


#include <stdlib.h>

#include "nodelib/nn.h"
#include "nodelib/misc.h"

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void nn_forward(NN *nn, double *input)
{
  unsigned i, j, k;
  NN_LAYER *slab;
  NN_LINKLIST *l;

  /* Clean up the net input. */
  for(i = 0; i < nn->numlayers; i++)
    for(j = 0; j < nn->layers[i].sz; j++)
      nn->layers[i].x[j] = 0.0;

  /* Fill up the input vector. */
  for(i = 0; i < nn->numin; i++)
    nn->x[i] = input[i];
  
  /* For each layer... */
  for(i = 0; i < nn->numlayers; i++) {

    /* Compute the net input contributed by links coming into this layer. */
    for(l = nn->layers[i].in; l != NULL; l = l->cdr)
      l->link->nfunc->forward(nn, l->link, &nn->layers[i]);

    /* For each sublayer... */
    for(j = 0; j < nn->layers[i].numslabs; j++) {
      /* Compute the net input contributed by links coming into this
       * sub layer.
       */
      slab = &nn->layers[i].slabs[j];
      for(l = slab->in; l != NULL; l = l->cdr)
	l->link->nfunc->forward(nn, l->link, slab);

      /* Map net inputs through the activation functions. */
      for(k = 0; k < slab->sz; k++)
	slab->y[k] = slab->afunc->func(slab->x[k]);
    }
  }
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void nn_backward(NN *nn, double *de_dy)
{
  unsigned i, j, k;
  NN_LAYER *slab;
  NN_LINKLIST *l;

  /* Clean up the derivatives. */
  for(i = 0; i < nn->numweights; i++)
    *nn->grads[i] = 0.0;

  for(i = 0; i < nn->numlayers; i++)
    for(j = 0; j < nn->layers[i].sz; j++)
      nn->layers[i].dy[j] = 0.0;

  /* Pass the partial of the error w.r.t. the output. */
  for(i = 0; i < nn->numout; i++)
    nn->dy[i] = de_dy[i];
 
  for(i = nn->numlayers; i > 0; i--) {

    for(l = nn->layers[i - 1].out; l != NULL; l = l->cdr)
      if(nn->layers[i - 1].need_grads || l->link->need_grads ||
	 nn->need_all_grads)
	l->link->nfunc->backward(nn, l->link, &nn->layers[i - 1]);

    for(j = 0; j < nn->layers[i - 1].numslabs; j++) {
      
      slab = &nn->layers[i - 1].slabs[j];
      for(l = slab->out; l != NULL; l = l->cdr)
	if(slab->need_grads || l->link->need_grads || nn->need_all_grads)
	  l->link->nfunc->backward(nn, l->link, slab);

      if(slab->need_grads || nn->need_all_grads)
	for(k = 0; k < slab->sz; k++)
	  slab->dx[k] = slab->dy[k]  * 
	    slab->afunc->deriv(slab->x[k], slab->y[k]);
      else
	for(k = 0; k < slab->sz; k++)
	  slab->dx[k] = 0;
    }
  }
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
