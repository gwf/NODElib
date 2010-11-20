
/* Copyright (c) 1996 by Gary William Flake. */


/* This is a simple example of how to use NODElib.  -- GWF */

#include <nodelib.h>
#include <stdio.h>


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* This is a function that is called for each training epoch.  It
   receives a (void *) type because this hook is called by the
   optimization routines which do not know anything about the NN type;
   hence, you should cast the (void *) into a (NN *) in order for
   things to be correct.

   This hook simply prints out the epoch number and the current
   training error.

   The return value of zero indicates that nothing has gone wrong.  If
   it was non-zero, then the optimization routines would have assumed
   that you wanted training to be prematurely halted for whatever
   reason. */

int training_hook(void *obj)
{
  NN *nn = obj;
  
  fprintf(stderr, "%d\t%d\t%d\t%f\t%f\t%f\n",  nn->info.opt.epoch,
	  nn->info.opt.fcalls, nn->info.opt.gcalls,
	  nn->info.opt.stepsz, nn->info.opt.error,
	  nn->info.opt.decayed_error);
  return(0);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* This is a function that is called once per testing cycle, i.e., on
   every single pattern in a DATASET.  It is passed a (NN *) because
   this hook is called by the nn_offline_test() or nn_offline_grad()
   routines which know about the NN type.

   I am just using this to print out the neural network's output on
   the training data.

   A return value of zero means that everything went okay. */

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
  DATASET* data;
  NN *nn;
  double** H;
  int n;

  nn = nn_create("2 2 1");   /* 2-1-1 architecture. */

  nn_link(nn, "0 -l-> 1");   /* Inputs to hidden link. */
  nn_link(nn, "1 -l-> 2");   /* Hidden to output link. */

  nn_set_actfunc(nn, 1, 0, "logistic"); /* logistic act fcn */
  nn_set_actfunc(nn, 2, 0, "logistic");
  nn_init(nn, 1.0);       /* init nnet (-1,1) */

  /* create data set */
  data = dataset_create(&dsm_matrix_method,
        dsm_c_matrix(&xor_data[0][0], 2, 1, 4));

  /* training parameters */
  nn->info.train_set = data;      /* training data */
  nn->info.opt.min_epochs = 10;   /* min epochs */
  nn->info.opt.max_epochs = 10000; /* max epochs */
  nn->info.error_function = opt_err_cross_entropy;       
  nn->info.opt.momentum = 0.2;                           
  nn->info.opt.rate = 0.7;
  nn->info.opt.engine = opt_gradient_descent;            

  nn_train(nn);   /* train the network */
  printf("Epochs: %d\n",  nn->info.opt.epoch);
  nn_offline_test(nn, data, testing_hook);       

  /* calculate the Hessian */
  n = nn->numweights;
  H = allocate_array(2, sizeof(double), n, n);
  nn_offline_hessian(nn,data,H);
  //print_hessian(H,n); /* print the hessian */

  nn_destroy(nn);         /* free up memory */
  dsm_destroy_matrix(dataset_destroy(data));
  nn_shutdown();

  return 0;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

