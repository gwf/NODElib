
/* Copyright (c) 1995-96 by G. W. Flake.
 *
 * NAME
 *   dense.h - basic density estimation routines
 * SYNOPSIS
 *   Given a DATASET, the routines in this package allow you to
 *   construct a simple density estimator with the kernel function
 *   of your choice.
 * AUTHOR
 *   Gary William Flake (\url{\bf{gary.flake@usa.net}}{mailto:gary.flake@usa.net}).
 */

#ifndef __DENSE_H__
#define __DENSE_H__

#include "nodelib/dataset.h"
#include "nodelib/etc/version.h"
#include "nodelib/etc/options.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* A Gaussian kernal for density estimation.  You can add your own
   kernel functions, if you like. */

double de_kernal_gauss(double x);


/* Computes the density of the data in \em{set} with\em{kernel}
   as a kernel function with variance \em{var}, and with respect
   to the input, \em{x}. */

double de_estimate(DATASET *set, double var, double (*kernal)(double x), /*\*/
		   double *x);

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __DENSE_H__ */
