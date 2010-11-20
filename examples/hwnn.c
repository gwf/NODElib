
/* Copyright (c) 1996 by Gary William Flake. */


/* This is a simple example of how to use NODElib.  -- GWF */

#include <nodelib.h>
#include <stdio.h>

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

int testing_hook(NN *nn)
{
  printf("%d %d --> %f\n", (int)nn->x[0], (int)nn->x[1], nn->y[0]);
  return(0);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* I am using a single C matrix to hold my my training data. */

static double xor_data[4][3] = {
  { 0.0, 0.0, 0.0 },
  { 1.0, 0.0, 1.0 },
  { 0.0, 1.0, 1.0 },
  { 1.0, 1.0, 0.0 },
};  

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

int main(int argc, char **argv)
{
  DATASET *data;
  NN *nn;

  nn = nn_create("2 1 1");   /* 2-1-1 architecture. */
  nn_link(nn, "0 -l-> 1");   /* Inputs to hidden link. */
  nn_link(nn, "1 -l-> 2");   /* Hidden to output link. */
  nn_link(nn, "0 -l-> 2");   /* Input to output short-circuit link. */  

  nn_set_actfunc(nn, 1, 0, "logistic");
  nn_set_actfunc(nn, 2, 0, "tanh");
  nn_init(nn, 1.0);

  data = dataset_create(&dsm_matrix_method,
			dsm_c_matrix(&xor_data[0][0], 2, 1, 4));

  nn->info.train_set = data;
  nn->info.opt.stepf = opt_lnsrch_golden;
  nn->info.opt.engine = opt_conjgrad_pr;
  nn->info.opt.min_epochs = 10;
  nn->info.opt.max_epochs = 100;
  nn->info.opt.error_tol = 1e-3;

  nn_train(nn);
  nn_offline_test(nn, data, testing_hook);

  return 0; 
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

