
/* Copyright (c) 1995, 1996
   by G. W. Flake. */

#include <stdlib.h>
#include <stddef.h>
#include <math.h>

#include "nodelib/misc.h"
#include "nodelib/xalloc.h"
#include "nodelib/optimize.h"

typedef struct CGDATA {
  double *g, *d;
} CGDATA;

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void generic_conjgrad(OPTIMIZER *opt, int state, int polak_ribiere)
{
  CGDATA *cgd = opt->internal;
  unsigned i;
  double gg, dgg, beta;

  /* Initialize internal state. */
  if(state == 0) {
    cgd = opt->internal = xmalloc(sizeof(CGDATA));
    cgd->g = allocate_array(1, sizeof(double), opt->size);
    cgd->d = allocate_array(1, sizeof(double), opt->size);
    for(i = 0; i < opt->size; i++)
      cgd->g[i] = cgd->d[i] = 0.0;
  }
  /* Do one conjugate gradiant step... */
  else if(state == 1) {
    opt_eval_grad(opt, NULL);

    if(opt->epoch == 1)
      beta = 0.0;
    else {
      gg = dgg = 0.0;
      for(i = 0; i < opt->size; i++) {
        gg += cgd->g[i] * cgd->g[i];
        if(polak_ribiere)
          dgg += (*opt->grads[i] - cgd->g[i]) * *opt->grads[i];
        else
	  dgg += *opt->grads[i] * *opt->grads[i];
      }
      if(gg == 0.0)
	return;
      else
	beta = dgg / gg;
    }
    for(i = 0; i < opt->size; i++) {
      cgd->d[i] = - *opt->grads[i] + beta * cgd->d[i];
      cgd->g[i] = *opt->grads[i];
    }
    if(opt->stochastic || opt->epoch == 1)
      opt->stepsz = 0;
    if(opt->stepf)
      opt->stepsz = opt->stepf(opt, cgd->d, opt->stepsz);
    else {
      for(i = 0; i < opt->size; i++)
	*opt->weights[i] += opt->rate * cgd->d[i];
      opt->stepsz = opt->rate;
    }
  }
  /* Clean up. */
  else if(state == -1) {
    deallocate_array(cgd->g);
    deallocate_array(cgd->d);
    xfree(cgd);
    opt->internal = NULL;
  }
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void opt_conjgrad_pr(OPTIMIZER *opt, int state)
{
  generic_conjgrad(opt, state, 1);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void opt_conjgrad_fr(OPTIMIZER *opt, int state)
{
  generic_conjgrad(opt, state, 0);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
