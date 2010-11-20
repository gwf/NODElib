
/* Copyright (c) 1995, 1996 by G. W. Flake. */

#include <math.h>

#define NN_SOLVE_OWNER 1

#include "nodelib/nn.h"
#include "nodelib/dataset.h"
#include "nodelib/kmeans.h"
#include "nodelib/dsmatrix.h"
#include "nodelib/misc.h"
#include "nodelib/svd.h"
#include "nodelib/array.h"

int nn_kmeans_online = 0;
int nn_kmeans_maxiters = 100;
int nn_kmeans_clusinit = 0;
double nn_kmeans_minfrac = 0;

int nn_rbf_centers_random = 0;
int nn_smlp_centers_random = 0;
int nn_rbf_basis_normalized = 0;

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static int nn_solve_all_internal(NN *nn, DATASET *set, double *weights)
{
  NN_LAYER *src;
  NN_LINK *link, **links;
  NN_ACTFUNC *af;
  NN_NETFUNC *nfl, *nfd, *nfq;
  unsigned in, out, n, i, j, k, l, m, nl, nweights, nlinks;
  unsigned numslabslinear;
  double *x, *y, **AtA, **Atb, *Arow, sum;
  
  /* First, check that the dataset and nn are compatible. */
  n = dataset_size(set);
  in = dataset_x_size(set);
  out = dataset_y_size(set);
  if(in != nn->numin || out != nn->numout) {
    ulog(ULOG_ERROR, "nn_solve: I/O dimensions are incompatible.%t"
         "NN dimension = (%d x %d)%tDATASET dimension = (%d x %d).",
         nn->numin, nn->numout, in, out);
    return(1);
  }

  /* Grab pointers to these net function so that we can check the
   * type of the net functions in the NN.
   */
  nfl = nn_find_netfunc("l");
  nfd = nn_find_netfunc("d");
  nfq = nn_find_netfunc("q");

  /* Count the number of slabs in the output layer that have a
   * linear activation function.
   */
  af = nn_find_actfunc("none");
  numslabslinear = 0;
  for(i = 0; i < nn->layers[nn->numlayers - 1].numslabs; i++)
    if(nn->layers[nn->numlayers - 1].slabs[i].afunc->func == af->func)
      numslabslinear++;

  /* Step through all of the links and collect pointers to all links
   * that can be solved.
   */
  nweights = 0;
  nlinks = 0;
  nl = nn->numlayers;
  links = allocate_array(1, sizeof(NN_LINK *), nn->numlinks);
  for(i = 0; i < nn->numlinks; i++) {
    link = nn->links[i];

    /* Is the destination the output layer? */
    if(link->dest->layer->idl != (int)nl - 1)
      continue;

    /* Is the output linear in the link's weights? */
    if(link->nfunc->forward != nfl->forward &&
       link->nfunc->forward != nfd->forward &&
       link->nfunc->forward != nfq->forward)
      continue;
    
    /* If the destination is an entire layer, is the whole layer
     * linear?
     */
    if(link->dest->layer->ids == -1 && 
       nn->layers[nn->numlayers - 1].numslabs != numslabslinear)
      continue;

    /* If the destination is a sublayer, is the sublayer linear? */
    if(link->dest->layer->ids != -1 &&
       link->dest->layer->afunc->func != af->func)
      continue;

    /* We've made it this far, so we can accept this link as one
     * that should be solved.
     */
    links[nlinks++] = link;
    nweights += link->numweights;
  }

  if(nlinks == 0) {
    ulog(ULOG_ERROR, "nn_solve_all: no links to solve.");
    return(1);
  }
  
  /**********************************************************************/

  /* Compute linear weights w.r.t. LMS.  The overall strategy is to
   * zero out the weights, do a forward call, then solve for the
   * residuals.
   */

  /* Step 1: zero out all weights to be solved. */
  for(i = 0; i < nlinks; i++) {
    link = links[i];
    /* Linear link */
    if(link->nfunc->forward == nfl->forward)
      for(j = 0; j < link->numout; j++) {
	for(k = 0; k < link->numin; k++)
	  link->u[j][k] = 0;
	link->a[j] = 0;
      }
    /* Diagonal quadratic link */
    else if(link->nfunc->forward == nfd->forward)
      for(j = 0; j < link->numout; j++) {
	for(k = 0; k < link->numin; k++)
	  link->u[j][k] = link->v[j][k] = 0;
	link->a[j] = 0;
      }
    /* Quadratic link */
    else if(link->nfunc->forward == nfq->forward)
      for(j = 0; j < link->numout; j++) {
	for(k = 0; k < link->numin; k++) {
	  for(l = 0; l < link->numin; l++)
	    link->A[j][k][l] = 0;
	  link->u[j][k] = 0;
	}
	link->a[j] = 0;
      }
  }

  /* Step 2: Get some space...  */
  AtA = allocate_array(2, sizeof(double), nweights, nweights);
  Atb = allocate_array(2, sizeof(double), out, nweights);
  Arow = allocate_array(1, sizeof(double), nweights);

  /* Step 3: Zero out AtA and Atb. */
  for(i = 0; i < nweights; i++)
    for(j = 0; j < nweights; j++)
      AtA[i][j] = 0.0;
  for(i = 0; i < out; i++)
    for(j = 0; j < nweights; j++)
      Atb[i][j] = 0.0;

  /* Step 4: Compute AtA and Atb.  For each pattern, k, compute the
   * cumulative sum of AtA[i][j] which involves n seperate feedforward
   * calls.  For Atb we must do it for each output.
   */
  for(k = 0; k < n; k++) {
    x = dataset_x(set, k);
    y = dataset_y(set, k);

    /* Sanity check on input. */
    for(i = 0; i < nn->numin; i++)
      if(x[i] != x[i])
	continue;

    nn_forward(nn, x);

    /* Step 4.1: Form Arow data by examining the source end of
     * every link.
     */
    m = 0;
    for(i = 0; i < nlinks; i++) {
      link = links[i];
      src = link->source->layer;
      /* Linear link */
      if(link->nfunc->forward == nfl->forward) {
	for(j = 0; j < link->numin; j++)
	  Arow[m++] = src->y[j]; /* link->u[][] */
	Arow[m++] = 1; /* link->a[] */
      }
      /* Diagonal quadratic link */
      else if(link->nfunc->forward == nfd->forward) {
	for(j = 0; j < link->numin; j++)
	  Arow[m++] = src->y[j]; /* link->u[][] */
	for(j = 0; j < link->numin; j++)
	  Arow[m++] = src->y[j] * src->y[j]; /* link->v[][] */
	Arow[m++] = 1; /* link->a[] */	
      }
      /* Quadratic link */
      else if(link->nfunc->forward == nfq->forward) {
	for(j = 0; j < link->numin; j++)
	  for(l = 0; l < link->numin; l++)
	    Arow[m++] = src->y[j] * src->y[l]; /* link->A[][][] */
	for(j = 0; j < link->numin; j++)
	  Arow[m++] = src->y[j]; /* link->u[][] */
	Arow[m++] = 1; /* link->a[] */
      }
    }
    
    /* Sanity check. */
    if(m != nweights)
      ulog(ULOG_FATAL, "nn_solve_all_internal: mismatch on weight count"
	   " (%d != %d)", m, nweights);

    /* Step 4.2: Form AtA and Atb from Arow. */
    for(i = 0; i < nweights; i++) {
      for(j = 0; j < nweights; j++)
	AtA[i][j] = Arow[i] * Arow[j];
      for(j = 0; j < out; j++)
	/* Sanity check on output. */
	if(y[j] == y[j])
	  Atb[j][i] += Arow[i] * (y[j] - nn->y[j]);
    }
  }

  /* Step 5: Invert AtA to get (A^t * A)^{-1}. */
  spinv(AtA, AtA, nweights);

  /* Step 6: For each output, calculate AtAiAtb to get the LMS weights. */ 
  for(l = 0; l < out; l++) {
    for(i = 0; i < nweights; i++) {
      sum = 0.0;
      for(j = 0; j < nweights; j++)
	sum += AtA[i][j] * Atb[l][j];
      /* Put new weights in Arow. */
      Arow[i] = sum;
    }

    /* Step 6.1: Replace the weights in the same order. */
    m = 0;
    for(i = 0; i < nlinks; i++) {
      link = links[i];
      /* Linear link */
      if(link->nfunc->forward == nfl->forward) {
	for(k = 0; k < link->numin; k++)
	  link->u[l][k] = Arow[m++];
	link->a[l] = Arow[m++];
      }
      /* Diagonal quadratic link */
      else if(link->nfunc->forward == nfd->forward) {
	for(k = 0; k < link->numin; k++)
	  link->u[l][k] = Arow[m++];
	for(k = 0; k < link->numin; k++)
	  link->v[l][k] = Arow[m++];
	link->a[l] = Arow[m++];
      }
      /* Quadratic link */
      else if(link->nfunc->forward == nfq->forward) {
	for(k = 0; k < link->numin; k++)
	  for(j = 0; j < link->numin; j++)
	    link->A[l][j][k] = Arow[m++];
	for(k = 0; k < link->numin; k++)
	  link->u[l][k] = Arow[m++];
	link->a[l] = Arow[m++];
      }
    }
  }

  deallocate_array(links);
  deallocate_array(Arow);
  deallocate_array(AtA);
  deallocate_array(Atb);

  return(0);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

int nn_solve_all(NN *nn, DATASET *set)
{
  return(nn_solve_all_internal(nn, set, NULL));
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

int nn_weighted_solve_all(NN *nn, DATASET *set, unsigned linknum,
			  double *weights)
{
  return(nn_solve_all_internal(nn, set, weights));
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static int nn_solve_internal(NN *nn, DATASET *set, unsigned linknum,
			     double *weights)
{
  NN_LAYER *src;
  NN_LINK *link;
  NN_ACTFUNC *af;
  NN_NETFUNC *nf;
  unsigned in, out, n, i, j, k, l, nl, nbasis;
  double *x, *y, **AtA, **Atb, sum;

  /* First, check that the dataset and nn are compatible. */
  n = dataset_size(set);
  in = dataset_x_size(set);
  out = dataset_y_size(set);
  if(in != nn->numin || out != nn->numout) {
    ulog(ULOG_ERROR, "nn_solve: I/O dimensions are incompatible.%t"
         "NN dimension = (%d x %d)%tDATASET dimension = (%d x %d).",
         nn->numin, nn->numout, in, out);
    return(1);
  }

  /* Check that linknum is sane. */
  if(linknum >= nn->numlinks) {
    ulog(ULOG_ERROR, "nn_solve: linknum is out of range (%d >= %d).",
         linknum, nn->numlinks);
    return(1);
  }

  /* Now check that this is a linear link and that the output
   * activation is in fact linear in it. 
   */
  nl = nn->numlayers;
  link = nn->links[linknum];
  src = link->source->layer;
  nbasis = src->sz;

  if(link->dest->layer->idl != (int)nl - 1) {
    ulog(ULOG_ERROR, "nn_solve: link is not directly connected to the output.");
    return(1);
  }    
  nf = nn_find_netfunc("l");
  if(link->nfunc->forward != nf->forward) {
    ulog(ULOG_ERROR, "nn_solve: link to solve must be linear.");
    return(2);
  }
  af = nn_find_actfunc("none");
  if(link->dest->layer->slabs[0].afunc->func != af->func) {
    ulog(ULOG_ERROR, "nn_solve: output layer must be linear.");
    return(3);
  }    

  /* Compute linear weights w.r.t. LMS.  The overall strategy is to
   * zero out the weights, do a forward call, the solve for the
   * residuals.
   */

  for(i = 0; i < out; i++) {
    for(j = 0; j < nbasis; j++)
      link->u[i][j] = 0;
    link->a[i] = 0;
  }

  /* Get some space...  */
  AtA = allocate_array(2, sizeof(double), nbasis + 1, nbasis + 1);
  Atb = allocate_array(2, sizeof(double), out, nbasis + 1);

  /* Zero out AtA and Atb. */
  for(i = 0; i < nbasis + 1; i++)
    for(j = 0; j < nbasis + 1; j++)
      AtA[i][j] = 0.0;
  for(i = 0; i < out; i++)
    for(j = 0; j < nbasis + 1; j++)
      Atb[i][j] = 0.0;


  /* Compute AtA and Atb.  For each pattern, k, compute the cumulative
   * sum of AtA[i][j] which involves n seperate feedforward calls.
   * For Atb we must do it for each output.
   */
  for(k = 0; k < n; k++) {
    x = dataset_x(set, k);
    y = dataset_y(set, k);
    for(i = 0; i < nn->numin; i++)
      if(x[i] != x[i])
	continue;
    nn_forward(nn, x);
    for(i = 0; i < nbasis + 1; i++) {
      for(j = 0; j < nbasis + 1; j++) {
	if(i < nbasis && j < nbasis)
	  AtA[i][j] += src->y[i] * src->y[j];
	else if(i < nbasis && j == nbasis)
	  AtA[i][j] += src->y[i];
	else if(i == nbasis && j < nbasis)
	  AtA[i][j] += src->y[j];
	else
	  AtA[i][j] += 1.0;
      }
      for(j = 0; j < out; j++)
	if(y[j] == y[j]) {
	  if(i < nbasis)
	    Atb[j][i] += src->y[i] * (y[j] - nn->y[j]);
	  else
	    Atb[j][i] += (y[j] - nn->y[j]);
	}
    }
  }

  /* Invert AtA to get (A^t * A)^{-1}. */
  spinv(AtA, AtA, nbasis + 1);

  /* For each output, calculate AtAiAtb to get the LMS weights. */
  for(l = 0; l < out; l++) {
    for(i = 0; i < nbasis + 1; i++) {
      sum = 0.0;
      for(j = 0; j < nbasis + 1; j++)
	sum += AtA[i][j] * Atb[l][j];
      if(i < nbasis) link->u[l][i] = sum;
      else link->a[l] = sum;
    }
  }

  deallocate_array(AtA);
  deallocate_array(Atb);

  return(0);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

int nn_solve(NN *nn, DATASET *set, unsigned linknum)
{
  return(nn_solve_internal(nn, set, linknum, NULL));
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

int nn_weighted_solve(NN *nn, DATASET *set, unsigned linknum,
		      double *weights)
{
  return(nn_solve_internal(nn, set, linknum, weights));
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static NN *nn_create_rbf_internal(unsigned nbasis, double var,
				  DATASET *set, int normed, int randdist)
{
  unsigned in, out, n, i, j, k;
  double *x, dist, bestd;
  DSM_MATRIX *dsmmtx;
  DATASET *centers;
  NN *rbf;

  /* Create the RBFN. */

  n = dataset_size(set);
  in = dataset_x_size(set);
  out = dataset_y_size(set);
  if(normed) {
    rbf = nn_create("%d %d %d %d", in, nbasis, nbasis, out);
    nn_set_actfunc(rbf, 1, 0, "exp(-x)");
    nn_set_actfunc(rbf, 2, 0, "linear");
    nn_set_actfunc(rbf, 3, 0, "linear");
    nn_link(rbf, "0 -e-> 1");
    nn_link(rbf, "1 -n-> 2");
    nn_link(rbf, "2 -l-> 3");
    nn_lock_link(rbf, 0);
    nn_lock_link(rbf, 1);
  }
  else {
    rbf = nn_create("%d %d %d", in, nbasis, out);
    nn_set_actfunc(rbf, 1, 0, "exp(-x)");
    nn_set_actfunc(rbf, 2, 0, "linear");
    nn_link(rbf, "0 -e-> 1");
    nn_link(rbf, "1 -l-> 2");
    nn_lock_link(rbf, 0);
  }
  
  /* Cluster or randomly set the centers. */
  
  if(randdist) {
    unsigned *used, l, unique;
    used = allocate_array(1, sizeof(unsigned), nbasis);
    for(i = 0; i < nbasis; i++) {
      do {
	unique = 1;
	k = random() % n;
	for(l = 0; l < i; l++)
	  if(k == used[l]) {
	    unique = 0;
	    break;
	  }
	if(unique) used[i] = k;
      } while(!unique);
      x = dataset_x(set, k);
      for(j = 0; j < in; j++)
	rbf->links[0]->u[i][j] = x[j];
      rbf->links[0]->a[i] = var;
    }
    deallocate_array(used);
  }
  else {
    if(nn_kmeans_online == 0)
      centers = kmeans(set, nbasis, nn_kmeans_minfrac,
		       nn_kmeans_maxiters, nn_kmeans_clusinit);
    else
      centers = kmeans_online(set, nbasis, nn_kmeans_maxiters,
			      nn_kmeans_clusinit);
    for(i = 0; i < nbasis; i++) {
      x = dataset_x(centers, i);
      for(j = 0; j < in; j++)
	rbf->links[0]->u[i][j] = x[j];
      rbf->links[0]->a[i] = var;
    }
    dsmmtx = dataset_destroy(centers);
    xfree(dsmmtx->x);
    dsm_destroy_matrix(dsmmtx);
  }

  /* Set variances with heuristic, if desired. */
  
  if(var <= 0.0) {
    var = (var != 0) ? fabs(var) : 1;		   
    for(i = 0; i < nbasis; i++) {
      bestd = 1e20;
      for(j = 0; j < nbasis; j++) {
	if(i == j) continue;
	dist = 0;
	for(k = 0; k < in; k++)
	  dist += (rbf->links[0]->u[i][k] - rbf->links[0]->u[j][k])
	    * (rbf->links[0]->u[i][k] - rbf->links[0]->u[j][k]);
	dist = sqrt(dist);
	if(dist < bestd)
	  bestd = dist;
      }
      rbf->links[0]->a[i] = bestd * var;
    }
  }


  /* Compute linear weights w.r.t. LMS */

  nn_solve(rbf, set, rbf->numlinks - 1);
  return(rbf);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

NN *nn_create_rbf(unsigned nbasis, double var, DATASET *set)
{
  return(nn_create_rbf_internal(nbasis, var, set, nn_rbf_basis_normalized,
				nn_rbf_centers_random));
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

NN *nn_create_smlp_internal(unsigned nbasis, double var, DATASET *set,
			    int randdist)
{
  unsigned in, out, n, i, j, k;
  double *x, dist, bestd, sqrt2, *c, *w, tmp;
  DSM_MATRIX *dsmmtx = NULL;
  DATASET *centers;
  NN *nn;

  /* Create the SMLP. */

  n = dataset_size(set);
  in = dataset_x_size(set);
  out = dataset_y_size(set);

  nn = nn_create("%d %d %d", in, nbasis, out);
  nn_set_actfunc(nn, 1, 0, "tanh");
  nn_set_actfunc(nn, 2, 0, "linear");
  nn_link(nn, "0 -d-> 1");
  nn_link(nn, "1 -l-> 2");
  
  /* Cluster the the centers. */
  
  if(randdist) {
    unsigned *used, l, unique;
    used = allocate_array(1, sizeof(unsigned), nbasis);    
    c = allocate_array(1, sizeof(double), nbasis * in);
    for(j = 0; j < nbasis; j++) {
      do {
	unique = 1;
	i = random() % n;
	for(l = 0; l < j; l++)
	  if(i == used[l]) {
	    unique = 0;
	    break;
	  }
	if(unique) used[j] = i;
      } while(!unique);
      x = dataset_x(set, i);
      for(k = 0; k < in; k++)
	c[j * in + k] = x[k];
    }
    deallocate_array(used);
  }
  else {
    if(nn_kmeans_online == 0)
      centers = kmeans(set, nbasis, nn_kmeans_minfrac,
		       nn_kmeans_maxiters, nn_kmeans_clusinit);
    else
      centers = kmeans_online(set, nbasis, nn_kmeans_maxiters,
			      nn_kmeans_clusinit);

    dsmmtx = dataset_destroy(centers);
    c = dsmmtx->x;
  }

  w = allocate_array(1, sizeof(double), nbasis);

  /* I am getting cozy with the DATASET returned.  Here, c[i * in + j]
   * refers to the jth component of the ith center.
   */

  sqrt2 = sqrt(2.0);

  /* Compute the widths */
  for(i = 0; i < nbasis; i++)
    w[i] = var;
  if(var <= 0.0) {
    var = (var != 0) ? fabs(var) : 1;		   
    for(i = 0; i < nbasis; i++) {
      bestd = 1e20;
      for(j = 0; j < nbasis; j++) {
	if(i == j) continue;
	dist = 0;
	for(k = 0; k < in; k++) {
	  tmp = c[i * in + k] - c[j * in + k];
	  dist += tmp * tmp;
	}
	dist = sqrt(dist);
	if(dist < bestd)
	  bestd = dist;
      }
      w[i] = sqrt2 * bestd * var;
    }
  }
  
  /* Fill the diagonal terms, v, the dot terms, u, and the bias terms, a */
  for(i = 0; i < nbasis; i++) {
    tmp = 0;
    for(j = 0; j < in; j++) {
      nn->links[0]->v[i][j] = 1 / (sqrt2 * w[i] * w[i]);
      nn->links[0]->u[i][j] = -2 * c[i * in + j] / (sqrt2 * w[i] * w[i]);
      tmp += c[i * in + j] * c[i * in + j];
    }
    nn->links[0]->a[i] = tmp / (sqrt2 * w[i] * w[i]);
  }

  if(randdist) {
    deallocate_array(c);
  }
  else {
    xfree(dsmmtx->x);
    dsm_destroy_matrix(dsmmtx);
  }
  deallocate_array(w);

  /* Compute linear weights w.r.t. LMS */
  nn_solve(nn, set, nn->numlinks - 1);

  return(nn);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

NN *nn_create_smlp(unsigned nbasis, double var, DATASET *set)
{
  return(nn_create_smlp_internal(nbasis, var, set, nn_smlp_centers_random));
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

