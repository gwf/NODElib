
/* Copyright (c) 1996 by Gary William Flake. */


/* This is example just checks for memory leaks in the library.
   Nothing useful is donw in this program unless all of NODElib
   is compiled with a -DXALLOC_DEBUG switch.  Hence, I keep this
   code in the etc directory under normal circumstances.  -- GWF

 */

#include <nodelib.h>
#include <stdio.h>
#include <unistd.h>

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
  /* These variables are for command-line options.
   */
  double mag = 0.1, etol = 10e-3, detol = 10e-8;
  int seed = 0, minepochs = 10, maxepochs = 100;
  char *afunc = "tanh";

  /* The OPTION array is used to easily parse command-line options.
   */
  OPTION opts[] = {
    { "-seed",      OPT_INT,    &seed,      "random number seed"           },
    { "-minepochs", OPT_INT,    &minepochs, "minimum # of training steps"  },
    { "-maxepochs", OPT_INT,    &maxepochs, "maximum # of training steps"  },
    { "-afunc",     OPT_STRING, &afunc,     "act. function for hidden node"},
    { "-mag",       OPT_DOUBLE, &mag,       "max size of initial weights"  },
    { "-etol",      OPT_DOUBLE, &etol,      "error tolerance"              },
    { "-detol",     OPT_DOUBLE, &detol,     "delta error tolerance"        },
    { NULL,         OPT_NULL,   NULL,       NULL                           }
  };

  /* The DATASET and the NN that we will use.
   */
  DATASET *data;
  NN *nn;

  /* Set it so that xalloc_report() will print to the screen.
   */
  ulog_threshold = ULOG_DEBUG;
  
  /* Get the command-line options.
   */
  get_options(argc, argv, opts, "Train a NN on XOR data.\n");

  /* Set the random seed.
   */
  srandom(seed);

  /* Create the neural network.  This one has two inputs, one hidden node,
   * and a single output.  The input are connected to the hidden node 
   * and the outputs, while the hidden node is just connected to the
   * outputs.
   */
  nn = nn_create("2 1 1");   /* 2-1-1 architecture. */
  nn_link(nn, "0 -l-> 1");   /* Inputs to hidden link. */
  nn_link(nn, "1 -l-> 2");   /* Hidden to output link. */
  nn_link(nn, "0 -l-> 2");   /* Input to output short-circuit link. */  

  /* Set the Activation functions of the hidden and output layers and
   * initialize the weights to uniform random values between -/+mag.
   */
  nn_set_actfunc(nn, 1, 0, afunc);
  nn_set_actfunc(nn, 2, 0, "logistic");
  nn_init(nn, mag);
 
  /* Convert the C matrix into a DATASET.  There are two inputs, one
   * output, and four patterns total.
   */
  data = dataset_create(&dsm_matrix_method,
			dsm_c_matrix(&xor_data[0][0], 2, 1, 4));

  /* Tell the NN how to train itself.
   */
  nn->info.train_set = data;
  nn->info.opt.min_epochs = minepochs;
  nn->info.opt.max_epochs = maxepochs;
  nn->info.opt.error_tol = etol;
  nn->info.opt.delta_error_tol = detol;

  nn_train(nn);
  nn_offline_test(nn, data, NULL);

  nn_write(nn, "xor.net");
  nn_destroy(nn);
  nn = nn_read("xor.net");
  nn_destroy(nn);
  unlink("xor.net");

  dsm_destroy_matrix(dataset_destroy(data));
  nn_shutdown();

  xalloc_report();

  /* Bye.
   */
  exit(0); 
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

