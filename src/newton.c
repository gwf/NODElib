
/* Copyright (c) 1995, 1996
   by G. W. Flake. */

#include <stdlib.h>
#include <stddef.h>
#include <math.h>

#include "nodelib/xalloc.h"
#include "nodelib/misc.h"
#include "nodelib/optimize.h"

typedef struct QNDATA {
  double *xd, *gd, *xo, *go, *hg, *u, *d;
  double **xdc, **hgc, **uc, **ho, **hn;
} QNDATA;

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static void generic_quasinewton(OPTIMIZER *opt, int state, int BFGS)
{
  QNDATA *qnd = opt->internal;
  unsigned i, j, n = opt->size;
  double **t, sum, xdgd, gdhd;

  /* Initialize internal state. */
  if(state == 0) {
    qnd = opt->internal = xmalloc(sizeof(QNDATA));
    qnd->xd = allocate_array(1, sizeof(double), n);
    qnd->gd = allocate_array(1, sizeof(double), n);
    qnd->xo = allocate_array(1, sizeof(double), n);
    qnd->go = allocate_array(1, sizeof(double), n);
    qnd->hg = allocate_array(1, sizeof(double), n);
    qnd->u = allocate_array(1, sizeof(double), n);
    qnd->d = allocate_array(1, sizeof(double), n);
    qnd->xdc = allocate_array(2, sizeof(double), n, n);
    qnd->hgc = allocate_array(2, sizeof(double), n, n);
    qnd->uc = allocate_array(2, sizeof(double), n, n);
    qnd->ho = allocate_array(2, sizeof(double), n, n);
    qnd->hn = allocate_array(2, sizeof(double), n, n);

    for(i = 0; i < n; i++) {
      for(j = 0; j < n; j++)
	qnd->ho[i][j] = (i == j) ? 1 : 0;
      qnd->xo[i] = qnd->go[i] = 0.0;
    }
    opt->stepsz = 0.0;
  }
  /* Do one quasi-Newton step. */
  else if(state == 1) {
    opt_eval_grad(opt, NULL);

    for(i = 0; i < n; i++) {
      qnd->xd[i] = *opt->weights[i] - qnd->xo[i];
      qnd->gd[i] = *opt->grads[i] - qnd->go[i];
    }
    for(i = 0; i < n; i++) {
      for(sum = 0, j = 0; j < n; j++)
	sum += qnd->ho[i][j] * qnd->gd[j];
      qnd->hg[i] = sum;
    }
    xdgd = gdhd = 0;
    for(i = 0; i < n; i++) {
      for(j = 0; j < n; j++) {
	qnd->xdc[i][j] = qnd->xd[i] * qnd->xd[j];
	qnd->hgc[i][j] = qnd->hg[i] * qnd->hg[j];
      }
      xdgd += qnd->xd[i] * qnd->gd[i];
      gdhd += qnd->gd[i] * qnd->hg[i];
    }
    
    if(xdgd == 0) xdgd = 1;
    if(gdhd == 0) gdhd = 1;

    if(BFGS) {
      for(i = 0; i < n; i++)
	qnd->u[i] = qnd->xd[i] / xdgd - qnd->hg[i] / gdhd;
      for(i = 0; i < n; i++)
	for(j = 0; j < n; j++)
	  qnd->uc[i][j] = qnd->u[i] * qnd->u[j];
    }
    
    for(i = 0; i < n; i++) {
      for(j = 0; j < n; j++) {
	qnd->hn[i][j] = qnd->ho[i][j] + qnd->xdc[i][j] /
	  xdgd - qnd->hgc[i][j] / gdhd;
	if(BFGS)
	  qnd->hn[i][j] += qnd->uc[i][j] * gdhd;
      }
    }
    
    for(i = 0; i < n; i++) {
      sum = 0;
      for(j = 0; j < n; j++)
	sum += qnd->ho[i][j] * *opt->grads[j];
      qnd->d[i] = -sum;
    }
   
    for(i = 0; i < n; i++) {
      qnd->go[i] = *opt->grads[i];
      qnd->xo[i] = *opt->weights[i];
    }

    t = qnd->ho; qnd->ho = qnd->hn; qnd->hn = t;
    
    opt->stepsz = (opt->stepf ? opt->stepf(opt, qnd->d, opt->stepsz) :
      opt_lnsrch_cubic(opt, qnd->d, opt->stepsz));
  }
  /* Clean up. */
  else if(state == -1) {
    deallocate_array(qnd->xd);
    deallocate_array(qnd->gd); 
    deallocate_array(qnd->hg);
    deallocate_array(qnd->go);
    deallocate_array(qnd->xo);
    deallocate_array(qnd->u);
    deallocate_array(qnd->d);
    deallocate_array(qnd->xdc);
    deallocate_array(qnd->hgc);
    deallocate_array(qnd->uc);
    deallocate_array(qnd->ho);
    deallocate_array(qnd->hn);
    xfree(qnd);
    opt->internal = NULL;
  }
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void opt_quasinewton_dfp(OPTIMIZER *opt, int state)
{
  generic_quasinewton(opt, state, 0);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void opt_quasinewton_bfgs(OPTIMIZER *opt, int state)
{
  generic_quasinewton(opt, state, 1);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

