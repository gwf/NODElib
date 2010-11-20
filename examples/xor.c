
/* Copyright (c) 1996 by Gary William Flake. */


/* This is a simple example of how to use NODElib.  -- GWF */

#include <nodelib.h>
#include <stdio.h>

static char *help_string = "\
Train a NN on XOR data.\n\
\n\
The activation function should be one of:\n\
  logistic, tanh, gauss, exp, linear, sin, or cos.\n\
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
  /* These variables are for command-line options.
   */
  double mag = 1.0, etol = 10e-3, detol = 10e-8;
  double rate = 0.1, moment = 0.9, subsamp = 0, decay = 0.9;
  int seed = 0, minepochs = 10, maxepochs = 100;
  char *afunc = "tanh";
  void *linealg = opt_lnsrch_golden, *optalg = opt_conjgrad_pr;

  OPTION_SET_MEMBER optsetm[] = {
    { "cgpr",   opt_conjgrad_pr },
    { "cgfr",   opt_conjgrad_fr },
    { "qndfp",  opt_quasinewton_dfp },
    { "qnbfgs", opt_quasinewton_bfgs },
    { "lm",     opt_levenberg_marquardt },
    { "gd",     opt_gradient_descent },
    { NULL,     NULL }
  };

  OPTION_SET_MEMBER linesetm[] = {
    { "golden", opt_lnsrch_golden },
    { "hybrid", opt_lnsrch_hybrid },
    { "cubic",  opt_lnsrch_cubic },
    { "stc",    nn_lnsrch_search_then_converge },
    { "none",   NULL },
    { NULL,     NULL }
  };

  OPTION_SET lineset = { &linealg, linesetm };
  OPTION_SET optset = { &optalg, optsetm };
    
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
    { "-rate",      OPT_DOUBLE, &rate,      "learning rate"                },
    { "-moment",    OPT_DOUBLE, &moment,    "momentum rate"                },
    { "-alg",       OPT_SET,    &optset,    "training algorithm"           },
    { "-subsamp",   OPT_DOUBLE, &subsamp,   "subsample value"  },
    { "-decay",     OPT_DOUBLE, &decay,     "stochastic decay"  },
    { "-srch",      OPT_SET,    &lineset,   "line search" },
    { NULL,         OPT_NULL,   NULL,       NULL                           }
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
  nn->info.opt.hook = training_hook;
  nn->info.opt.rate = rate;
  nn->info.opt.momentum = moment;
  nn->info.opt.decay = decay;
  nn->info.subsample = subsamp;
  if(subsamp != 0) {
    nn->info.subsample = subsamp;
    nn->info.opt.stochastic = 1;
  }
  nn->info.opt.stepf = linealg;
  nn->info.opt.engine = optalg;
  nn->info.stc_eta_0 = 1;
  nn->info.stc_tau = 100;


  /* Do the training.  This will print out the epoch number and
   * The error level until trianing halts via one of the stopping
   * criterion.
   */
  nn_train(nn);
  nn->info.subsample = 0;

  /* Print out each input training pattern and the respective
   * NN output.
   */
  printf("--------------------\n");
  nn_offline_test(nn, data, testing_hook);

  /* Free up everything.
   */
  nn_destroy(nn);
  dsm_destroy_matrix(dataset_destroy(data));
  nn_shutdown();

  /* Bye.
   */
  exit(0); 
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

