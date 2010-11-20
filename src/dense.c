
/* Copyright (c) 1995-96 by G. W. Flake. */

#include "nodelib/dataset.h"
#include "nodelib/misc.h"

#include <stdio.h>
#include <math.h>
#include <stdlib.h>

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

double de_kernal_gauss(double x)
{
  return(exp(-0.5 * x * x) / sqrt(2 * M_PI));
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

double de_estimate(DATASET *set, double var, double (*kernal)(double x),
		   double *x)
{
  unsigned n, m, i, j;
  double result, diff, hn, *xi;

  m = dataset_x_size(set);
  n = dataset_size(set);
  result = 0.0;
  hn = var / sqrt((double)n);
  for(i = 0; i < n; i++) {
    xi = dataset_x(set, i);
    diff = 0.0;
    for(j = 0; j < m; j++)
      diff += (xi[j] - x[j]) * (xi[j] - x[j]);
    result += kernal(sqrt(diff) / hn);
  }
  return(result / (n * hn));
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
