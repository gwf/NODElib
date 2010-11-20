
/* Copyright (c) 1996 by Gary William Flake. */


/* This is a simple example of how to use NODElib.  -- GWF */

#include <nodelib.h>
#include <stdio.h>
#include <string.h>

static char *help_string = "\
An SMLP applied to multiple compositions of the logistic map.\n\
";

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

DATASET *make_data(int points, double noise, int delay, int nout)
{
  int i, j;
  double x, y;
  SERIES *ser;
  DATASET *data;

  ser = series_create();
  ser->x_width = ser->x_delta = ser->y_delta = ser->offset = 1;
  ser->y_width = nout;
  ser->step = nout + 1;

  /* Fill up a SERIES with 'points' patterns that consists of a single
   * input, and 'nout' outputs.  Each output is a successive delay from
   * the logistic map.
   */
  for(i = 0; i < points; i++) {
    x = y = random_range(0, 1);
    for(j = 0; j < delay; j++)
      y = 4 * y * (1 - y);
    series_append_val(ser, x);
    for(j = 0; j < nout; j++) {
      series_append_val(ser, y + noise * random_gauss());
      y = 4 * y * (1 - y);
    }
  }

  data = dataset_create(&dsm_series_method, ser);
  return(data);
}

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
  
  fprintf(stderr, "%d\t%f\n", nn->info.opt.epoch, nn->info.opt.error);
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
  int i;

  printf("%f", nn->x[0]);
  for(i = 0; i < nn->numout; i++)
    printf("\t%f", nn->y[i]);
  printf("\n");

  return(0);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

int main(int argc, char **argv)
{
  /* These variables are for command-line options.
   */
  double var = 0.5, noise = 0.0;
  int seed = 0, nbasis = 4, nout = 1, delay = 1, points = 100, steps = 50;
  char *alg = "qnbfgs";
  
  /* The OPTION array is used to easily parse command-line options.
   */
  OPTION opts[] = {
    { "-var",    OPT_DOUBLE, &var,    "variance of basis functions"  },
    { "-noise",  OPT_DOUBLE, &noise,  "variance of Gaussian noise"   },
    { "-seed",   OPT_INT,    &seed,   "random number seed"           },
    { "-nbasis", OPT_INT,    &nbasis, "number of basis functions"    },
    { "-nout",   OPT_INT,    &nout,   "number of outputs"            },
    { "-delay",  OPT_INT,    &delay,  "delays in logistic map data"  },
    { "-points", OPT_INT,    &points, "number of data points"        },
    { "-alg",    OPT_STRING, &alg,    "training algorithm"           },
    { "-steps",  OPT_INT,    &steps,  "number extra training steps"  },
    { NULL,      OPT_NULL,   NULL,    NULL                           }
  };

  /* The DATASET and the NN that we will use.  */
  DATASET *data;
  NN *nn;


  /* Get the command-line options.  */
  get_options(argc, argv, opts, help_string, NULL, 0);

  /* Set the random seed. */
  srandom(seed);

  /* Make the data, and build an rbf from it.  */
  data = make_data(points, noise, delay, nout);
  nn = nn_create_smlp(nbasis, var, data);

#if 0
  nn_init(nn, 1.0);
#endif

  /* Tell the NN how to train itself, and do it. */
  nn->info.train_set = data;
  nn->info.opt.min_epochs = steps;
  nn->info.opt.max_epochs = steps;
  nn->info.opt.hook = training_hook;
  nn->info.opt.engine = opt_conjgrad_pr;
  if(strcmp(alg, "cgfr") == 0)
    nn->info.opt.engine = opt_conjgrad_fr;
  else if(strcmp(alg, "qndfp") == 0)
    nn->info.opt.engine = opt_quasinewton_dfp;
  else if(strcmp(alg, "qnbfgs") == 0)
    nn->info.opt.engine = opt_quasinewton_bfgs;

  nn_train(nn);

  /* Now, let's see how well the SMLP performs.  */
  nn_offline_test(nn, data, testing_hook);

  /* Free up everything. */
  nn_destroy(nn);
  series_destroy(dataset_destroy(data));
  nn_shutdown();

  /* Bye. */
  exit(0); 
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

