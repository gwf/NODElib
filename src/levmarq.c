
/* Copyright (c) 1998 by G. W. Flake. */

#include <stdlib.h>
#include <stddef.h>
#include <math.h>

#include "nodelib/xalloc.h"
#include "nodelib/misc.h"
#include "nodelib/optimize.h"
#include "nodelib/svd.h"

typedef struct LMDATA {
  double *xo, *go, *hxd;
  double **ho, **hn, **hh, **hi;
  double lambda, last_error;
} LMDATA;

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void opt_levenberg_marquardt(OPTIMIZER *opt, int state)
{
  LMDATA *lmd = opt->internal;
  unsigned i, j, n = opt->size;
  double **t, xdd, sum;

  /* Initialize internal state. */
  if(state == 0) {
    lmd = opt->internal = xmalloc(sizeof(LMDATA));
    lmd->hxd = allocate_array(1, sizeof(double), n);
    lmd->go = allocate_array(1, sizeof(double), n);
    lmd->xo = allocate_array(1, sizeof(double), n);
    lmd->ho = allocate_array(2, sizeof(double), n, n);
    lmd->hn = allocate_array(2, sizeof(double), n, n);
    lmd->hh = allocate_array(2, sizeof(double), n, n);
    lmd->hi = allocate_array(2, sizeof(double), n, n);

    for(i = 0; i < n; i++) {
      for(j = 0; j < n; j++)
	lmd->ho[i][j] = (i == j) ? 1 : 0;
      lmd->xo[i] = 0.0;
      lmd->go[i] = 0.0;
    }

    lmd->lambda = 10;
    lmd->last_error = 10;
  }
  /* Do one Levenberg-Marquardt step... */
  else if(state == 1) {
    xdd = 0;
    for(i = 0; i < n; i++) {
      sum = 0;
      for(j = 0; j < n; j++)
	sum += lmd->ho[i][j] * (*opt->weights[j] - lmd->xo[j]);
      lmd->hxd[i] = sum;
      xdd += (*opt->weights[i] - lmd->xo[i]) *
	(*opt->weights[i] - lmd->xo[i]);
    }    
    if(xdd == 0) xdd = 1;
    for(i = 0; i < n; i++)
      for(j = 0; j < n; j++)
	lmd->hn[i][j] = lmd->ho[i][j] + 
	  (*opt->grads[i] - lmd->go[i] - lmd->hxd[i]) * 
	  (*opt->weights[j] - lmd->xo[j]) / xdd;
    for(i = 0; i < n; i++) {
      lmd->go[i] = *opt->grads[i];
      lmd->xo[i] = *opt->weights[i];
    }
    while(1) {
      for(i = 0; i < n; i++)
	for(j = 0; j < n; j++)
	  lmd->hh[i][j] = (i != j) ? lmd->hn[i][j] :
	    (1 + lmd->lambda) * lmd->hn[i][j];      
      
      pinv(lmd->hh, lmd->hi, n);
      
      for(i = 0; i < n; i++) {
	sum = 0;
	for(j = 0; j < n; j++)
	  sum += lmd->hi[i][j] * *opt->grads[j];
	*opt->weights[i] = lmd->xo[i] - sum;
      }
      
      opt_eval_func(opt, NULL);

      if(opt->epoch > 1 && opt->error < lmd->last_error) {
	if(lmd->lambda > 10e-10) lmd->lambda /= 10;
	return;
      }
      else {
	if(lmd->lambda < 10e10) lmd->lambda *= 10;
	else return;	  
      }
    }
     
    t = lmd->ho; lmd->ho = lmd->hn; lmd->hn = t;
    lmd->last_error = opt->error;

    if(lmd->lambda > 10e10) {
      for(i = 0; i < n; i++)
	for(j = 0; j < n; j++)
	  lmd->ho[i][j] = (i == j) ? 1 : 0;
    }
  }
  /* Clean up. */
  else if(state == -1) {
    deallocate_array(lmd->hxd);
    deallocate_array(lmd->go);
    deallocate_array(lmd->xo);
    deallocate_array(lmd->ho);
    deallocate_array(lmd->hn);
    deallocate_array(lmd->hh);
    deallocate_array(lmd->hi);
    xfree(lmd);
    opt->internal = NULL;
  }
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

