
/* Copyright (c) 1996 by Gary William Flake. */


/* This is a simple example of how to use NODElib.  -- GWF */

#include <nodelib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

static char *help_string = "\
This program tests the J-prop features NODElib.  After training an \n\
auto-associator, a function of the Jacobian matrix is differentiated\n\
numerically and analytically, so that the results can be compared.\n\
\n\
The training algorithm should be one of:\n\
  gd, cgpr, cgfr, qndfp, or qnbfgs,\n\
while the activation function should be one of:\n\
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
	  nn->info.opt.stepsz, nn->info.opt.gradmag, nn->info.opt.error);
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
  printf("%d %d %d %d --> %.3f %.3f %.3f %.3f\n",
	 (int)nn->x[0], (int)nn->x[1], (int)nn->x[2], (int)nn->x[3],
	 nn->y[0], nn->y[1], nn->y[2], nn->y[3]);
  return(0);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* I am using a single C matrix to hold my my training data. */

static double rawdata[4][8] = {
  { 0.0, 0.0, 0.0, 1.0,   0.0, 0.0, 0.0, 1.0 },
  { 0.0, 0.0, 1.0, 0.0,   0.0, 0.0, 1.0, 0.0 },
  { 0.0, 1.0, 0.0, 0.0,   0.0, 1.0, 0.0, 0.0 },
  { 1.0, 0.0, 0.0, 0.0,   1.0, 0.0, 0.0, 0.0 },
};  

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

int main(int argc, char **argv)
{
  /* These variables are for command-line options.
   */
  double mag = 1.0, etol = 10e-3, detol = 10e-8, rate = 0.1;
  int seed = 0, minepochs = 10, maxepochs = 100;
  char *afunc = "tanh", *alg = "cgpr", *srch = "cubic";

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
    { "-alg",       OPT_STRING, &alg,       "training algorithm"           },
    { "-srch",      OPT_STRING, &srch,      "line search"                  },
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

  nn = nn_create("4 2 4");   /* 2-2-1 architecture. */
  nn_link(nn, "0 -l-> 1");   /* Inputs to hidden link. */
  nn_link(nn, "1 -l-> 2");   /* Hidden to output link. */

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
			dsm_c_matrix(&rawdata[0][0], 4, 4, 4));

  /* Tell the NN how to train itself.
   */
  nn->info.train_set = data;
  nn->info.opt.min_epochs = minepochs;
  nn->info.opt.max_epochs = maxepochs;
  nn->info.opt.error_tol = etol;
  nn->info.opt.delta_error_tol = detol;
  nn->info.opt.hook = training_hook;
  nn->info.opt.rate = rate;

  if(strcmp(srch, "hybrid") == 0)
    nn->info.opt.stepf = opt_lnsrch_hybrid;
  else if(strcmp(srch, "golden") == 0)
    nn->info.opt.stepf = opt_lnsrch_golden;
  else if(strcmp(srch, "cubic") == 0)
    nn->info.opt.stepf = opt_lnsrch_cubic;
  else if(strcmp(srch, "none") == 0)
    nn->info.opt.stepf = NULL;
  
  if(strcmp(alg, "cgpr") == 0)
    nn->info.opt.engine = opt_conjgrad_pr;
  else if(strcmp(alg, "cgfr") == 0)
    nn->info.opt.engine = opt_conjgrad_fr;
  else if(strcmp(alg, "qndfp") == 0)
    nn->info.opt.engine = opt_quasinewton_dfp;
  else if(strcmp(alg, "qnbfgs") == 0)
    nn->info.opt.engine = opt_quasinewton_bfgs;
  else if(strcmp(alg, "lm") == 0)
    nn->info.opt.engine = opt_levenberg_marquardt;
  else if(strcmp(alg, "bp") == 0) {
    nn->info.opt.engine = opt_gradient_descent;
    nn->info.opt.stepf = NULL;
    nn->info.subsample = 1;
    nn->info.opt.stepf = nn_lnsrch_search_then_converge;
    nn->info.opt.momentum = 0.9;
    nn->info.stc_eta_0 = 1;
    nn->info.stc_tau = 100;
  }

  /* Do the training.  This will print out the epoch number and
   * The error level until trianing halts via one of the stopping
   * criterion.
   */
  nn_train(nn);

  /* Print out each input training pattern and the respective
   * NN output.
   */
  printf("--------------------\n");
  nn_offline_test(nn, data, testing_hook);

#if 1
  { 
    const double dw = 0.000001;
    double jj1, jj2, *Rg, Rin[4], Rdout[4], dedy[4], err;
    int j, k, l, n = nn->numweights;
    Rg = allocate_array(1, sizeof(double), nn->numweights);
    nn->need_all_grads = 1;
    for(k = 0; k < 4; k++) {
      
      nn_forward(nn, &rawdata[k][0]);
      for(l = 0; l < nn->numout; l++)
	dedy[l] = nn->y[l] - rawdata[k][l];
      nn_backward(nn, dedy);
      for(l = 0; l < nn->numout; l++)
	/* Fixed */
	Rin[l] =  nn->dx[l] - dedy[l];

      nn_Rforward(nn, Rin, NULL);
      for(l = 0; l < nn->numout; l++)
	/* Fixed */
        Rdout[l] = nn->Ry[l] - nn->Rx[l];

      nn_Rbackward(nn, Rdout);
      nn_get_Rgrads(nn, Rg);

      for(j = 0; j < n; j++) {
	nn_forward(nn, &rawdata[k][0]);
	for(l = 0; l < nn->numout; l++)
	  dedy[l] = nn->y[l] - rawdata[k][l];
	nn_backward(nn, dedy);
	jj1 = 0;
	for(l = 0; l < nn->numout; l++)
	  jj1 += 0.5 * (dedy[l] - nn->dx[l]) * (dedy[l] - nn->dx[l]);

	*nn->weights[j] += dw;
	nn_forward(nn, &rawdata[k][0]);
	for(l = 0; l < nn->numout; l++)
	  dedy[l] = nn->y[l] - rawdata[k][l];
	nn_backward(nn, dedy);
	jj2 = 0;
	for(l = 0; l < nn->numout; l++)
	  jj2 += 0.5 * (dedy[l] - nn->dx[l]) * (dedy[l] - nn->dx[l]);
	err = fabs(Rg[j] - (jj2 - jj1) / dw) / fabs(Rg[j]);
	printf("(%d, %2d) ja = % .5e  jn = % .5e  error = % .2e  %s\n",
	       k, j, Rg[j], (jj2 - jj1) / dw,
	       err, (err > 10e-4) ? "BAD" : "GOOD");
	*nn->weights[j] -= dw;
      }
    }
  }
#endif

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

