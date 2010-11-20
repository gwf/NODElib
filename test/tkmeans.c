/* From darken Wed May 28 10:39 EDT 1997 */

#include <nodelib.h>

static char *help_string = "\
Test the kmeans algorithms on uniform exemplars from [0,1]\n\
The options for 'init' correspond to initializing the\n\
cluster locations to:\n\
\t0: Random exemplars\n\
\t1: The centroids of a random partition of the data\n\
\t2: The centroids of a sequential partition of the data\n\
'maxiters' is the number of passes through the entire data set\n\
for offline kmeans and the total number of online steps for online kmeans.\n\
A reasonable value for online kmeans is 5*n.\n\
The optimal solution has centers at .166..., 0.5, and .833...\n";

int main(int argc, char** argv)
{
  DATASET *ds, *cl;
  double *tempp;
  int i, dim=1, n, nclus=3, km_init=0, online, init, seed, maxiters;

  OPTION opts[] = {
    { "-online",    OPT_SWITCH, &online,  "Online variant of the algorithm"},
    { "-init",      OPT_INT,    &init,    "See above"},
    { "-n",         OPT_INT,    &n,       "Number of random exemplars"},
    { "-seed",      OPT_INT,    &seed,    "Seed for random number generator"},
    { "-maxiters",  OPT_INT,    &maxiters,"See above"},
    { NULL,         OPT_NULL,   NULL,       NULL                           }
  };

  online = 0;  
  init = 0;
  n = 10000;
  seed = 12345;
  maxiters = 10;

  /* Parse command line */
  get_options(argc, argv, opts, help_string, NULL, 0);
  switch(init) {
  case 0:
    km_init = KMEANS_RANDOM_EXEMPLARS;
    break;
  case 1:
    km_init = KMEANS_RANDOM_PARTITION;
    break;
  case 2:
    km_init = KMEANS_SEQ_PARTITION;
    break;
  default:
    fprintf(stderr,"Illegal value for 'init' (%d)\nExiting.\n",init);
    exit(9);
  }

  tempp = xmalloc(sizeof(double) * n * dim);
  ds = dataset_create(&dsm_matrix_method,
		      dsm_c_matrix(tempp, dim, 0, n));
  srandom(seed);

  for(i=0;i<n;i++) {
    tempp[i] = random_range(0,1);
  }

  if(online)
    cl = kmeans_online(ds,nclus,maxiters,km_init);
  else
    cl = kmeans(ds,nclus,0,maxiters,km_init);

  printf("Final Cluster Locations\n");
  for(i=0;i<nclus;i++)
    printf("%g\n",dataset_x(cl,i)[0]);

  exit(0);
}


