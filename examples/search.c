
/* Copyright (c) 1996 by Gary William Flake. */


/* This is a simple example of how to use NODElib.  -- GWF */

#include <nodelib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

char *help_string = "\
This test program will perform a line search to find a minimum\n\
of the function (sin(x) * sin(x)).  The initial point is randomly\n\
picked to be from the segment (-|range|, |range|).  Search type\n\
should be one of cubic, golden, or hybrid.\n\
\n";

typedef struct FUNC {
  double x, y, g, t;
  double *xp, *gp;
} FUNC;

#define SQR(x) ((x)*(x))

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static double func_wrapper(void *obj)
{
  FUNC *func = obj;

  func->y = SQR(sin(func->x));
  return(func->y);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static double grad_wrapper(void *obj)
{
  FUNC *func = obj;

  func->y = SQR(sin(func->x));
  func->g = 2 * sin(func->x) * cos(func->x);
  return(func->y);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

int main(int argc, char **argv)
{
  /* These variables are for command-line options.
   */
  int seed = 0;
  double range = 10, alpha = 0, x0;
  char *type = "cubic";
  OPTIMIZER opt = OPTIMIZER_DEFAULT;
  FUNC func;

  /* The OPTION array is used to easily parse command-line options.
   */
  OPTION opts[] = {
    { "-range", OPT_DOUBLE, &range, "range for random start" },
    { "-seed",  OPT_INT,    &seed,  "random number seed"     },
    { "-type",  OPT_STRING, &type,  "search type"            },
    { NULL,     OPT_NULL,   NULL,    NULL                    }
  };

  /* Get the command-line options.
   */
  get_options(argc, argv, opts, help_string, NULL, 0);

  /* Set the random seed.
   */
  srandom(seed);
  range = fabs(range);
  x0 = func.x = random_range(-range, range);
  func.xp = &func.x;
  func.gp = &func.g;

  opt.size = 1;
  opt.weights = &func.xp;
  opt.grads = &func.gp;
  opt.funcf = func_wrapper;
  opt.gradf = grad_wrapper;
  opt.obj = &func;

  opt_eval_grad(&opt, NULL);
  func.t = -func.g;
  
  printf("initial x\t= % f\n", func.x);
  printf("initial y\t= % f\n", func.y);

  if(strcmp(type, "cubic") == 0)
    alpha = opt_lnsrch_cubic(&opt, &func.t, 0);
  else if(strcmp(type, "golden") == 0)
    alpha = opt_lnsrch_golden(&opt, &func.t, 0);   
  else if(strcmp(type, "hybrid") == 0)
    alpha = opt_lnsrch_hybrid(&opt, &func.t, 0);

  printf("\nfinal x \t= % f\n", func.x + alpha * func.t);
  printf("final y \t= % f\n", func.y);
  printf("step size\t= % f\n", alpha);
  printf("\nfunc evals\t= % d\n", opt.fcalls);
  printf("grad evals\t= % d\n", opt.gcalls);

  exit(0); 
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

