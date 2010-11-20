
/* Copyright (c) 1998 by G. W. Flake. */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "nodelib/svm.h"


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

double svm_kernel_gauss(double *x1, double *x2, double aux, int dim)
{
  int i;
  double d, sum = 0;

  for(i = 0; i < dim; i++) {
    d = x1[i] - x2[i];
    sum += d * d;
  }
  return exp(-sum / (aux * aux));    
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

double svm_kernel_poly(double *x1, double *x2, double aux, int dim)
{
  int i;
  double sum = 0;

  for(i = 0; i < dim; i++) {
    sum += x1[i] * x2[i];
  }
  if (aux == 1) return (sum + 1);
  if (aux == 2) return (sum + 1) * (sum + 1);
  if (aux == 3) return (sum + 1) * (sum + 1) * (sum + 1);
  if (aux == 4) return (sum + 1) * (sum + 1) * (sum + 1) * (sum + 1);
  return pow((sum + 1), aux);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

double svm_kernel_tanh(double *x1, double *x2, double aux, int dim)
{
  int i;
  double sum = 0;

  for(i = 0; i < dim; i++) {
    sum += x1[i] * x2[i];
  }
  return tanh(sum - aux);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

double svm_kernel_linear(double *x1, double *x2, double aux, int dim)
{
  int i;
  double sum = 0;

  for(i = 0; i < dim; i++) {
    sum += x1[i] * x2[i];
  }
  return sum;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

double svm_kernel_coulomb(double *x1, double *x2, double aux, int dim)
{
  int i;
  double d, sum = 0, r, R = aux, out = 0;

  for(i = 0; i < dim; i++) {
    d = x1[i] - x2[i];
    sum += d * d;
  }
  
  r = sqrt(sum);
  if (sum == 0.0) return 0.0;

  
  dim += 2; // KLUGE
  if (dim == 2) {
    if (r > R) {
      out = -log(r);
    }
    else {
      out = -log (r) * r * r / (R * R);
    }
  }
  else {
    if (r > R) {
      out = pow(r, 2 - dim);
    }
    else {
      out = r * r / pow(R, dim);
    }
  }
  return out;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


