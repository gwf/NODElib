
/* Copyright (c) 1996 by G. W. Flake. */

/* A test for the pca() routine... */

#include <nodelib.h>
#include <stdio.h>
#include <math.h>

char *help = "Compute principle components of Gaussian distribution.\n";

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

int main(int argc, char **argv)
{
  int n = 500, d = 5, p = 3, seed = 0;
  OPTION opts[] = {
    { "-seed",  OPT_INT,    &seed,    "random number seed"             },
    { "-d",     OPT_INT,    &d,       "number of dimensions"           },
    { "-p",     OPT_INT,    &p,       "number of principle components" },
    { "-n",     OPT_INT,    &n,       "number of points"               },
    { NULL,     OPT_NULL,   NULL,     NULL                             }
  };
  DATASET *dset;
  unsigned i, j;
  double **data, *var, *offset, **D, *S, *M;
  
  get_options(argc, argv, opts, help, NULL, 0);
  if(p > d) {
    fprintf(stderr, "tpca: number components is larger than dimensions\n");
    exit(1);
  }
  srandom(seed);

  data = allocate_array(2, sizeof(double), n, d);
  var = allocate_array(1, sizeof(double), d);
  offset = allocate_array(1, sizeof(double), d);

  printf("offsets =\n");
  for(i = 0; i < d; i++) {
    offset[i] = random_range(-100, 100);
    printf("% .3f ", offset[i]);
  }
  printf("\n\n");

  printf("variances =\n");
  for(i = 0; i < d; i++) {
    var[i] = random_range(0, 20);
    printf("% .3f ", var[i]);
  }
  printf("\n\n");

  for(i = 0; i < n; i++)
    for(j = 0; j < d; j++)
      data[i][j] = random_gauss() * var[j] + offset[j];

  dset = dataset_create(&dsm_dblptr_method, dsm_c_dblptr(data, d, 0, n));
  pca(dset, p, &D, &S, &M);

  printf("means =\n");
  for(i = 0; i < d; i++)
    printf("% .3f ", M[i]);
  printf("\n\n");

  printf("eigenvalues =\n");
  for(i = 0; i < p; i++)
    printf("% .3f ", S[i]);
  printf("\n\n");

  printf("principle components =\n");
  for(i = 0; i < p; i++) {
    for(j = 0; j < d; j++)
      printf("% .3f ", D[i][j]);
    printf("\n");
  }

  deallocate_array(data);
  deallocate_array(var);
  deallocate_array(offset);
  deallocate_array(D);
  deallocate_array(S);
  deallocate_array(M);

  dataset_destroy(dset);

  exit(0);
}


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

