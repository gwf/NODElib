
/* Copyright (c) 1996 by G. W. Flake. */

/* A test for the pinv() routine... */

#include <nodelib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

int main(int argc, char **argv)
{
  extern int svd_original;
  int n = 10, seed = 0, sym = 0, orig = 0;
  double mag = 1;
  OPTION opts[] = {
    { "-seed",  OPT_INT,    &seed,    "random number seed"        },
    { "-n",     OPT_INT,    &n,       "size of matrix"            },
    { "-mag",   OPT_DOUBLE, &mag,     "max magnitude of elements" },
    { "-sym",   OPT_SWITCH, &sym,     "symmetric matrix?"         },
    { "-orig",  OPT_SWITCH, &orig,    "original (stupid) SVD?"    },
    { NULL,     OPT_NULL,   NULL,     NULL                        }
  };
  double **A, **Ainv, sum, error, target;
  unsigned i, j, k;

  get_options(argc, argv, opts, NULL, NULL, 0);
  
  svd_original = orig;
  srandom(seed);
  
  A = allocate_array(2, sizeof(double), n, n);
  Ainv = allocate_array(2, sizeof(double), n, n);

  for(i = 0; i < n; i++)
    for(j = 0; j < n; j++)
      A[i][j] = random_range(-mag, mag);
  if(sym) {
    memcpy(&Ainv[0][0], &A[0][0], n * n * sizeof(double));
    for(i = 0; i < n; i++)
      for(j = 0; j < n; j++) {
	sum = 0;
	for(k = 0; k < n; k++)
	  sum += Ainv[i][k] * Ainv[j][k];
	A[i][j] = sum;
      }
  }
  
  printf("A = \n");
  for(i = 0; i < n; i++) {
    for(j = 0; j < n; j++)
      printf("% .5f  ", A[i][j]);
    printf("\n");
  }

  if(sym)
    spinv(A, Ainv, n);
  else
    pinv(A, Ainv, n);
  
  error = 0;
  for(i = 0; i < n; i++)
    for(j = 0; j < n; j++) {
      sum = 0;
      for(k = 0; k < n; k++)
	sum += A[i][k] * Ainv[k][j];
      target = (i == j) ? 1 : 0;
      error += (sum - target) * (sum - target);
    }

  printf("\nerror = %f\n\n", error);

  printf("Ainv = \n");
  for(i = 0; i < n; i++) {
    for(j = 0; j < n; j++)
      printf("% .5f  ", Ainv[i][j]);
    printf("\n");
  }

  deallocate_array(A);
  deallocate_array(Ainv);
  
  exit(0); 
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
