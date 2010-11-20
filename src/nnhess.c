
/* Copyright (c) 1995 by G. W. Flake. */


#include <stdlib.h>
#include <math.h>

#include "nodelib/nn.h"
#include "nodelib/misc.h"

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void nn_jacobian(NN *nn, double *input, double *J)
{
  unsigned i, j, save;
  double *de_dy;

  save = nn->need_all_grads;
  nn->need_all_grads = 1;
  de_dy = allocate_array(1, sizeof(double), nn->numout);
  for(j = 0; j < nn->numout; j++)
    de_dy[j] = 0.0;

  for(i = 0; i < nn->numout; i++) {
    de_dy[i] = 1.0;
    nn_forward(nn, input);
    nn_backward(nn, de_dy);
    for(j = 0; j < nn->numin; j++)
      J[i * nn->numin + j] = nn->dx[j];
    de_dy[i] = 0.0;
  }
  deallocate_array(de_dy);
  nn->need_all_grads = save;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void nn_Rforward(NN *nn, double *Rinput, double *Rweights)
{
  unsigned i, j, k;
  NN_LAYER *slab;
  NN_LINKLIST *l;

  /* Clean up the R net input. */
  for(i = 0; i < nn->numlayers; i++)
    for(j = 0; j < nn->layers[i].sz; j++)
      nn->layers[i].Rx[j] = 0.0;

  /* Fill up the R input vector. */
  if(Rinput)
    for(i = 0; i < nn->numin; i++)
      nn->Rx[i] = Rinput[i];
  else
    for(i = 0; i < nn->numin; i++)
      nn->Rx[i] = 0.0;

  if(Rweights)
    nn_set_Rweights(nn, Rweights);
  else {
    double *Rw = allocate_array(1, sizeof(double), nn->numweights);
    for(i = 0; i < nn->numweights; i++)
      Rw[i] = 0;
    nn_set_Rweights(nn, Rw);
    deallocate_array(Rw);
  }
  
  /* For each layer... */
  for(i = 0; i < nn->numlayers; i++) {

    /* Compute the net input contributed by links coming into this layer. */
    for(l = nn->layers[i].in; l != NULL; l = l->cdr)
      l->link->nfunc->Rforward(nn, l->link, &nn->layers[i]);

    /* For each sublayer... */
    for(j = 0; j < nn->layers[i].numslabs; j++) {
      /* Compute the net input contributed by links coming into this
       * sub layer.
       */
      slab = &nn->layers[i].slabs[j];
      for(l = slab->in; l != NULL; l = l->cdr)
	l->link->nfunc->Rforward(nn, l->link, slab);

      /* Map net inputs through the activation functions. */
      for(k = 0; k < slab->sz; k++)
	slab->Ry[k] = slab->Rx[k] *
	  slab->afunc->deriv(slab->x[k], slab->y[k]);
    }
  }
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void nn_Rbackward(NN *nn, double *Rdoutput)
{
  unsigned i, j, k;
  double deriv, *Rg;
  NN_LAYER *slab;
  NN_LINKLIST *l;

  /* Clean up the Rderivatives. */
  Rg = allocate_array(1, sizeof(double), nn->numweights);
  for(i = 0; i < nn->numweights; i++)
    Rg[i] = 0.0;
  nn_set_Rgrads(nn, Rg);
  deallocate_array(Rg);

  for(i = 0; i < nn->numlayers; i++)
    for(j = 0; j < nn->layers[i].sz; j++)
      nn->layers[i].Rdy[j] = 0.0;

  /* Pass the partial of the error w.r.t. the output. */
  for(i = 0; i < nn->numout; i++)
    nn->Rdy[i] = Rdoutput[i];
 
  for(i = nn->numlayers; i > 0; i--) {

    for(l = nn->layers[i - 1].out; l != NULL; l = l->cdr)
      l->link->nfunc->Rbackward(nn, l->link, &nn->layers[i - 1]);

    for(j = 0; j < nn->layers[i - 1].numslabs; j++) {
      slab = &nn->layers[i - 1].slabs[j];

      for(l = nn->layers[i - 1].slabs[j].out; l != NULL; l = l->cdr)
	l->link->nfunc->Rbackward(nn, l->link, slab);

      for(k = 0; k < slab->sz; k++) {
	deriv = slab->afunc->deriv(slab->x[k], slab->y[k]);
	slab->Rdx[k] = slab->Rdy[k] * deriv +
	  slab->afunc->second_deriv(slab->x[k], slab->y[k], deriv) *
	    slab->Rx[k] * slab->dy[k];
      }
    }
  }
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void nn_Hv(NN *nn, double *input, double *target, double *v)
{
  double errsum, *dedy, *d2edy2, *Rg;
  unsigned i;
  
  errsum = 0.0;
  dedy = xmalloc(sizeof(double) * nn->numout);
  d2edy2 = xmalloc(sizeof(double) * nn->numout);
  nn_forward(nn, input);
  for(i = 0; i < nn->numout; i++) {
    errsum += nn->info.error_function(nn->y[i], target[i],
				      &dedy[i], &d2edy2[i]);
    nn->t[i] = target[i];
  }
  nn_backward(nn, dedy);
  nn_Rforward(nn, NULL, v);

  Rg = allocate_array(1, sizeof(double), nn->numweights);
  for(i = 0; i < nn->numweights; i++)
    Rg[i] = 0.0;
  nn_set_Rgrads(nn, Rg);
  deallocate_array(Rg);

  for(i = 0; i < nn->numout; i++)
     d2edy2[i] *= nn->Ry[i];
  nn_Rbackward(nn, d2edy2);
  
  xfree(dedy);
  xfree(d2edy2);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

int nn_hessian(NN *nn, double *input, double *target, double **H)
{
  double h1, h2, *Rw, *Rg;
  unsigned i, j;

  Rw = allocate_array(1, sizeof(double), nn->numweights);
  Rg = allocate_array(1, sizeof(double), nn->numweights);

  for(i = 0; i < nn->numweights; i++) {
    Rw[i] = 1.0;
    nn_Hv(nn, input, target, Rw);
    nn_get_Rgrads(nn, Rg);
    for(j = 0; j < nn->numweights; j++)
      H[j][i] = Rg[j];
    Rw[i] = 0.0;
  }

  /* The sanity check below compares the first 6 significant digits. */
  for(i = 0; i < nn->numweights; i++)
    for(j = 0; j < nn->numweights; j++) {
      if(i == j) continue;
      else if(H[i][j] == 0.0 && H[j][i] == 0.0) continue;
      else if(H[i][j] == 0.0 && fabs(H[j][i]) > 10e-100)
	goto BADHESSIAN;
      else if(fabs(H[i][j]) > 10e-100 && H[j][i] == 0.0)
	goto BADHESSIAN;
      else if(fabs(H[i][j]) > 10e-100 && fabs(H[j][i]) > 10e-100){
	h1 = pow(10.0, floor(log10(fabs(H[i][j]))));
	h2 = pow(10.0, floor(log10(fabs(H[j][i]))));
	if(h1 != h2)
	  goto BADHESSIAN;
	if(fabs(H[i][j] / h1 - H[j][i] / h2) > 10e-6)
	  goto BADHESSIAN;
      }
    }
  deallocate_array(Rw);
  deallocate_array(Rg);
  return(0);
  
 BADHESSIAN:
  deallocate_array(Rw);
  deallocate_array(Rg);
  ulog(ULOG_ERROR, "nn_hessian: Hessian failed sanity check."
       "%t(H[%d][%d] = %g) != (H[%d][%d] = %g).", i, j, H[i][j],
       j, i, H[j][i]);
  return(1);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

int nn_offline_hessian(NN *nn, DATASET *set, double **H)
{
  double *x, *t, **HH;
  unsigned i, j, k, pats;

  pats = dataset_size(set);
  if(dataset_x_size(set) != nn->numin || dataset_y_size(set) != nn->numout) {
    ulog(ULOG_ERROR, "nn_offline_hessian: I/O dimensions are incompatible.%t"
	 "NN dimension = (%d x %d)%tDATASET dimension = (%d x %d).",
	 nn->numin, nn->numout, dataset_x_size(set), dataset_y_size(set));
    return(-1);
  }

  for(i = 0; i < nn->numweights; i++)
    for(j = 0; j < nn->numweights; j++)
      H[i][j] = 0.0;
  HH = allocate_array(2, sizeof(double), nn->numweights, nn->numweights);
  for(k = 0; k < pats; k++) {
    x = dataset_x(set, k);
    t = dataset_y(set, k);
    if(nn_hessian(nn, x, t, HH)) {
      deallocate_array(HH);
      return(1);
    }
    for(i = 0; i < nn->numweights; i++)
      for(j = 0; j < nn->numweights; j++)
	H[i][j] += HH[i][j];
  }

  /* Removed chunk below per Dragan Obradovic's assertion that the
   * novelty detection scheme does not want the Hessian to be
   * normalized by the number of patterns.
   */

  for(i = 0; i < nn->numweights; i++)
    for(j = 0; j < nn->numweights; j++)
      H[i][j] /= pats;

  deallocate_array(HH);
  return(0);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

