
/* Copyright (c) 1996 by Gary William Flake. */


/* This is a simple example of how to use NODElib.  -- GWF */

#include <nodelib.h>
#include <stdio.h>

static char *help_string = "\
An RBFN applied to multiple compositions of the logistic map.\n\
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
  int seed = 0, nbasis = 4, nout = 1, delay = 1, points = 100, norm = 0;

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
    { "-norm",   OPT_SWITCH, &norm,   "normalized basis functions?"  },
    { NULL,      OPT_NULL,   NULL,    NULL                           }
  };

  /* The DATASET and the NN that we will use.
   */
  DATASET *data;
  NN *nn;


  /* Get the command-line options.
   */
  get_options(argc, argv, opts, help_string, NULL, 0);

  /* Set the random seed.
   */
  srandom(seed);

  /* Make the data, and build an rbf from it.
   */
  data = make_data(points, noise, delay, nout);
  nn_rbf_basis_normalized = norm;
  nn = nn_create_rbf(nbasis, var, data);
  
  /* Now, let's see how well the RBF performs.  */
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

