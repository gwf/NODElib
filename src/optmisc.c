
/* Copyright (c) 1995 by G. W. Flake. */

/* Handy routines to simplify the interface to the optimizers. */

#include <stdlib.h>
#include "nodelib/optimize.h"
#include "nodelib/ulog.h"
#include "nodelib/misc.h"

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

double opt_eval_func(OPTIMIZER *opt, double *weights)
{
  unsigned i;

  if(opt->stochastic) srandom(opt->seed);
  if(weights)
    for(i = 0; i < opt->size; i++)
      *opt->weights[i] = weights[i];
  opt->error = opt->funcf(opt->obj);
  if(opt->wdecay) {
    double sum = 0;
    for(i = 0; i < opt->size; i++)
      sum += *opt->weights[i] * *opt->weights[i];
    opt->error += opt->wdecay * sum;
  }
  opt->fcalls++;
  return(opt->error);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

double opt_eval_grad(OPTIMIZER *opt, double *weights)
{
  unsigned i;

  if(opt->stochastic) srandom(opt->seed);
  if(weights)
    for(i = 0; i < opt->size; i++)
      *opt->weights[i] = weights[i];
  opt->error = opt->gradf(opt->obj);
  if(opt->wdecay) {
    double sum = 0;
    for(i = 0; i < opt->size; i++)
      sum += *opt->weights[i] * *opt->weights[i];
    opt->error += opt->wdecay * sum;
    for(i = 0; i < opt->size; i++)
      *opt->grads[i] += 2 * opt->wdecay * *opt->weights[i];
  }
  opt->gradmag = 0;
  for(i = 0; i < opt->size; i++)
    opt->gradmag += *opt->grads[i] * *opt->grads[i];
  opt->gcalls++;
  return(opt->error);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

int optimize(OPTIMIZER *opt)
{
  double last_error, last_decayed_error, last_decayed_delta_error;

  if(opt->engine == NULL) {
    ulog(ULOG_ERROR, "optimizer: OPTIMIZER engine is NULL.");
    return(1);
  }

  /* Initialize internal state. */
  opt->engine(opt, 0);

  opt->fcalls = opt->gcalls = 0;
  opt->error = opt->decayed_error = 0;
  opt->delta_error = opt->decayed_delta_error = 0;

  for(opt->epoch = 1; opt->epoch <= opt->max_epochs; opt->epoch++) {

    last_error = opt->error;
    last_decayed_error = opt->decayed_error;
    last_decayed_delta_error = opt->decayed_delta_error;
    
    /* Do one optimization step. */
    opt->engine(opt, 1);

    /* Update statistics. */
    if(opt->epoch == 1)
      opt->decayed_error = opt->error;
    else
      opt->decayed_error = opt->decay * last_decayed_error +
	(1 - opt->decay) * opt->error;

    if(opt->epoch == 1)
      opt->decayed_delta_error = 0;
    else if(opt->epoch == 2)
      opt->decayed_delta_error = last_decayed_error - opt->decayed_error;
    else
      opt->decayed_delta_error = opt->decay * last_decayed_delta_error +
	(1 - opt->decay) * (last_decayed_error - opt->decayed_error);

    opt->delta_error = last_error - opt->error;

    /* Do the hook if needed. */
    if(opt->hook && opt->hook_freq && (opt->epoch % opt->hook_freq) == 0)
      if(opt->hook(opt->obj) != 0)
	break;

    /* Check for alternate halting conditions. */
    if(opt->haltf)
      if(opt->haltf(opt->obj) != 0) {
	/* opt->badness = 2; */
	break;
      }

    /* Increment stochastic seed, if appropriate. */
    if(opt->stochastic)
      opt->seed++;

    /* Check the stopping criterion. */
    if(opt->epoch < opt->min_epochs)
      continue;
    if(opt->error < opt->error_tol)
      break;
    if(opt->delta_error < opt->delta_error_tol)
      break;
    if(opt->stochastic && opt->decayed_error < opt->decayed_error_tol)
      break;
    if(opt->stochastic && opt->decayed_delta_error <
       opt->decayed_delta_error_tol)
      break;
  }

  /* Clean up. */
  opt->engine(opt, -1);

  return(0);
}


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

