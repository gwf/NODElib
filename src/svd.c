
/* Copyright (c) 1995 by G. W. Flake. */

#include <math.h>
#include <stdio.h>

#include "nodelib/misc.h"
#include "nodelib/dataset.h"
#include "nodelib/ulog.h"

int svd_original = 0;

#ifdef NL_EXTENSIONS
#undef LAPACK_SVD
#define LAPACK_SVD 1
#endif

#ifdef LAPACK
#undef LAPACK_SVD
#define LAPACK_SVD 1
#endif

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* Original comments from Bryant Marks:
 
   This SVD routine is based on pgs 30-48 of "Compact Numerical Methods
   for Computers" by J.C. Nash (1990), used to compute the pseudoinverse.
   Modifications include:
        Translation from Pascal to ANSI C.
        Array indexing from 0 rather than 1.
        Float replaced by double everywhere.
        Support for the Matrix structure.
        I changed the array indexing so that the matricies (float [][])
           could be replaced be a single list (double *) for more
           efficient communication with Mathematica.

  From: bryant@sioux.stanford.edu (Bryant Marks)
  >
  > A couple of things to note: A needs to have twice as much room
  > allocated for it (2*n + 2*m) since the W in the svd function requires
  > this (part of a rotation algorithm).  After the routine has run W
  > contains two maticies of the decomposition A = USV'.  The first nRow
  > rows contain the product US and the next nCol rows contain V (not V').
  > Z is equal to the vector of the sqares of the diagonal elements of S. */

/* Comments from GWF: The note above is not strictly correct.  To compute
   the SVD of an (m x n) matrix, W must be ((m + n) x n) in size.

   12-26-96: I slightly rewrote things to include V so that the interface
   could at least be close to a lapack-like routine. 

   01-02-97: Rewrote it so that U,S,V was actually returned.  V is now optional. */

#define TOLERANCE 1.0e-12

static void svd_simple_internal(double *U, double *S, double *V,
				int nRow, int nCol)
{
  int i, j, k, EstColRank, RotCount, SweepCount, slimit;
  double eps, e2, tol, vt, p, x0, y0, q, r, c0, s0, d1, d2;

  eps = TOLERANCE;
  slimit = nCol / 4;
  if (slimit < 6.0)
    slimit = 6;
  SweepCount = 0;
  e2 = 10.0 * nRow * eps * eps;
  tol = eps * .1;
  EstColRank = nCol;
  if(V)
    for (i = 0; i < nCol; i++)
      for (j = 0; j < nCol; j++) {
	V[nCol * i + j] = 0.0;
	V[nCol * i + i] = 1.0;
      }
  RotCount = EstColRank * (EstColRank - 1) / 2;
  while (RotCount != 0 && SweepCount <= slimit) {
    RotCount = EstColRank * (EstColRank - 1) / 2;
    SweepCount++;
    for (j = 0; j < EstColRank - 1; j++) {
      for (k = j + 1; k < EstColRank; k++) {
	p = q = r = 0.0;
	for (i = 0; i < nRow; i++) {
	  x0 = U[nCol * i + j];
	  y0 = U[nCol * i + k];
	  p += x0 * y0;
	  q += x0 * x0;
	  r += y0 * y0;
	}
	S[j] = q;
	S[k] = r;
	if (q >= r) {
	  if (q <= e2 * S[0] || fabs(p) <= tol * q)
	    RotCount--;
	  else {
	    p /= q;
	    r = 1 - r / q;
	    vt = sqrt(4 * p * p + r * r);
	    c0 = sqrt(fabs(.5 * (1 + r / vt)));
	    s0 = p / (vt * c0);
	    for (i = 0; i < nRow; i++) {
	      d1 = U[nCol * i + j];
	      d2 = U[nCol * i + k];
	      U[nCol * i + j] = d1 * c0 + d2 * s0;
	      U[nCol * i + k] = -d1 * s0 + d2 * c0;
	    }
	    if(V)
	      for (i = 0; i < nCol; i++) {
		d1 = V[nCol * i + j];
		d2 = V[nCol * i + k];
		V[nCol * i + j] = d1 * c0 + d2 * s0;
		V[nCol * i + k] = -d1 * s0 + d2 * c0;
	      }
	  }
	}
	else {
	  p /= r;
	  q = q / r - 1;
	  vt = sqrt(4 * p * p + q * q);
	  s0 = sqrt(fabs(.5 * (1 - q / vt)));
	  if (p < 0)
	    s0 = -s0;
	  c0 = p / (vt * s0);
	  for (i = 0; i < nRow; i++) {
	    d1 = U[nCol * i + j];
	    d2 = U[nCol * i + k];
	    U[nCol * i + j] = d1 * c0 + d2 * s0;
	    U[nCol * i + k] = -d1 * s0 + d2 * c0;
	  }
	  if(V)
	    for (i = 0; i < nCol; i++) {
	      d1 = V[nCol * i + j];
	      d2 = V[nCol * i + k];
	      V[nCol * i + j] = d1 * c0 + d2 * s0;
	      V[nCol * i + k] = -d1 * s0 + d2 * c0;
	    }
	}
      }
    }
    while (EstColRank >= 3 && S[(EstColRank - 1)] <= S[0] * tol + tol * tol)
      EstColRank--;
  }
  for(i = 0; i < nCol; i++)
    S[i] = sqrt(S[i]);
  for(i = 0; i < nCol; i++)
    for(j = 0; j < nRow; j++)
      U[nCol * j + i] = U[nCol * j + i] / S[i];
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifdef MAX
#undef MAX
#endif
#ifdef MIN
#undef MIN
#endif
#define MAX(a,b) (((a) > (b)) ? (a) : (b))
#define MIN(a,b) (((a) < (b)) ? (a) : (b))

void svd(double *U, double *S, double *V, unsigned nRow, unsigned nCol)
{
#ifndef LAPACK_SVD

  svd_simple_internal(U, S, V, nRow, nCol);
  
#else /* LAPACK_SVD */

  int dgesvd_(char *jobu, char *jobvt, long *m, long *n, double *a,
	      long *lda, double *s, double *u, long *ldu, double *vt,
	      long *ldvt, double *work, long *lwork, long *info,
	      int jobu_len, int jobvt_len);

  char jobu, jobvt;
  long m, n, lda, ldu, ldvt, lwork, info, i, j;
  double *a, *s, *u, *vt, *work, tmp;

  if(svd_original) {
    svd_simple_internal(U, S, V, nRow, nCol);
    return;
  }

  jobu = 'O';
  if(V)
    jobvt = 'A';
  else
    jobvt = 'N';
  m = nRow;
  n = nCol;
  lda = m;
  ldu = m;
  ldvt = n;
  lwork = MAX(3 * MIN(m,n) + MAX(m,n), 5 * MIN(m, n) - 4);

  a = U;
  u = U;
  for(i = 0; i < nRow - 1; i++)
    for(j = i + 1; j < nCol; j++) {
      tmp = U[i * nCol + j];
      U[i * nCol + j] = U[j * nCol + i];
      U[j * nCol + i] = tmp;
    }

  s = S;
  vt = V;
  work = allocate_array(1, sizeof(double), lwork);

  dgesvd_(&jobu, &jobvt, &m, &n, a, &lda, s, NULL, &ldu, vt, &ldvt,
	  work, &lwork, &info, 1, 1);

  for(i = 0; i < nRow; i++)
    for(j = i; j < nCol; j++) {
      tmp = U[i * nCol + j];
      U[i * nCol + j] = U[j * nCol + i];
      U[j * nCol + i] = tmp;
    }

  deallocate_array(work);
  
#endif /* LAPACK_SVD */
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void spinv(double **A, double **Ainv, unsigned n)
{
  unsigned i, j, k;
  double **U, *S, smin, smax, sum;

  U = allocate_array(2, sizeof(double), n, n);
  S = allocate_array(1, sizeof(double), n);
  for(i = 0; i < n; i++)
    for(j = 0; j < n; j++)
      U[i][j] = A[i][j];
  svd(&U[0][0], S, NULL, n, n);

  /* Zero out tiny values */
  smax = 0.0;
  for(i = 0; i < n; i++)
    if(S[i] > smax)
      smax = S[i];
  smin = fabs(sqrt(smax) * n * 2.2204e-16);
  for(i = 0; i < n; i++)
    if(fabs(S[i]) < smin)
      S[i] = 0.0;
    else
      S[i] = 1.0 / S[i];

  for(i = 0; i < n; i++)
    for(j = 0; j < n; j++) {
      sum = 0;
      for(k = 0; k < n; k++)
	sum += U[i][k] * U[j][k] * S[k];
      Ainv[i][j] = sum;
    }
  
  deallocate_array(U);
  deallocate_array(S);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void pinv(double **A, double **Ainv, unsigned n)
{
  unsigned i, j, k;
  double **U, **V, *S, smin, smax, sum;

  U = allocate_array(2, sizeof(double), n, n);
  V = allocate_array(2, sizeof(double), n, n);
  S = allocate_array(1, sizeof(double), n);
  for(i = 0; i < n; i++)
    for(j = 0; j < n; j++)
      U[i][j] = A[i][j];
  svd(&U[0][0], S, &V[0][0], n, n);

  /* Zero out tiny values */
  smax = 0.0;
  for(i = 0; i < n; i++)
    if(S[i] > smax)
      smax = S[i];
  smin = fabs(sqrt(smax) * n * 2.2204e-16);
  for(i = 0; i < n; i++)
    if(fabs(S[i]) < smin)
      S[i] = 0.0;
    else
      S[i] = 1.0 / S[i];

  for(i = 0; i < n; i++)
    for(j = 0; j < n; j++) {
      sum = 0;
      for(k = 0; k < n; k++)
	sum += V[i][k] * U[j][k] * S[k];
      Ainv[i][j] = sum;
    }
  
  deallocate_array(U);
  deallocate_array(V);
  deallocate_array(S);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

int pca(DATASET *data, unsigned m, double ***D, double **SV, double **M)
{
  unsigned i, j, k, n, p, smaxi, num;
  double **U, *S, *x, *mean, *eigenval, **pcas, smax;

  n = dataset_x_size(data);
  if(m > n) {
    ulog(ULOG_ERROR, "pca: more requested components than dimensions "
	 "(%d > %d).", m, n);
    return(0);
  }

  p = dataset_size(data);
  U = allocate_array(2, sizeof(double), n, n);
  S = allocate_array(1, sizeof(double), n);
  mean = allocate_array(1, sizeof(double), n);
  
  for(i = 0; i < n; i++) {
    mean[i] = 0;
    for(j = 0; j < n; j++)
      U[i][j] = 0;
  }
  for(i = 0; i < p; i++) {
    x = dataset_x(data, i);
    for(j = 0; j < n; j++)
      mean[j] += x[j];
  }
  for(j = 0; j < n; j++)
    mean[j] /= p;

  for(i = 0; i < p; i++) {
    x = dataset_x(data, i);
    for(j = 0; j < n; j++)
      for(k = 0; k < n; k++)
	U[j][k] += (x[j] - mean[j]) * (x[k] - mean[k]);
  }
  for(j = 0; j < n; j++)
    for(k = 0; k < n; k++)
      U[j][k] /= p;

  svd(&U[0][0], S, NULL, n, n);

  eigenval = allocate_array(1, sizeof(double), m);
  pcas = allocate_array(2, sizeof(double), m, n);
  for(i = 0; i < m; i++)
    for(j = 0; j < n; j++)
      pcas[i][j] = 0;

  num = m;
  for(i = 0; i < m; i++) {
    smax = 0; smaxi = 0;
    for(j = 0; j < n; j++)
      if(S[j] > smax) {
	smax = S[j];  smaxi = j;
      }
    if(smax > 0) {
      for(j = 0; j < n; j++)
	pcas[i][j] = U[j][smaxi];
      eigenval[i] = S[smaxi];
      S[smaxi] = 0;
    }
    else {
      num = i;
      break;
    }
  }

  deallocate_array(U);
  deallocate_array(S);

  *D = pcas;
  *SV = eigenval;
  *M = mean;
  return(num);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
