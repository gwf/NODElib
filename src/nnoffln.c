
/* Copyright (c) 1995 by G. W. Flake. */

#include <stdlib.h>
#include <math.h>

#include "nodelib/nn.h"
#include "nodelib/misc.h"
#include "nodelib/dataset.h"
#include "nodelib/optimize.h"

double nn_offline_bignum_skip = 0.0;

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

double nn_offline_test(NN *nn, DATASET *set, int (*hook)(NN *nn))
{
  double errsum, deriv, deriv2, *x, *t, rmse;
  unsigned i, j, pats, index, maxi, cont_flag, totalouts = 0;

  nn->info.subsample = fabs(nn->info.subsample);

  pats = dataset_size(set);
  if(dataset_x_size(set) != nn->numin || dataset_y_size(set) != nn->numout) {
    ulog(ULOG_ERROR, "nn_offline_test: I/O dimensions are incompatible.%t"
	 "NN dimension = (%d x %d)%tDATASET dimension = (%d x %d).",
	 nn->numin, nn->numout, dataset_x_size(set), dataset_y_size(set));
    return(-1.0);
  }
  errsum = rmse = 0.0;
  
  maxi = (int) ((nn->info.subsample == 0) ? pats :
		(nn->info.subsample > 0 && nn->info.subsample < 1) ? 
		pats * nn->info.subsample + 0.5 :
		(nn->info.subsample < pats) ? nn->info.subsample : pats);

  for(i = 0; i < maxi; i++) {
    if(nn->info.subsample == 0.0)
      index = i;
    else
      index = random() % pats;	
    x = dataset_x(set, index);
    t = dataset_y(set, index);

    /* Check for funky conditions. */
    cont_flag = 0;
    for(j = 0; j < nn->numin; j++)
      if(x[j] != x[j]) {
	cont_flag = 1;
	break;
      }
    if(cont_flag) continue;

    if(nn_offline_bignum_skip != 0.0)
      for(j = 0; j < nn->numin; j++)
	if(fabs(x[j]) >= fabs(nn_offline_bignum_skip)) {
	  cont_flag = 1;
	  break;
	}
    if(cont_flag) continue;

    nn_forward(nn, x);
    for(j = 0; j < nn->numout; j++) {
      
      /* Check for funky conditions. */
      if(t[j] == t[j] && (nn_offline_bignum_skip == 0.0 ||
			  fabs(t[j]) < fabs(nn_offline_bignum_skip))) {
	errsum += nn->info.error_function(nn->y[j], t[j], &deriv, &deriv2);
	rmse += (nn->y[j] - t[j]) * (nn->y[j] - t[j]);
	totalouts++;
      }

      nn->t[j] = t[j];
    }
    if(hook)
      hook(nn);
  }
  nn->info.error = errsum / totalouts;
  nn->info.rmse = sqrt(rmse / totalouts);
  return(nn->info.error);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

double nn_offline_grad(NN *nn, DATASET *set, int (*hook)(NN *nn))
{
  double *gall;
  double errsum, *x, *t, *dedy, *d2edy2, rmse;
  unsigned i, j, pats, maxi, index, cont_flag, totalouts = 0;

  nn->info.subsample = fabs(nn->info.subsample);

  pats = dataset_size(set);
  if(dataset_x_size(set) != nn->numin || dataset_y_size(set) != nn->numout) {
    ulog(ULOG_ERROR, "nn_offline_grad: I/O dimensions are incompatible.%t"
	 "NN dimension = (%d x %d)%tDATASET dimension = (%d x %d).",
	 nn->numin, nn->numout, dataset_x_size(set), dataset_y_size(set));
    return(-1.0);
  }
  errsum = rmse = 0.0;
  dedy = xmalloc(sizeof(double) * nn->numout);
  d2edy2 = xmalloc(sizeof(double) * nn->numout);
  gall = allocate_array(1, sizeof(double), nn->numweights);
  for(i = 0; i < nn->numweights; i++)
    gall[i] = 0.0;

  maxi = (int) ((nn->info.subsample == 0) ? pats :
		(nn->info.subsample > 0 && nn->info.subsample < 1) ?
		pats * nn->info.subsample + 0.5 :
		(nn->info.subsample < pats) ? nn->info.subsample : pats);

  for(i = 0; i < maxi; i++) {
    if(nn->info.subsample == 0.0)
      index = i;
    else
      index = random() % pats;	

    x = dataset_x(set, index);
    t = dataset_y(set, index);

    /* Check for funky conditions. */
    cont_flag = 0;
    for(j = 0; j < nn->numin; j++)
      if(x[j] != x[j]) {
	cont_flag = 1;
	break;
      }
    if(cont_flag) continue;

    if(nn_offline_bignum_skip != 0.0)
      for(j = 0; j < nn->numin; j++)
	if(fabs(x[j]) >= fabs(nn_offline_bignum_skip)) {
	  cont_flag = 1;
	  break;
	}
    if(cont_flag) continue;

    nn_forward(nn, x);
    for(j = 0; j < nn->numout; j++) {

      /* Check for funky conditions. */
      if(t[j] == t[j] && (nn_offline_bignum_skip == 0.0 ||
			  fabs(t[j]) < fabs(nn_offline_bignum_skip))) {
	errsum += nn->info.error_function(nn->y[j], t[j],
					  &dedy[j], &d2edy2[j]);
	rmse += (nn->y[j] - t[j]) * (nn->y[j] - t[j]);
	totalouts++;
      }
      else
	dedy[j] = d2edy2[j] = 0;

      nn->t[j] = t[j];
    }
    nn_backward(nn, dedy);
    for(j = 0; j < nn->numweights; j++)
      gall[j] += *nn->grads[j];
    if(hook)
      hook(nn);
  }
  for(j = 0; j < nn->numweights; j++)
    /* *nn->grads[j] = gall[j] / (nn->numout * pats); */
    *nn->grads[j] = gall[j] / totalouts;

  deallocate_array(gall);
  xfree(dedy);
  xfree(d2edy2);
  nn->info.error = errsum / totalouts;
  nn->info.rmse = sqrt(rmse / totalouts);
  return(nn->info.error);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static double nn_gradf_wrapper(void *obj)
{
  NN *nn = obj;

  return(nn_offline_grad(nn, nn->info.train_set, NULL));
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static double nn_funcf_wrapper(void *obj)
{
  NN *nn = obj;

  return(nn_offline_test(nn, nn->info.train_set, NULL));
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* Check for cross validation condition. */

static double last_test_error;

static int nn_haltf_wrapper(void *obj)
{
  NN *nn = obj;
  
  double new_test_error;
  unsigned result = 0;

  if(nn->info.test_set) {
    new_test_error = nn_offline_test(nn, nn->info.test_set, NULL);
    if(new_test_error > last_test_error)
      result = 1;
  }
  return(result);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

int nn_train(NN *nn)
{
  /* Check for the sanity of the train and test pattern sets. */

  if(!nn->info.train_set) {
    ulog(ULOG_ERROR, "nn_train: no training patterns specified.");
    return(1);
  }
  
  if(dataset_x_size(nn->info.train_set) != nn->numin ||
     dataset_y_size(nn->info.train_set) != nn->numout) {
    ulog(ULOG_ERROR, "nn_train: train I/O dimensions are"
	 " incompatible.%tNN dimension = (%d x %d)%tDATASET "
	 "dimension = (%d x %d).", nn->numin, nn->numout,
	 dataset_x_size(nn->info.train_set),
	 dataset_y_size(nn->info.train_set));
    return(1);
  }
  if(nn->info.test_set && (dataset_x_size(nn->info.test_set) != nn->numin ||
     dataset_y_size(nn->info.test_set) != nn->numout)) {
    ulog(ULOG_ERROR, "nn_train: test I/O dimensions are"
	 " incompatible.%tNN dimension = (%d x %d)%tDATASET "
	 "dimension = (%d x %d).", nn->numin, nn->numout,
	 dataset_x_size(nn->info.test_set),
	 dataset_y_size(nn->info.test_set));
    return(1);
  }
  
  if(!nn->info.opt.engine) {
    ulog(ULOG_ERROR, "nn_train: no algorithm specified.");
    return(1);
  }

  last_test_error = 10e20;

  /* Fill up the opt structure. */

  nn->info.opt.size = nn->numweights;
  nn->info.opt.weights = nn->weights;
  nn->info.opt.grads = nn->grads;
  nn->info.opt.obj = nn;

  nn->info.opt.gradf = nn_gradf_wrapper;
  nn->info.opt.funcf = nn_funcf_wrapper;
  nn->info.opt.haltf = nn_haltf_wrapper;

  /* Do the training and collect the statistics. */

  optimize(&nn->info.opt);

  return(0);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

