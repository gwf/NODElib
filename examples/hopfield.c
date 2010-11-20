
/* Copyright (c) 1996 by Gary William Flake. */

/* This is a simple example of how to use NODElib.  -- GWF */

#include <nodelib.h>
#include <stdio.h>
#include <math.h>

static char *help_string = "\
A Hopfield network applied to a task assigment problem.  See the\n\
documentation in the source code for more details.\n\
";

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* This example demonstrates how to build a Hopfield network with NODElib.
   I don't recommend this as a typical use of NODElib, but it is an
   interesting exercise nonetheless.

   The Hopfield network that I will implement is defined by the
   equations:

      dU[i]/dt = SUM_j ( T[i,j] * V[j] ) + I[i] - U[i] / tau
   
      V[i] = g(U[i]), and

      g(x) = 1 / (1 + exp(-x * gain)),

   where U is an internal state, V is a neuron's activation value, T
   is the connection matrix, I is an external input, g() is the
   activation function, and tau and gain are simulation parameters.

   The discrete time version, with 't' now indicating time, can then
   be described by:

      U[i](t + 1) = U[i](t) + dt * ( SUM_j ( T[i,j] * V[j](t) ) +
                                     I[i] - U[i](t) / tau )

   I will simulate this in NODElib by using a two node layer network.
   The first layer will consist of linear nodes that correspond to the
   U values, and the second layer consists of sigmoidal nodes that
   correspond to the V terms.

   To simulate the dynamics, we need three types of connections and
   some assumptions on the input:

   1) The external input, I, is feed into the network as input (i.e.,
      through the nn_forward() call), but is scaled by dt.  (N.B.: we
      could also make this the "bias" term from the linear connection
      described in 2).)

   2) linear connections from the second layer to the first represent
      the T terms.  Since these flow from the V layer to the U layers,
      this is a perfect match.

   3) There is a self-recurrent scalar connection in the first layer.
      All of these weights will be equal to -dt/tau.  This gives us
      the -dt*U[i]/tau term.

   4) Finally, copy connections go from the first to the second layer,
      which passes U through the nonlinear activation function to
      compute V = g(U). */

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* This example will solve the task assigment problem which is a
   combinatorial optimization problem.  The data for the problem
   consists of an N x N grid.  The (i,j) entry corresponds to worker
   (i) efficiency at performing task(j).  Our goal is to find the
   permuation matrix (i.e., a 1-to-1 assigment of workers to tasks
   such that all tasks and all workers are paired exactly once) that
   maximizes the global performance. */

static double task_assigment_scores[] = {
  10.0,   5.0,    4.0,    6.0,    5.0,    1.0,
  6.0,    4.0,    9.0,    7.0,    3.0,    2.0,
  1.0,    8.0,    3.0,    6.0,    4.0,    6.0,
  5.0,    3.0,    7.0,    2.0,    1.0,    4.0,
  3.0,    2.0,    5.0,    6.0,    8.0,    7.0,
  7.0,    6.0,    4.0,    1.0,    3.0,    2.0
};

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

NN *create_hopfield_net(double gain, double tau, double dt)
{
  NN *nn;
  int sz, n, i, j, k, l, ii, jj;
  char str[80];

  /* How many neurons?  What is the N x N size? */
  sz = sizeof(task_assigment_scores) / sizeof(double);
  n = sqrt(sz);

  /* Make a two-layer net.  The first layer will be the U terms, while
   * the second layer hold the V terms.
   */
  sprintf(str, "%d %d", sz, sz);
  nn = nn_create(str);
  nn_set_actfunc(nn, 0, 0, "none");
  nn_set_actfunc(nn, 1, 0, "sigmoid");

  /* This just makes the logistic activation function behave as
   * specified in the main documentation above.
   */
#if 0
  nn->layers[1].slabs[0].aux[0] = gain;
#endif

  /* The first connection is for the T terms.  The second is for the
   * -dt * U / tau terms, and the third is just to pass V = g(U).
   */
  nn_link(nn, "1 -l-> 0");
  nn_link(nn, "0 -s-> 0");
  nn_link(nn, "0 -c-> 1");

  /* This is just a little hack to set of the T connections.  By Page
   * and Tagliarini's K-out-of-N rule, for any pair of neurons that
   * reside in the same column or row we want T[i,j] = -2.  Otherwise,
   * it whould be 0.
   *
   * The i and k indices move over rows, the j and l indices move over
   * columns, and ii and jj index the neuron as they appear in the NN.
   */

  /* For every neuron in the N x N grid...  */
  for(i = 0, ii = 0; i < n; i++)
    for(j = 0; j < n; j++, ii++)
     
      /* For every other neuron in the N x N grid... */
      for(k = 0, jj = 0; k < n; k++)
	for(l = 0; l < n; l++, jj++)
	 
	  /* Are they in the same column or Row?  If so, then make the
	   * weight a -2, but multiply it by dt as well.  Otherwise,
	   * the weight should be zero.
	   */
	  if((i == k && j != l) || (i != k && j == l))
	    nn->links[0]->u[ii][jj] = -2.0 * dt;
	  else
	    nn->links[0]->u[ii][jj] = 0.0;

  /* We don't need these, so zero them out. */
  for(i = 0; i < sz; i++)
    nn->links[0]->a[i] = 0;

  /* These next connections are for the U - dt * U / tau terms. */
  for(i = 0; i < sz; i++)
    nn->links[1]->a[i] = 1 - dt / tau;

  /* Finally, give our V terms a random initial state. */
  for(i = 0; i < sz; i++)
    nn->layers[1].y[i] = random_range(0.3, 0.7);

  return(nn);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

double *create_external_input(double scale, double dt)
{
  double min = 10e10, max = -10e010, ave = 0.0, *data;
  int i, sz;

  /* How many neurons? */
  sz = sizeof(task_assigment_scores) / sizeof(double);

  /* Get some memory. */
  data = allocate_array(1, sizeof(double), sz);

  for(i = 0; i < sz; i++) {
    if(task_assigment_scores[i] < min) min = task_assigment_scores[i];
    if(task_assigment_scores[i] > max) max = task_assigment_scores[i];
    ave += task_assigment_scores[i];
  }
  ave /= sz;

  for(i = 0; i < sz; i++)
    data[i] = dt * (scale * (task_assigment_scores[i] - ave) /
                            (max - min) + 2);

  return(data);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

int main(int argc, char **argv)
{
  double dt = 0.1, tau = 10.0, gain = 0.5, scale = 0.5;
  int steps = 400, seed = 0;

  OPTION opts[] = {
    { "-dt",     OPT_DOUBLE,  &dt,     "time step increment"            },
    { "-tau",    OPT_DOUBLE,  &tau,    "decay term"                     },
    { "-gain",   OPT_DOUBLE,  &gain,   "sigmoidal gain"                 },
    { "-scale",  OPT_DOUBLE,  &scale,  "scaling for inputs"             },
    { "-seed",   OPT_INT,     &seed,   "random seed for initial state"  },
    { "-steps",  OPT_INT,     &steps,  "number of time steps"           },
    { NULL,      OPT_NULL,    NULL,    NULL                             }
  };

  NN *nn;
  double *data;
  int i, j, k, ii, sz, n, sum;

  /* Get the command-line options.  */
  get_options(argc, argv, opts, help_string, NULL, 0);

  /* How many neurons?  What is the N x N size? */
  sz = sizeof(task_assigment_scores) / sizeof(double);
  n = sqrt(sz);

  /* Set the random seed. */
  srandom(seed);

  /* Create the Hopfield network. */
  nn = create_hopfield_net(gain, tau, dt);

  /* Create the input data.  Note that if we changed the data for this
   * task assigment problem, then this would be the only thing that
   * would need changing for this example.
   */
  data = create_external_input(scale, dt);
  
  for(ii = 0; ii < steps; ii++) {

    /* Print out the network's activation values. */
    for(i = 0, k = 0; i < n; i++) {
      for(j = 0; j < n; j++, k++)
	printf("% .3f\t", nn->y[k]);
      printf("\n");
    }
    printf("----------------------------------------------\n");

    /* Simulate a single time step. */
    nn_forward(nn, data);   
  }

  /* Note that I am beeing lazy and not checking that the solution forms
   * a permutation matrix.
   */

  sum = 0;
  for(i = 0; i < sz; i++)
    if(nn->y[i] > 0.5) sum += task_assigment_scores[i];
  printf("\nFinal score = %d\n\n", sum);

  /* Free up everything. */ 
  deallocate_array(data);
  nn_destroy(nn);
  nn_shutdown();

  exit(0);  
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
