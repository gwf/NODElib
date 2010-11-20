
/* Copyright (c) 1995-97  by G. W. Flake. */


#include <math.h>

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

double opt_err_quadratic(double output, double target, double *derivative,
			 double *second_derivative)
{
  double err, diff;

  diff = output - target;
  err = 0.5 * (diff * diff);
  *derivative = diff;
  *second_derivative = 1.0;
  return(err);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

double opt_err_logistic(double output, double target, double *derivative,
			double *second_derivative)
{
  double err, diff;

  diff = output - target;
  err = log(cosh(diff));
  *derivative = tanh(diff);
  *second_derivative = (1.0 - *derivative) * (1.0 + *derivative);
  return(err);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

double opt_err_huber(double output, double target, double *derivative,
		     double *second_derivative)
{
  double ferr, err, diff;

  diff = output - target;
  ferr = fabs(diff);
  err = (ferr < 1.0) ? 0.5 * diff * diff : ferr - 0.5;
  *derivative = (ferr < 1.0) ? diff : (diff > 0) ? 1.0 : -1.0;
  *second_derivative = (ferr < 1.0) ? 1.0 : 0.0;
  return(err);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* E           = -d * log(y) - (1-d) * log(1-y)
   dC/dy       = -d / y + (1-d) / (1-y)
   d^2C/(dy)^2 = d / y^2 + (1-d) / (1-y)^2

 */

double opt_err_cross_entropy(double output, double target,
			     double *derivative, double *second_derivative)
{
  double err, epsilon = 1e-8;

  target = (target < 0) ? 0 : (target > 1) ? 1 : target;
  output = (output < 0) ? epsilon : (output > 1) ? 1 - epsilon : output;
  if (target == output) {
    err = 0;
    *derivative = -target / output;
    *second_derivative = -target / (output * output);
  }
  else {
    err = -target * log(output) - (1 - target) * log(1 - output);
    *derivative = -target / output + (1 - target) / (1 - output);
    *second_derivative = -target / (output * output) +
      (1 - target) / ((1 - output) * (1 - output));
  }
  return(err);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/*
   Same as above but does (x + 1) / 2 to both the output and the target.

   E           = -d * log(y) - (1-d) * log(1-y)
   dC/dy       = -d / y + (1-d) / (1-y)
   d^2C/(dy)^2 = d / y^2 + (1-d) / (1-y)^2
 */

double opt_err_symmetric_cross_entropy(double output, double target,
				       double *derivative,
				       double *second_derivative)
{
  target = (target + 1) / 2;
  output = (output + 1) / 2;
  return(opt_err_cross_entropy(output, target, derivative, second_derivative));
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

