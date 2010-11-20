

/* Copyright (c) 1995 by G. W. Flake.
 *
 * NAME
 *   errfunc.h - error functions for optimization routines
 * SYNOPSIS
 *   Error functions currently in this module include the standard
 *   Quadratic, the logistic error function, and Huber's error
 *   function.
 * DESCRIPTION
 *   The eror functions declared in this module can be used
 *   with the optimization package.  Each function takes four
 *   arguments: an actual output, a target value for the output,
 *   and two pointers to doubles where the first and second
 *   derivatives are computed and stored.  The error value is
 *   returned.
 * AUTHOR
 *   Gary William Flake (\url{\bf{gary.flake@usa.net}}{mailto:gary.flake@usa.net}).
 */


#ifndef __ERRFUNC_H__
#define __ERRFUNC_H__

#include "nodelib/etc/version.h"
#include "nodelib/etc/options.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* The standard quadratic error function: 0.5 * (output - target)^2. */

double opt_err_quadratic(double output, double target, /*\*/
			 double *derivative, double *second_derivative);



/* This is the logistic error function from statistics, defined as
   \bf{ln(cosh(}\em{e}\bf{))}, where \em{e} is the difference between the
   actual and the desired outputs. */

double opt_err_logistic(double output, double target, /*\*/
			double *derivative, double *second_derivative);


/* Huber's function is also from robust estimation.  It is defined as
   \bf{e*e/2} for \bf{|e| <= 1} and \bf{|e| - 1/2} for \bf{|e| > 1},
   where \bf{e} is the difference between the actual and the desired
   outputs. */

double opt_err_huber(double output, double target, /*\*/
		     double *derivative, double *second_derivative);


/* Cross entropy error function which is useful for classification
   problems.  This function expects \em{target} and \em{output} to be
   strictly binary. */

double opt_err_cross_entropy(double output, double target, /*\*/
			     double *derivative, double *second_derivative);


/* Same as cross entropy but renormalizes \em{target} and \em{output} with
   (x + 1) / 2, to make the function suitable for (-1, 1) outputs. */

double opt_err_symmetric_cross_entropy(double output, double target, /*\*/
				       double *derivative, /*\*/
				       double *second_derivative);

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __ERRFUNC_H__ */

