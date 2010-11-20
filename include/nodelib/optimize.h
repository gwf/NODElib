
/* Copyright (c) 1996 by G. W. Flake.
 *
 * NAME
 *   optimize.h - numerical optimization routines
 * SYNOPSIS
 *   This module provides a generic and uniform interface to
 *   optimization routines that can be used on a wide variety of
 *   problems.  This package currently includes multi-dimensional
 *   search algorithms (steepest descent, conjugate gradient,
 *   and quasi-Newton) and line search routines (cubic interpolation,
 *   golden section, and a hybrid of the first two).
 * DESCRIPTION
 *   With the suplied optimization routines, you can perform numerical
 *   optimization that's stops under a wide variety of user specified
 *   conditions, performs regular auxiliary functions through a hook
 *   mechanism, and computes a few useful statistics.
 *
 *   The source file graddesc.c gives a simple example of how an
 *   optimization routine should be written for this package.  You can
 *   use that file as a template for creating additional optimization
 *   routines.
 * AUTHOR
 *   Gary William Flake (\url{\bf{gary.flake@usa.net}}{mailto:gary.flake@usa.net}).
 */


#ifndef __OPTIMIZE_H__
#define __OPTIMIZE_H__

#include "nodelib/etc/version.h"
#include "nodelib/etc/options.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* This is the structure that must be filled in by the user so as to 
   specify all of the details as to how the optimization routines
   should work.  Each optimization routines takes a pointer to this
   structure as a single argument. */

typedef struct OPTIMIZER {
  /*
   * These fields *must* be specified for an optimization
   * routine.  They essentially specify the function that
   * is to be optimized what method for optimization to use.
   *
   * The fields below contain: the size of the weight space,
   * the function to minimize, a function to compute the
   * gradient, pointers to get and set the weights and
   * gradients without evaluating the function or the
   * gradients, and the optimization engine to use.
   */
  unsigned size;
  double (*funcf)(void *obj);
  double (*gradf)(void *obj);
  double **weights, **grads;
  void (*engine)(struct OPTIMIZER *opt, int state);
  /*
   * Optional fields.  These allow you perform regular
   * events and constrained optimization.
   *
   * An object to pass to some functions, a function to
   * call on each epoch, a hook that is called when
   * (epoch % hook_freq == 0), a function to check for
   * special halting conditions, and an alternate line
   * search algorithm.
   */
  void *obj;
  int (*hook)(void *obj);
  unsigned hook_freq;
  int (*haltf)(void *obj);
  double (*stepf)(struct OPTIMIZER *opt, double *dir,
		  double stepsz);
  /*
   * These fields specify stopping criterion:
   * the minimum number of iterations, the maximum number
   * of iterations, stop if error falls below error_tol,
   * stop if delta error falls below delta_error_tol.
   */
  unsigned min_epochs, max_epochs;
  double error_tol, delta_error_tol;
  /*
   * The fields below are for stochastic optimization
   * routines.  If the stochastic field is non-zero, then
   * the decayed halting criteria will also be checked by
   * the optimization procedures.  The value of decay
   * specifies how rapidly the decayed statistics age.  As
   * an example, given an instantaneous error, the new
   * value of the decayed error is set to (old * decay +
   * (1 - decay) * error).  The seed field is for internal
   * use only as it allows the routines to compute
   * consistent function evaluations from one step to the
   * next.
   */
  double decayed_error_tol, decayed_delta_error_tol, decay;
  unsigned stochastic, seed;
  /*
   * Information for online techniques: fixed step size
   * and the momentum.
   */
  double rate, momentum;
  /*
   * Weight decay term which augments the error
   * function with a sum of the squared weights times
   * wdecay.  The gradient function is also modified
   * accordingly.
   */
  double wdecay;
  /*
   * These are statistics filled in by the optimization
   * routines: the number of calls made to funcf(), the
   * number of calls made to gradf(), the current epoch,
   * error and delta erors (dacayed and not decayed),
   * the magnitude of the gradient vector, and the last
   * step size used by a line search routine.
   */
  unsigned fcalls, gcalls, epoch;
  double error, delta_error;
  double decayed_error, decayed_delta_error;
  double gradmag, stepsz;
  /*
   * Since the optimization engine and the line search
   * routines can halt for a variety of reasons, the
   * last results are stored in the feilds below.
   */
  unsigned stepf_result, engine_result;
  /*
   * The next five auxiliary fields are for use by
   * user-defined engines and line search routines
   * that may need additional parameters that don't
   * quite fit anywhere in this structure.
   */
   double aux1, aux2, aux3, aux4, aux5;
  /*
   * This is normally set to the pointer of the structure
   * that "owns" this optimizer.
   */
   void *owner;
  /*
   * This is an internal data structure that the optimization
   * routines use to hold the progress of the optimization.
   */
  void *internal;
} OPTIMIZER;

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* Evaluates the objective function, \em{opt->funcf(opt->obj)}, and
   sets \em{opt->val}, \em{opt->goodfval},  and \em{opt->fcalls}
   appropriately.  The last evaluated value of the objective function
   is returned.  If \em{weights} is non-NULL, then its values are
   copied on top of \em{opt->weights} prior to doing anything. */

double opt_eval_func(OPTIMIZER *opt, double *weights);

/* Evaluates the gradient function, \em{opt->funcg(opt->obj)}, and
   sets \em{opt->val}, \em{opt->goodfval}, \em{opt->goodgval},  and
   \em{opt->gcalls} appropriately.  The last evaluated value of the
   objective function is returned.  If \em{weights} is non-NULL,
   then its values are copied on top of \em{opt->weights} prior
   to doing anything.*/

double opt_eval_grad(OPTIMIZER *opt, double *weights);


/* Optimizes \em{opt} by calling \em{opt->engine}.  All calculations
   for statistics and checking halting conditions is done here. */

int optimize(OPTIMIZER *opt);

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* Line search with cubic interpolation.  The point defined by 
   \em{opt->weights} is searched in the direction of \em{dir}
   with the initial guess for a step size contained in \em{stepsz}.
   If \em{stepsz} is zero, then the routine will make its own
   guess.  The weights are changed to \em{opt->weights + alpha * dir}
   where \em{alpha} is the optimimal step size (which is also
   the value that is returned by the function).  This routine assumes
   that the current gradient values are valid for the current
   weights. */

double opt_lnsrch_cubic(OPTIMIZER *opt, double *dir, double stepsz);

/* Line search via the golden section algorithm.  The point defined by
   \em{opt->weights} is searched in the direction of \em{dir} with the
   initial guess for a step size contained in \em{stepsz}.  If
   \em{stepsz} is zero, then the routine will make its own guess.  The
   weights are changed to \em{opt->weights + alpha * dir} where
   \em{alpha} is the optimimal step size (which is also the value that
   is returned by the function).  This routine assumes that the
   current gradient values are valid for the current weights. */

double opt_lnsrch_golden(OPTIMIZER *opt, double *dir, double stepsz);

/* Line search via a relatively inaccurate hyrbrid algorithm.  The
   point defined by \em{opt->weights} is searched in the direction of
   \em{dir} with the initial guess for a step size contained in
   \em{stepsz}.  If \em{stepsz} is zero, then the routine will make
   its own guess.  The weights are changed to
   \em{opt->weights + alpha * dir} where \em{alpha} is the optimimal
   step size (which is also the value that is returned by the function). 
   The algorithm performs a simple but thorough search to bracket the
   minimum then fits a parabola to the last three locations of the search
   space.  If the bottom of the parabola evalutes to a lower value, then
   it is accepted as the final value; otherwise, the bast value from the
   bracketing procedure is used.  This routine assumes that the
   current gradient values are valid for the current weights. */

double opt_lnsrch_hybrid(OPTIMIZER *opt, double *dir, double stepsz);


#ifdef OPT_LINESRCH_OWNER

int opt_lnsrch_max_steps = 10;
double opt_lnsrch_min_relative_change = 0.01;

#endif

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* Levenberg-Marquardt.  This routine is currently broken. */

void opt_levenberg_marquardt(OPTIMIZER *opt, int state);


/* Polak-Ribiere deterministic conjugate gradient. */

void opt_conjgrad_pr(OPTIMIZER *opt, int state);


/* Fletcher-Reeves deterministic conjugate gradient. */

void opt_conjgrad_fr(OPTIMIZER *opt, int state);


/* Davidson-Fletcher-Powell quasi-Newton. */

void opt_quasinewton_dfp(OPTIMIZER *opt, int state);


/* Broyden-Fletcher-Goldfarb-Shanno quasi-Newton. */

void opt_quasinewton_bfgs(OPTIMIZER *opt, int state);


/* Batched gradient descent with momentum. */

void opt_gradient_descent(OPTIMIZER *opt, int state);


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* h2man:skipbeg */

typedef enum OPT_RESULT {
  OPT_SUCCESS,
  OPT_NO_ENGINE,
  OPT_HOOK_RETURN,
  OPT_HALTF_RETURN,
  OPT_ERROR_TOL,
  OPT_DELTA_ERROR_TOL,
  OPT_DECAY_ERROR_TOL,
  OPT_DECAY_DELTA_ERROR_TOL,

  OPT_MAX_EPOCHS_REACHED,
  OPT_MIN_DELTA_STEP,
  OPT_MIN_DELTA_ERROR,
  OPT_MAX_LINE_STEPS,

  OPT_ZERO_GRAD,
  OPT_DIV_ZERO,
  OPT_BAD_DIRECTION,
  OPT_GRAD_DIR_ORTHOGONAL,
  OPT_INTERPOLATE_FAILED,
  OPT_EXTRAPOLATE_FAILED,
  OPT_BRACKET_FAILED,

} OPT_RESULT;

#ifdef OWNER
#undef OWNER

const OPTIMIZER OPTIMIZER_DEFAULT = {
  0, NULL, NULL, NULL, NULL, opt_conjgrad_pr,
  NULL, NULL, 1, NULL, opt_lnsrch_cubic,  
  0, 0, 0.0, 0.0,
  0.0, 0.0, 0.99, 0, 0,
  0.01, 0.9,
  0.0,
  0, 0, 0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
  0, 0,
  0.0, 0.0, 0.0, 0.0, 0.0,
  NULL, NULL
};

const char *const opt_result_strings[] =
{
  "success",
  "no engine assigned to OPTIMIZER",
  "user hook returned with nonzero value",
  "haltf hook returned with nonzero value",
  "error tolerance exceeded",
  "delta error tolerance exceeded",
  "decayed error tolerance exceeded",
  "decayed delta error tolerance exceeded",
  "maximum number of epochs reached",
  "gradf returned zero length gradient",
  "division by zero averted",
  "engine produced bad search direction",
  "search direction orthogonal to gradient",
  "line search interpolation failed",
  "line search extrapolation failed",
  "minimum allowed change in line search step size exceeded",
  "minimum allowed change in line search objective exceeded",
  "maximum iterations of line search reached",
  "line search bracketing procedure failed"
};

#else

extern OPTIMIZER OPTIMIZER_DEFAULT;
extern const char *const opt_result_strings[];

#endif /* OWNER */

/* h2man:skipend */

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __OPTIMIZE_H__ */

