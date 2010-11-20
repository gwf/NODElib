
/* Copyright (c) 1996 by G. W. Flake. */

#include "nodelib/kmeans.h"
#include "nodelib/dataset.h"
#include "nodelib/dsmethod.h"
#include "nodelib/dsmatrix.h"
#include "nodelib/xalloc.h"
#include "nodelib/ulog.h"
#include "nodelib/misc.h"

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* Returns an array of integers from 1 to n the first ns of which have 
 * been shuffled.
 */

int *shuffle(int n, int ns)
{
  int i, j, *temp, swap;
  temp = (int*) xmalloc(n*sizeof(int));
  for(i=0;i<n;i++)
    temp[i] = i;
  for(i=0;i<ns;i++) {
    j = i + random() % (n-i);
    swap = temp[i];
    temp[i] = temp[j];
    temp[j] = swap;
  }
  return temp;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

DATASET *kmeans_initial_clusters(DATASET *inset, unsigned nclus, 
				 int clusinit, int **count, int **memb)
{
  unsigned i, j, dim, nex;
  int *shuf, clusno;
  double *tempp, *clusi, *exi;
  DATASET *initial;

  dim = dataset_x_size(inset);
  tempp = xmalloc(sizeof(double) * nclus * dim);
  initial = dataset_create(&dsm_matrix_method,
			   dsm_c_matrix(tempp, dim, 0, nclus));

  nex = dataset_size(inset);

  switch( clusinit ) {

  case KMEANS_RANDOM_EXEMPLARS:
    shuf = shuffle(nex,nclus);
    for(i = 0; i < nclus; i++) {
      clusi = dataset_x(inset, shuf[i]);
      /*
      printf("cluster: %d  shuffled value: %d  exemplar: %g\n",
	     i,shuf[i],clusi[0]);
     */
      for(j = 0; j < dim; j++)
	tempp[i * dim + j] = clusi[j];
    }
    xfree(shuf);
    *count = NULL;
    *memb = NULL;
    return initial;

  case KMEANS_RANDOM_PARTITION:
    shuf = shuffle(nex,nex);
    break;

  case KMEANS_SEQ_PARTITION:
    shuf = shuffle(nex,0);
    break;

  default:
    /* TO DO: Send error message?? */
    return (DATASET*) NULL;
  }  

  *count = (int *) xmalloc( sizeof(int) * nclus );
  *memb = (int *) xmalloc( sizeof(int) * nex );
  for(i=0;i<nclus*dim;i++)  tempp[i]=0; 
  for(i=0;i<nclus;i++)  (*count)[i]=0; 
  
  for(i=0;i<nex;i++) {
    clusno = i * nclus / nex;
    (*memb)[ shuf[i] ] = clusno;
    ++ (*count)[clusno];
    exi = dataset_x(inset, shuf[i]);
    for(j=0;j<dim;j++)
      tempp[clusno * dim + j] += exi[j];
  }
  /* Normalize sum to make it a mean */
  for(i=0;i<nclus;i++) {
    for(j=0;j<dim;j++)
      tempp[i * dim + j] /= (*count)[i];
  }

  xfree(shuf);
  return initial;
}
     

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static double eucdis2(double *x, double *y, int d)
{
  int i;
  double dis = 0;

  for(i = 0; i < d; i++)
    dis += (x[i] - y[i]) * (x[i] - y[i]);
  return dis;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* Given 'inset' (which has n x-dimensional data points), perform keans
   clustering and return 'nclus' clusters in the returned DATASET.  The
   algorithm proceeds for at most 'maxiters' steps or until the fractional
   decrease in the the distortion for one iteration is less that 'minfrac'.
  
   If 'maxit' is set to zero, then 'minfrac' controls termination.

   If 'minfrac' is less than zero, then 'maxiters' controls termination.

   If 'minfrac' is less than zero and 'maxit' is equal to zero, then
   the routine does nothing and returns NULL. 

   If 'clusinit' is zero, the clusters are initialized to random exemplars.

   If 'clusinit' is one, the clusters are initialized to the means of a
   random partitioning of the entire data set.

   If 'clusinit' is two, the clusters are initialized to the means of a
   sequential partitioning of the entire data set.

   Thanks to Chris Darken for the code from which this is based.

*/

DATASET *kmeans(DATASET *inset, unsigned nclus, double minfrac,
		unsigned maxiters, int clusinit)
{
  DATASET *outset, *initial;
  int *memb, *count;
  unsigned i, j, k, nsamps, dim;
  double **clus, *sampi, *tempp, sumerror, lasterror=-1, temp;
  long closeid, old;
  double closedis, *clusi;	
  
  if(minfrac < 0) minfrac = 0;
  nsamps = dataset_size(inset);
  dim = dataset_x_size(inset);

  if(nsamps < nclus) {
    ulog(ULOG_ERROR, "kmeans: set size (%d) is less than # clusters (%d).",
	 nsamps, nclus);
    return(NULL);
  }
  if(minfrac == 0 && maxiters == 0) {
    ulog(ULOG_ERROR, "kmeans: invalid termination condition.");
    return(NULL);
  }

  tempp = xmalloc(sizeof(double) * nclus * dim);
  outset = dataset_create(&dsm_matrix_method,
			  dsm_c_matrix(tempp, dim, 0, nclus));

  if(nclus == nsamps) {
    for(i = 0; i < nsamps; i++) {
      sampi = dataset_x(inset, i);
      for(j = 0; j < dim; j++)
	tempp[i * dim + j] = sampi[j];
    }
    return(outset);
  }

  initial = kmeans_initial_clusters(inset,nclus,clusinit,&count,&memb);
  if( initial == NULL ) {
    /* TO DO: generate error message ?? */
    xfree(dsm_destroy_matrix(dataset_destroy(outset)));
    if( memb != NULL ) {
      xfree(memb);
      xfree(count);
    }
    return (DATASET*) NULL;
  }

  clus = xmalloc(sizeof(double *) * nclus);
  for(i = 0; i < nclus; i++) {
    clusi = dataset_x(initial,i);
    for(j = 0; j < dim; j++) 
      tempp[i * dim + j] = clusi[j];
    clus[i] = tempp + i * dim;
  }
  xfree(dsm_destroy_matrix(dataset_destroy(initial)));

  /* Calculate the initial membership and count arrays, if necessary,
   * and locate clusters at the mean of their owned exemplars
   */
  if( memb == NULL ) { 
    memb = (int *) xcalloc(nsamps,sizeof(int));
    count = (int *) xcalloc(nclus,sizeof(int));
    for(i=0;i<nsamps;i++) {
      closeid = 0;
      sampi = dataset_x(inset, i);
      closedis = eucdis2(sampi, clus[0], dim);
      for(j=1;j<nclus;j++) {
	if( (temp = eucdis2(sampi, clus[j], dim)) < closedis) {
	  closeid = j;
	  closedis = temp;
	}
      }
      ++ count[closeid];
      memb[i] = closeid;
    }
    for(i=0;i<nclus;i++) {
      for(j=0;j<dim;j++)
	clus[i][j]=0;
    }
    for(i=0;i<nsamps;i++) {
      sampi = dataset_x(inset, i);
      for(j=0;j<dim;j++)
	clus[memb[i]][j] += sampi[j];
    }
    for(i=0;i<nclus;i++) {
      for(j=0;j<dim;j++)
	clus[i][j] /= count[i];
    }
  }

  /*
  printf("Cluster locations before main loop:\n");
  for(i=0;i<nclus;i++)
    printf("%g\n",clus[i][0]);
  */


  /* Main kmeans loop. */
  for(k = 0; (k < maxiters) || (maxiters == 0); k++) {
    for(i = 0; i < nsamps; ++i) {
      old = memb[i];
      closeid = 0;
      sampi = dataset_x(inset, i);
      closedis = eucdis2(sampi, clus[0], dim);
      for(j = 1; j < nclus; j++) {
	temp = eucdis2(sampi, clus[j], dim);
	if(temp < closedis) {
	  closeid = j;
	  closedis = temp;
	}
      }

      if(closeid != memb[i]) {
	memb[i] = closeid;

	/* remove from old cluster */
	for(j = 0; j < dim; j++) {
	  if(count[old] != 0 && count[old] != 1) {
	    clus[old][j] -= sampi[j] / count[old];
	    clus[old][j] *= (double)count[old] / (count[old] - 1);
	  }
	}
	--count[old];

	/* add to closest cluster */
	for(j = 0; j < dim; j++) {
	  if(count[old] != -1) {
	    clus[closeid][j] *= (double)count[closeid] / (count[closeid] + 1);
	    clus[closeid][j] += sampi[j] / (count[closeid] + 1);
	  }
	}
	++count[closeid];
      }
    }

    /* calculate error */
    sumerror = 0;
    for(i = 0; i < nsamps; i++) {
      sampi = dataset_x(inset, i);
      sumerror += eucdis2(sampi, clus[memb[i]], dim); 
    }
    if(lasterror > 0 && (lasterror - sumerror) / sumerror <= minfrac)
      break;
    lasterror = sumerror;
  }      

  xfree(memb);
  xfree(count);
  xfree(clus);
  return(outset);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* Given 'inset' (which has n x-dimensional data points), perform online
   kmeans clustering and return 'nclus' clusters in the returned DATASET.  
   The algorithm operates on 'maxiters' shuffled samples, reshuffling the
   data set as necessary.

   If 'clusinit' is zero, the clusters are initialized to random exemplars.
   This is the usual choice for online kmeans.

   If 'clusinit' is one, the clusters are initialized to the means of a
   random partitioning of the entire data set.

   If 'clusinit' is two, the clusters are initialized to the means of a
   sequential partitioning of the entire data set.

*/

DATASET *kmeans_online(DATASET *inset, unsigned nclus, unsigned maxiters,
		       int clusinit)
{
  DATASET *outset, *initial;
  int *memb, *count, *shuf = NULL;
  unsigned i, j, k, nsamps, dim;
  double **clus, *sampi, *tempp, temp;
  long closeid;
  double closedis, *clusi;
  double eta;

  nsamps = dataset_size(inset);
  dim = dataset_x_size(inset);

  if(nsamps < nclus) {
    ulog(ULOG_ERROR, "kmeans: set size (%d) is less than # clusters (%d).",
	 nsamps, nclus);
    return(NULL);
  }

  tempp = xmalloc(sizeof(double) * nclus * dim);
  outset = dataset_create(&dsm_matrix_method,
			  dsm_c_matrix(tempp, dim, 0, nclus));

  if(nclus == nsamps) {
    for(i = 0; i < nsamps; i++) {
      sampi = dataset_x(inset, i);
      for(j = 0; j < dim; j++)
	tempp[i * dim + j] = sampi[j];
    }
    return(outset);
  }

  initial = kmeans_initial_clusters(inset,nclus,clusinit,&count,&memb);
  /* memb and count are not used in online kmeans */
  if( memb != NULL ) {
    xfree(memb);
    xfree(count);
  }
  if( initial == NULL ) {
    /* TO DO: generate error message ?? */
    xfree(dsm_destroy_matrix(dataset_destroy(outset)));
    xfree(tempp);
    return (DATASET*) NULL;
  }

  clus = xmalloc(sizeof(double *) * nclus);
  for(i = 0; i < nclus; i++) {
    clusi = dataset_x(initial,i);
    for(j = 0; j < dim; j++) 
      tempp[i * dim + j] = clusi[j];
    clus[i] = tempp + i * dim;
  }
  xfree(dsm_destroy_matrix(dataset_destroy(initial)));

  /* Main kmeans loop. */
  eta = 0.01;
  for(k = 0; k < maxiters; k++) {
    if( k > maxiters/2 )
      eta = 0.001;
    if(k % nsamps == 0) {
      if(shuf != NULL)  xfree(shuf);
      shuf = shuffle(nsamps,nsamps);
      i = 0;
    }
    closeid = 0;
    sampi = dataset_x(inset, shuf[i++]);
    closedis = eucdis2(sampi, clus[0], dim);
    for(j = 1; j < nclus; j++) {
      temp = eucdis2(sampi, clus[j], dim);
      if(temp < closedis) {
	closeid = j;
	closedis = temp;
      }
    }
    for(j = 0; j < dim; j++ )
      clus[closeid][j] += eta * ( sampi[j] - clus[closeid][j] );
  }
    
  xfree(clus);
  if(shuf != NULL) xfree(shuf);
  return(outset);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */















