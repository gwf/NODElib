
/* Copyright (c) 1996 by Gary William Flake. */


/* This is a simple example of how to use NODElib.  -- GWF */

#include <nodelib.h>
#include <stdio.h>

char *help_string = "\
This test program will randomly generate 'n' points from a 'd'\n\
dimensional space, each of which is centered on one of 'm' clusters\n\
that has a Gaussian distribution with a variance of 'var'.  The\n\
program will then attempt to cluster the 'n' points into 'm' clusters\n\
with the k-means algorithm.  Termination of the algorithm is specified\n\
by 'maxi' and 'tol' which are used per the k-means documentation.\n";


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

int main(int argc, char **argv)
{
  /* These variables are for command-line options.
   */
  int seed1 = 0, seed2 = 0, n = 100, d = 3, m = 5, maxi = 100;
  int online = 0, init = 0;
  double var = 0.5, tol = 0.0;

  /* The OPTION array is used to easily parse command-line options.
   */
  OPTION opts[] = {
    { "-n",         OPT_INT,    &n,      "number of points"          },
    { "-d",         OPT_INT,    &d,      "input dimensionality"      },
    { "-m",         OPT_INT,    &m,      "number of clusters"        },
    { "-var",       OPT_DOUBLE, &var,    "ditribution variance"      },
    { "-tol",       OPT_DOUBLE, &tol,    "fractional tolerance"      },
    { "-seed1",     OPT_INT,    &seed1,  "first random number seed"  },
    { "-seed2",     OPT_INT,    &seed2,  "second random number seed" },
    { "-maxi",      OPT_INT,    &maxi,   "maximum iterations"        },
    { "-online",    OPT_SWITCH, &online, "use online k-means?"       },
    { "-init",      OPT_INT,    &init,   "init method (0, 1, 2)"     },
    { NULL,         OPT_NULL,   NULL,    NULL                        }
  };

  int i, j, k;
  double *points, *clusts, *x;
  DATASET *data, *kclusts;

  /* Get the command-line options.
   */
  get_options(argc, argv, opts, help_string, NULL, 0);

  /* Set the random seed.
   */
  srandom(seed1);

  points = xmalloc(sizeof(double) * n * d);
  clusts = xmalloc(sizeof(double) * m * d);

  /* Pick unform random clusters from [-10, 10]. 
   */
  for(i = 0; i < m; i++)
    for(j = 0; j < d; j++)
      clusts[i * d + j] = random_range(-10, 10);

  /* Randomly generate points from clusters.
   */
  for(i = 0; i < n; i++) {
    k = random_range(0, m);
    for(j = 0; j < d; j++)
      points[i * d + j] = clusts[k * d + j] + random_gauss() * var;
  }
  
  /* Wrap the points into a DATASET so that kmeans() can "talk"
   * to it.
   */
  data = dataset_create(&dsm_matrix_method, dsm_c_matrix(points, d, 0, n));
  
  /* Do the clustering and output the results.
   */
  srandom(seed2);
  if(!online)
    kclusts = kmeans(data, m, tol, maxi, init);
  else
    kclusts = kmeans_online(data, m, maxi, init);
  if(kclusts == NULL) {
    printf("There was a problem with the clustering. Check "
	   "the command-line options.\n");
    exit(1);
  }
  else {
    printf("Original clusters:\n");
    for(i = 0; i < m; i++) {
      for(j = 0; j < d; j++)
	printf("% .5f  ", clusts[i * d + j]);
      printf("\n");
    }
    printf("\nComputed clusters:\n");
    for(i = 0; i < m; i++) {
      x = dataset_x(kclusts, i);
      for(j = 0; j < d; j++)
	printf("% .5f  ", x[j]);
      printf("\n");
    }
  }

  xfree(dsm_destroy_matrix(dataset_destroy(data)));
  xfree(dsm_destroy_matrix(dataset_destroy(kclusts)));
  xfree(clusts);

  exit(0); 
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

