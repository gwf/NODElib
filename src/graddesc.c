
/* Copyright (c) 1996 by G. W. Flake. */

#include <stdlib.h>
#include <stddef.h>
#include <math.h>

#define OWNER
#include "nodelib/optimize.h"
#undef OWNER

#include "nodelib/xalloc.h"
#include "nodelib/misc.h"

typedef struct CGDATA {
  double *d;
} GDDATA;

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void opt_gradient_descent(OPTIMIZER *opt, int state) 
{
  GDDATA *gdd = opt->internal;
  unsigned i;

  /* Initialize internal state. */
  if(state == 0) {
    gdd = opt->internal = xmalloc(sizeof(GDDATA));
    gdd->d = allocate_array(1, sizeof(double), opt->size);
    for(i = 0; i < opt->size; i++)
      gdd->d[i] = 0.0;
  }
  /* Do one gradiant descent step... */
  else if(state == 1) {
    opt_eval_grad(opt, NULL);
    for(i = 0; i < opt->size; i++)
      gdd->d[i] = opt->momentum * gdd->d[i] - opt->rate * *opt->grads[i];
    if(opt->stepf)
      opt->stepsz = opt->stepf(opt, gdd->d, opt->stepsz);
    else
      for(i = 0; i < opt->size; i++)
	*opt->weights[i] += gdd->d[i];
  }
  /* Clean up. */
  else if(state == -1) {
    deallocate_array(gdd->d);
    xfree(gdd);
    opt->internal = NULL;
  }
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
