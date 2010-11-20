double   opt_lnsrch_step_epsilon = 10e-6;
double   opt_lnsrch_function_epsilon = 10e-6;
unsigned opt_lnsrch_max_step = 20;
unsigned opt_lnsrch_max_bracket_step = 20;


/* Copyright (c) 1995 by G. W. Flake. */

/* This file contains three line searches: cubic interpolation, GMK's
   hybrid, and golden section, in that order. */

/* Cubic interpolation: Original version by RLW.  Modified for
   inclusion into NODElib by GWF.  RLW's original block comments
   are retained below. -- GWF */

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* Author:      Raymond L. Watrous
   Department of Computer and Information Science
   University of Pennsylvania,
   Philadelphia,PA 19104
   December, 1988.

   Department of Computer Science
   University of Toronto,
   Toronto, Ontario M5S1A4
   September, 1990.

   Copyright (C) 1988, Trustees of the University of Pennsylvania.
   Copyright (C) 1990, Trustees of the University of Toronto.

   Permission is granted to any individual or institution to use, copy, or
   redistribute this software so long as it is not sold for profit, provided this
   copyright notice is retained. 

   NO WARRANTY

   Because GRADSIM is licensed free of charge, absolutely NO WARRANTY is
   provided. GRADSIM is supplied "as is" without warranty of any kind, either
   expressed or implied, including, but not limited to, the implied warranties
   of merchantability and fitness for a particular purpose. The entire risk as
   to the quality and performance of the program is with you. Should the
   program prove defective, you assume the cost of all necessary servicing,
   repair or correction.

   In no event will Raymond L. Watrous, the University of Pennsylvania,
   or the University of Toronto be liable for damages, including any
   lost profits, lost monies, or other special, incidental or
   consequential damages arising out of the use or inability to use
   this program. */

/* Line_search.c

   This is an auxiliary module for
   executing line search as a part of optimization.


   The necessary variables are the search direction,
   and an evaluation function.

   The source for this algorithm is R. Fletcher,
   Practical Methods for Optimization, John Wiley, 1980
   pp 25ff.


   In order to be fairly general, the following functions
   are required: the error function to be minimized, and
   gradient. Given is the initial function value, gradient
   vector and initial alpha. Returned is the minimizing
   value for alpha.


   9/23/87 - extensively modified based on further insight
   into Fletcher's descriptions, including errors.

   A more useful reference being Dennis & Schnabel, Numerical
   Methods for Unconstrained Optimization and Nonlinear Equations,
   Prentice-Hall, 1983.


   12/5/87 - modified for vcc, function name args

   12/15/87 - modified for VERBOSE mode switch

   1/20/88 - modified for new gradient computation; leaves new
   gradient value at new_grad

   4/4/89 - corrected convergence problem

   4/6/89 - added modifications from Fletcher 2nd edition */

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include <stdio.h>
#include <math.h>

#include "nodelib/xalloc.h"

#define  OPT_LINESRCH_OWNER
#include "nodelib/optimize.h"
#undef   OPT_LINESRCH_OWNER

#include "nodelib/ulog.h"
#include "nodelib/misc.h"

/* Line Search Constraints: RHO in (0,1/2), SIGMA in (RHO,1)
   0.1 ==> strict line search, 0.9 ==> weak line search,
   TAU in (0,SIGMA) */

#define RHO             (0.1)
#define SIGMA           (0.1)
#define TAU             (5e-2)
#define MAX_EXTRAPOL    (9)
#define EXTENSOR        (2.0)
#define EPSILON         (1e-18)
#define EPSILON10       (1e-17)
#define EXP_MIN         (0.0)


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* quadratic match given values (a1,f1) and (a2,f2) and
   slope f1_prime at a1. Stores new point at res;
   returns flag=1 if minimum found. */

static int quadratic(double a1, double f1, double f1_prime, double a2,
		     double f2, double *res)
{
  double delta, dx;

  delta = a2 - a1;
  dx = delta * f1_prime;

  if((f2 - f1) <= dx) {
    return (0);
  }
  *res = a1 + 0.5 * delta / (1 + (f1 - f2) / (dx));
  return (1);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* Cubic match given points (a1,f1) and (a2,f2) and
   slopes f1_prime at a1, f2_prime at a2. New point written at res.
   Returns ok=1 for found minimum, 0 for no minimum. */

static int cubic(double a1, double f1, double f1_prime, double a2,
		 double f2, double f2_prime, double *res)
{
  double delta, lambda, new, w, z, b;
  double rad;
  int ok;

  new = 0;
  lambda = (a2 - a1);
  delta = (f2 - f1) / lambda;
  b = f1_prime + f2_prime - 2 * delta;

  ok = 1;
  if(b == 0) {

    /* Quadratic form: if f1_prime == f2_prime, (straight line) -
     * Sign conditions for min/max dictate that for minimum,
     * f2_prime > f1_prime.
     */
    if(f2_prime <= f1_prime) {
      ok = 0;
    }
    else {
      new = (a1 * f2_prime - a2 * f1_prime) / (f2_prime - f1_prime);
    }
  }
  else {
    z = b - delta;
    rad = z * z - f1_prime * f2_prime;

    /* If rad is 0, inflection is saddle point - extend by default action.
     * If rad is negative, imaginary minima - also extend by default.
     */
    if(rad <= 0) {
      ok = 0;
    }
    /* It turns out that the sign conditions for the inflection points
     * guarantee that the negative choice for w always yields the
     * minimum rather than the maximum. Why is this?
     */
    else {
      w = sqrt(rad);
      new = a1 + lambda * (1.00 - (f2_prime - w + z) / (3 * b));
    }
  }
  *res = new;
  return (ok);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* Extrapolation given points (a1,f1) and (a2,f2) and
   slopes f1_prime at a1, f2_prime at a2, and amax. */

static int extrapolate(double a1, double f1, double f1_prime, double a2,
		       double f2, double f2_prime, double amax,
		       double *res, int bracket)
{
  double lambda, new, range;
  double low, high;
  int success;

  /* Check extrapolation preconditions
   */
  range = (amax - a2) * TAU;
  if(range <= EPSILON) {
    return (0);
  }
  else {
    lambda = (a2 - a1);
    if(lambda <= EPSILON) {
      new = a2 + range;
      *res = new;
      return (1);
    }
  }
  /* Get extrapolation value
   */

  success = cubic(a1, f1, f1_prime, a2, f2, f2_prime, &new);
  if(new < a2)
    success = 0;

  /* How the extrapolation failure is treated turns out to have an
   * effect on average convergence counts for Rosenbrock's function.
   * The more conservative a2+1.3 *lambda works pretty well.
   */
  if(success == 0) {
    if(bracket)
      new = (a2 + amax) / 2;
    else
      new = a2 + EXTENSOR * lambda;
  }

  /* check constraints on alpha new
   */
  if(bracket) {
    low = a2 + range;
    high = amax - range;
  }
  else {
    high = a2 + lambda * MAX_EXTRAPOL;
    if(amax < high)
      high = amax;
    low = a2 + TAU * lambda;
    if(high < low)
      low = high;
  }
  if(new < low) {
    new = low;
    /*  new = a2 + EXTENSOR * lambda; */
  }
  else if(new > high) {
    new = high;
  }
  *res = new;
  return (1);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* Interpolation:
   inputs -
   (a1,f1) and slope f1_prime at a1.
   (a2,f2) and slope f2_prime at a2 if flag = 1 */

static int interpolate(double a1, double f1, double f1_prime, double a2,
		       double f2, double f2_prime, int f2_flag, double *res)
{
  double delta, new;
  double low, high, range;
  int success;

  /* Check Interpolation pre-conditions
   */
  delta = (a2 - a1);

  if(-delta * f1_prime <= EPSILON) {
    return (0);
  }

  /* Check limits on interval
   */
  range = TAU * delta;

  if(range <= EPSILON) {
    return (0);
  }

  /* Now, interpolate point by quadratic or cubic methods
   * based on f2_flag
   */

  if(f2_flag == 1) {
    success = cubic(a1, f1, f1_prime, a2, f2, f2_prime, &new);
  }
  else {
    success = quadratic(a1, f1, f1_prime, a2, f2, &new);
  }

  /* Failed interpolation - go to center of interval
   */
  if(success == 0) {
    new = a1 + 0.5 * delta;
  }

  /* check interpolate post-conditions
   * new value is within (a1,a2) and not within range of ends
   */
  low = a1 + range;
  high = a2 - range;

  if(new < low) {
    new = low;
  }
  else if(new > high) {
    new = high;
  }
  *res = new;
  return (1);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static double dot_product_p(double **x, double *y, unsigned sz)
{
  unsigned i;
  double d = 0;

  for(i = 0; i < sz; i++)
    d += *x[i] * y[i];
  return(d);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if 0
static double dot_product(double *x, double *y, unsigned sz)
{
  unsigned i;
  double d = 0;

  for(i = 0; i < sz; i++)
    d += x[i] * y[i];
  return(d);
}
#endif 

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static void extend(double **z, double *x, double *y, double a, unsigned sz)
{
  unsigned i;

  for(i = 0; i < sz; i++)
    *z[i] = x[i] + a * y[i];
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* Given opt, return alpha such that f(w + alpha * aux) is minimized. */

double opt_lnsrch_cubic(OPTIMIZER *opt, double *d, double sz)
{
  double new_f, alpha;		/* new function evaluation */
  double a1, a2;		/* limits of search - initially 0 and inf */
  double f_prime;		/* slope scalar at x */
  double delta, alpha_hat;
  double f0, f1, f1_prime, comp, reference, ref_f;
  double *w0;
  int done, f2_ok, succeed, bracket;
  unsigned i;
  OPT_RESULT result;

  f_prime = f1_prime = 0;

  w0 = allocate_array(1, sizeof(double), opt->size);
  for(i = 0; i < opt->size; i++)
    w0[i] = *opt->weights[i];
  f0 = opt->error;

  if((f1_prime = dot_product_p(opt->grads, d, opt->size)) == 0) {
    opt->stepf_result = OPT_GRAD_DIR_ORTHOGONAL;
    return (0);
  }
  reference = f1_prime;

  a1 = 0;
  a2 = (EXP_MIN - opt->error) / (RHO * f1_prime);

  f1 = opt->error;
  ref_f = f1;

  /* Estimate initial alpha
   */
  delta = opt->error - EXP_MIN;
  if(delta < EPSILON10)
    delta = EPSILON10;
  alpha = -2 * delta / f1_prime;
  if(alpha > a2)
    alpha = a2;
  if(sz > 0) alpha = sz;
  /* alpha should be 1.00 whenever possible, because the Newton
   * step will result in faster convergence, near the solution,
   * for BFGS methods
   */

  /* Loop conditions
   * 0 - still looking
   * 1 - successful
   * 2 - error, search failed to meet constraints
   */
  bracket = 0;
  done = 0;
  result = OPT_SUCCESS;
  while(!done) {
    /*  Step 1 : evaluate function at x + alpha * s
     */
    extend(opt->weights, w0, d, alpha, opt->size);
    opt_eval_func(opt, NULL);
    new_f = opt->error;
    f2_ok = 0;

    /* step2: check actual delta_f against expected 
     * (First Goldstein Criterion)
     */
    delta = new_f - ref_f;
    comp = RHO * alpha * reference;
    if((delta <= comp) && (new_f < f1)) {
      opt_eval_grad(opt, NULL);
      f_prime = dot_product_p(opt->grads, d, opt->size);
      f2_ok = 1;

      /* Check termination conditions (Wolfe Test)
       */
      if(fabs(f_prime) <= SIGMA * (-reference))
	done = 1;
      else if(f_prime > 0) {
	succeed = interpolate(a1, f1, f1_prime, alpha, new_f,
			      f_prime, f2_ok, &alpha_hat);
	if(succeed == 1) {
	  if(alpha == alpha_hat) done = 1;
	  a2 = alpha;
	  alpha = alpha_hat;
	  bracket = 1;
	}
	else {
	  done = 2;
	  result = OPT_INTERPOLATE_FAILED;
	}
      }
      else {
	/* Extrapolate to point within interval (alpha,a2)
	 */
	succeed = extrapolate(a1, f1, f1_prime, alpha,
			      new_f, f_prime, a2, &alpha_hat, bracket);
	if(succeed == 1) {
	  a1 = alpha;
	  f1_prime = f_prime;
	  f1 = new_f;
	  alpha = alpha_hat;
	}
	else {
	  done = 2;
	  result = OPT_EXTRAPOLATE_FAILED;
	}
      }
    }
    else {
      /* Interpolate to new value within interval (a1,alpha)
       */
      succeed = interpolate(a1, f1, f1_prime, alpha, new_f,
			    f_prime, f2_ok, &alpha_hat);
      if(succeed == 1) {
	a2 = alpha;
	alpha = alpha_hat;
	bracket = 1;
      }
      else {
	done = 2;
	result = OPT_INTERPOLATE_FAILED;
      }
    }
  }
  opt->stepf_result = result;
  if(done != 1) {
    extend(opt->weights, w0, d, alpha, opt->size);
    opt_eval_func(opt, NULL);
    if(f0 < opt->error)
      extend(opt->weights, w0, d, 0, opt->size);
    opt_eval_func(opt, NULL);
  }
  deallocate_array(w0);
  return(alpha);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#define GOLD (1.618)
#define R (GOLD-1)
#define C (1-R)

double opt_lnsrch_hybrid(OPTIMIZER *opt, double *d, double sz)
{
  double poly[3][2], alpha, lasterr;
  double *w0;
  int pi, pc;
  unsigned i;			      

  /* Save original weights. */
  w0 = allocate_array(1, sizeof(double), opt->size);
  for(i = 0; i < opt->size; i++)
    w0[i] = *opt->weights[i];

  /* Use the previous step size, if supplied, else make an educated guess. */
  if(sz > 0)
    alpha = sz;
  else {
    if((alpha = dot_product_p(opt->grads, d, opt->size)) == 0) {
      opt->stepf_result = OPT_GRAD_DIR_ORTHOGONAL;
      return (0);
    }
    alpha = -2 * opt->error / alpha;
  }

  /* Start the bracketing with the two guesses using the current alpha
   * and an alpha of zero.  If the current alpha is better than zero (i.e., 
   * The function is running downhill), then keep increasing alpha in size
   * until the function goes uphill (thus, completing the bracketing).
   */
  poly[0][0] = 0.0; poly[0][1] = opt->error;
  pc = 1; pi = 1;
  do {
    lasterr = opt->error;
    extend(opt->weights, w0, d, alpha, opt->size);
    opt_eval_func(opt, NULL);
    poly[pi][0] = alpha; poly[pi][1] = opt->error;
    pc++; pi = (pi + 1) % 3;
    alpha *= 2;
  }
  while(opt->error < lasterr);

  /* It turned out that f(w) < f(w + alpha * d), so we need to look for
   * smaller alpha such that f(w) > f(w + alpha * d).
   */
  if(pc == 2) {
    do {
      poly[2][0] = poly[1][0]; poly[2][1] = poly[1][1];
      poly[1][0] = alpha = poly[2][0] / 2;
      extend(opt->weights, w0, d, alpha, opt->size);
      opt_eval_func(opt, NULL);
      poly[1][1] = opt->error;
    }
    while(poly[1][1] > poly[0][1]);
    pc = 3; pi = 0;
  }

  /* We now have the minimum bracketed in such a way that the middle point
   * obeys the golden ratio for the endpoints.  Now, fit the last three
   * points to a parabola, and look in the middle of the dip.
   */
  {
    double x1 = poly[0][0], x11 = x1 * x1, y1 = poly[0][1];
    double x2 = poly[1][0], y2 = poly[1][1];
    double x3 = poly[2][0], y3 = poly[2][1];
    double a, b, c;
 
    if(x1 == x2 || x2 == x3 || x1 == x3) 
      alpha = poly[(pi + 1) % 3][0];
    else {
      a = (y1-y2)/((x1-x2)*(x2-x3)) - (y1-y3)/((x1-x3)*(x2-x3));
      b = (y1-y2)/(x1-x2) - a*(x1+x2);
      c = y1 - a*x11 - b*x1;
      if(a != 0)
	alpha = -b / (2 * a);
      else
	alpha = poly[(pi + 1) % 3][0];
      extend(opt->weights, w0, d, alpha, opt->size);
      opt_eval_func(opt, NULL);
      if(poly[(pi + 1) % 3][1] < opt->error) {
	alpha = poly[(pi + 1) % 3][0];
	extend(opt->weights, w0, d, alpha, opt->size);
	opt_eval_func(opt, NULL);
      }
    }
  }

  deallocate_array(w0);
  return(alpha);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

double opt_lnsrch_golden(OPTIMIZER *opt, double *d, double sz)
{
  double poly[3][2], alpha, lasterr;
  double *w0;
  int pi, pc;
  unsigned i;

  /* Save original weights. */
  w0 = allocate_array(1, sizeof(double), opt->size);
  for(i = 0; i < opt->size; i++)
    w0[i] = *opt->weights[i];

  /* Use the previous step size, if supplied, Else, make an educated guess. */
  if(sz > 0)
    alpha = sz;
  else
    alpha = -2 * opt->error / dot_product_p(opt->grads, d, opt->size);

#if 0
  alpha = dot_product_p(opt->grads, d, opt->size) /
    dot_product(d, d, opt->size);
#endif

  /* Start the bracketing with the two guesses using the current alpha
   * and an alpha of zero.  If the current alpha is better than zero (i.e., 
   * The function is running downhill), then keep increasing alpha in size
   * until the function goes uphill (thus, completing the bracketing).
   */
  poly[0][0] = 0.0; poly[0][1] = opt->error;
  pc = 1; pi = 1;
  do {
    lasterr = opt->error;
    extend(opt->weights, w0, d, alpha, opt->size);
    opt_eval_func(opt, NULL);
    poly[pi][0] = alpha; poly[pi][1] = opt->error;
    pc++; pi = (pi + 1) % 3;
    alpha = poly[(pi - 2 + 3) % 3][0] +
      (alpha - poly[(pi - 2 + 3) % 3][0]) / C;
  }
  while(opt->error < lasterr);

  /* It turned out that f(w) < f(w + alpha * d), so we need to look for
   * smaller alpha such that f(w) > f(w + alpha * d).
   */
  if(pc == 2) {
    do {
      poly[2][0] = poly[1][0]; poly[2][1] = poly[1][1];
      poly[1][0] = alpha = C * poly[2][0];
      extend(opt->weights, w0, d, alpha, opt->size);
      opt_eval_func(opt, NULL);
      poly[1][1] = opt->error;
    }
    while(poly[1][1] > poly[0][1]);
    pc = 3; pi = 0;
  }
    

  /* We now have the minimum bracketed in such a way that the middle point
   * obeys the golden ratio for the endpoints.  Now, search in this area.
   */
  {
    double ax, bx, cx, af, bf, cf, x, f;
    int count = 0;

    ax = poly[(pi) % 3][0];     af = poly[(pi) % 3][1];
    bx = poly[(pi + 1) % 3][0]; bf = poly[(pi + 1) % 3][1];
    cx = poly[(pi + 2) % 3][0]; cf = poly[(pi + 2) % 3][1];
    
    while(fabs(ax - cx) > opt_lnsrch_step_epsilon 
	  && fabs(af - cf) > opt_lnsrch_function_epsilon 
	  && count < opt_lnsrch_max_step) {
      if(bx - ax < cx - bx) {
	alpha = x = bx * R + cx * C;
	extend(opt->weights, w0, d, x, opt->size);
	opt_eval_func(opt, NULL);
	f = opt->error;
	if(f < bf) {
	  ax = bx; af = bf;
	  bx = x; bf = f;
	}
	else {
	  cx = x; cf = f;
	}
      }
      else {
	alpha = x = ax * C + bx * R;
	extend(opt->weights, w0, d, x, opt->size);
	opt_eval_func(opt, NULL);
	f = opt->error;
	if(f < bf) {
	  cx = bx; cf = bf;
	  bx = x; bf = f;
	}
	else {
	  ax = x; af = f;
	}
      }
      count++;
    }
    if(alpha != bx) {
      extend(opt->weights, w0, d, bx, opt->size);
      opt_eval_func(opt, NULL);
    }
  }

  deallocate_array(w0);
  return(alpha);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
