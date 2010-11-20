
/* Copyright (c) 1996 by G. W. Flake. */


/* This is a simple example of how to use NODElib.  -- GWF */

#include <nodelib.h>
#include <stdio.h>

static char *help_string = "\
Compute the Jacobian is a NN.
";

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

double f1(double x1, double x2)
{
  return(x1*x1 - 2*x2*x2 + 3*x1*x2 - 4*x1 + 5*x2 + 6);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

double df1dx1(double x1, double x2)
{
  return(x1 + 3*x2 - 4);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

double df1dx2(double x1, double x2)
{
  return(-2*x2 + 3*x1 + 5);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

double f2(double x1, double x2)
{
  return(6*x1*x1 - 5*x2*x2 + 4*x1*x2 - 3*x1 + 2*x2 + 1);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

double df2dx1(double x1, double x2)
{
  return(6*x1 + 4*x2 - 3);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

double df2dx2(double x1, double x2)
{
  return(-5*x2 + 4*x1 + 2);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

int training_hook(void *obj)
{
  NN *nn = obj;
  
  fprintf(stderr, "%d\t%f\n", nn->info.opt.epoch, nn->info.opt.error);
  return(0);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

DATASET *make_data(int points)
{
  int i;
  double x1, x2;
  SERIES *ser;
  DATASET *data;

  ser = series_create();
  ser->x_width = ser->y_width = 2;
  ser->x_delta = ser->y_delta = ser->offset = 1;
  ser->step = 4;

  for(i = 0; i < points; i++) {
    x1 = random_range(-1, 1);
    x2 = random_range(-1, 1);
    series_append_val(ser, x1);
    series_append_val(ser, x2);
    series_append_val(ser, f1(x1, x2));
    series_append_val(ser, f2(x1, x2));
  }

  data = dataset_create(&dsm_series_method, ser);
  return(data);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

int main(int argc, char **argv)
{
  /* These variables are for command-line options. */
  double var = 0.25, *x, **J, err;
  int seed = 0, nbasis = 12, points = 200, i;

  /* The OPTION array is used to easily parse command-line options. */
  OPTION opts[] = {
    { "-var",    OPT_DOUBLE, &var,    "variance of basis functions"  },
    { "-seed",   OPT_INT,    &seed,   "random number seed"           },
    { "-nbasis", OPT_INT,    &nbasis, "number of basis functions"    },
    { "-points", OPT_INT,    &points, "number of data points"        },
    { NULL,      OPT_NULL,   NULL,    NULL                           }
  };

  /* The DATASET and the NN that we will use. */
  DATASET *data;
  NN *nn;

  /* Get the command-line options. */
  get_options(argc, argv, opts, help_string, NULL, 0);

  srandom(seed);

  /* Make the data, and build an rbf from it. */
  data = make_data(points);

  nn = nn_create("2 2");
  nn_link(nn, "0 -q-> 1");
  nn_set_actfunc(nn, 1, 0, "linear");

  nn->info.train_set = data;
  nn->info.opt.min_epochs = 10;
  nn->info.opt.max_epochs = 25;
  nn->info.opt.error_tol = 10e-5;
  nn->info.opt.delta_error_tol = 10e-6;
  nn->info.opt.hook = training_hook;
  nn->info.opt.engine = opt_quasinewton_bfgs;
  nn_train(nn);

  J = allocate_array(2, sizeof(double), 2, 2);
  
  /* Now test to see of nn_jacobian() works. */
  for(i = 0; i < points; i++) {
    x = dataset_x(data, i);
    nn_jacobian(nn, x, &J[0][0]);

#if 0
    printf("% 2.2f\t% 2.2f\t% 2.2f\t% 2.2f\n", nn->x[0], nn->x[1],
	   nn->y[0], nn->y[1]);
    printf("% 2.2f\t% 2.2f\t% 2.2f\t% 2.2f\n", J[0][0], J[0][1],
	   J[1][0], J[1][1]);
    printf("% 2.2f\t% 2.2f\t% 2.2f\t% 2.2f\n", df1dx1(x[0], x[1]),
	   df1dx2(x[0], x[1]), df2dx1(x[0], x[1]), df2dx2(x[0], x[1]));
    printf("--\n");
#endif
#if 1
    err = J[0][0] - df1dx1(x[0], x[1]);
    err = err * err;
    printf("% 2.2f\t", err);

    err = J[0][1] - df1dx2(x[0], x[1]);
    err = err * err;
    printf("% 2.2f\t", err);

    err = J[1][0] - df2dx1(x[0], x[1]);
    err = err * err;
    printf("% 2.2f\t", err);

    err = J[1][1] - df2dx2(x[0], x[1]);
    err = err * err;
    printf("% 2.2f\n", err);
#endif
  }

  /* Free up everything. */
  deallocate_array(J);
  nn_destroy(nn);
  series_destroy(dataset_destroy(data));
  nn_shutdown();

  exit(0); 
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

