
/* Copyright (c) 1996 by Gary William Flake. */


/* This is a simple example of how to use NODElib.  -- GWF */

#include <nodelib.h>
#include <stdio.h>
#include <math.h>

static char *help_string = "\
A CNLS network using the pair-wise product link applied to a\n\
continuous function.\n\
";

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

DATASET *make_data(int points, double noise)
{
  int i;
  double x, y;
  SERIES *ser;
  DATASET *data;

  ser = series_create();
  ser->y_width = ser->x_delta = ser->y_delta = ser->offset = 1;
  ser->x_width = 2; ser->step = 3;

  /* Fill up a SERIES with 'points' patterns that consists of a single
   * input, and 'nout' outputs.  Each output is a successive delay from
   * the logistic map.
   */
  for(i = 0; i < points; i++) {
    x = random_range(-1, 1);
    y = random_range(-1, 1);
    series_append_val(ser, x);
    series_append_val(ser, y);
    series_append_val(ser, sin(5 * x * y) + y);
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
  printf("%f %f %f %f\n", nn->x[0], nn->x[1], nn->t[0], nn->y[0]);
  return(0);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

int main(int argc, char **argv)
{
  /* These variables are for command-line options. */
  double noise = 0.0;
  int seed = 0, nbasis = 4, points = 100;

  /* The OPTION array is used to easily parse command-line options. */
  OPTION opts[] = {
    { "-noise",  OPT_DOUBLE, &noise,  "variance of Gaussian noise"   },
    { "-seed",   OPT_INT,    &seed,   "random number seed"           },
    { "-nbasis", OPT_INT,    &nbasis, "number of basis functions"    },
    { "-points", OPT_INT,    &points, "number of data points"        },
    { NULL,      OPT_NULL,   NULL,    NULL                           }
  };

  /* The DATASET and the NN that we will use. */
  DATASET *data;
  NN *nn;

  /* Get the command-line options.  */
  get_options(argc, argv, opts, help_string, NULL, 0);
  srandom(seed);

  /* Make the data, and build a CNLS net. */
  data = make_data(points, noise);
  nn = nn_create("2 (%d %d) %d 1", nbasis, nbasis, nbasis);
  nn_set_actfunc(nn, 1, 0, "linear");
  nn_set_actfunc(nn, 1, 1, "exp(-x)");
  nn_set_actfunc(nn, 2, 0, "linear");
  nn_set_actfunc(nn, 3, 0, "linear");

  nn_link(nn, "0 -l-> (1 0)");
  nn_link(nn, "0 -e-> (1 1)");
  nn_link(nn, "(1 1) -l-> 3");
  nn_link(nn, "(1 0) (1 1) -p-> 2");
  nn_link(nn, "2 -l-> 3");

  nn_init(nn, 1);

  nn->info.train_set = data;
  nn->info.opt.min_epochs = 10;
  nn->info.opt.max_epochs = 100;
  nn->info.opt.error_tol = 1e-5;
  nn->info.opt.delta_error_tol = 1e-7;
  nn->info.opt.hook = training_hook;
  nn_train(nn);

  /* Now, let's see how well the NN performs.
   */
  nn_offline_test(nn, data, testing_hook);

  /* Free up everything.
   */
  nn_destroy(nn);
  series_destroy(dataset_destroy(data));
  nn_shutdown();

  /* Bye.
   */
  exit(0); 
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

