
/* Copyright (c) 1996 by G. W. Flake. */


/* This is a simple example of how to use NODElib.  -- GWF */

#include <nodelib.h>
#include <stdio.h>
#include <math.h>

static char *help_string = "\
Hill-Plateau test for NRBFs.\n\
";

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
  
  fprintf(stderr, "%d\t%f\n", nn->info.opt.epoch,
	  sqrt(2 * nn->info.opt.error));
  return(0);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* This is a function that is called once per training cycle, i.e., on
   every single pattern in a DATASET.  It is passed a (NN *) because
   this hook is called by the nn_offline_test() or nn_offline_grad()
   routines which know about the NN type.

   I am just useing this to print out the neural network's output on
   the training data.

   A return value of zero means that everything went okay. */

int testing_hook(NN *nn)
{
  printf("% f\t% f\t% f\t% f\n", nn->x[0], nn->x[1], nn->y[0], nn->t[0]);
  return(0);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

int main(int argc, char **argv)
{
  /* These variables are for command-line options.
   */
  double var = 0.0;
  int seed = 0, nbasis = 4, norm = 0;
  
  /* The OPTION array is used to easily parse command-line options.
   */
  OPTION opts[] = {
    { "-var",    OPT_DOUBLE, &var,    "variance of basis functions"  },
    { "-seed",   OPT_INT,    &seed,   "random number seed"           },
    { "-nbasis", OPT_INT,    &nbasis, "number of basis functions"    },
    { "-norm",   OPT_SWITCH, &norm,   "normalized basis functions?"  },
    { NULL,      OPT_NULL,   NULL,    NULL                           }
  };

  /* The DATASET and the NN that we will use.
   */
  SERIES *trainser, *testser;
  DATASET *trainds, *testds;
  NN *nn;


  /* Get the command-line options.
   */
  get_options(argc, argv, opts, help_string, NULL, 0);

  /* Set the random seed.
   */
  srandom(seed);

  testser =  series_read_ascii("hp41.dat");
  testser->x_width = 2;
  testser->y_width = testser->offset = testser->x_delta = testser->y_delta = 1;
  testser->step = testser->x_width + testser->y_width;
  testds = dataset_create(&dsm_series_method, testser);

  trainser =  series_read_ascii("hp21.dat");
  trainser->x_width = 2;
  trainser->y_width = trainser->offset = trainser->x_delta = trainser->y_delta = 1;
  trainser->step = trainser->x_width + trainser->y_width;
  trainds = dataset_create(&dsm_series_method, trainser);

  nn_rbf_basis_normalized = norm;
  nn = nn_create_rbf(nbasis, var, trainds);

  nn->links[0]->need_grads = 1;
  nn->info.train_set = trainds;
  nn->info.opt.min_epochs = 20;
  nn->info.opt.max_epochs = 200;
  nn->info.opt.error_tol = 1e-3;
  nn->info.opt.delta_error_tol = 1e-8;
  nn->info.opt.hook = training_hook;
  nn->info.opt.stepf = opt_lnsrch_cubic;
  nn->info.opt.engine = opt_quasinewton_bfgs;
  nn_train(nn);

  /* Now, let's see how well the RBF performs.
   */
  nn_offline_test(nn, testds, testing_hook);

  /* Free up everything.
   */
  nn_destroy(nn);
  series_destroy(dataset_destroy(testds));
  series_destroy(dataset_destroy(trainds));
  nn_shutdown();

  /* Bye.
   */
  exit(0); 
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

