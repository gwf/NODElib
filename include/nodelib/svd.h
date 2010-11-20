
/* Copyright (c) 1996-97 by G. W. Flake.
 *
 * NAME
 *   svd.h - singular value decomposition and friends
 * SYNOPSIS
 *   Included here is a basic SVD call and some basic applications of
 *   the SVD, such as pseudo matrix inversion and principal component
 *   analysis. 
 * DESCRIPTION
 *   The heart of this module is the SVD routine.  The supplied SVD
 *   function is very small and simple but is by no means optimal.
 *   If this module is compiled with either a -DSL_EXTENSIONS or a
 *   -DLAPACK command line option and linked with either "-llapack
 *   -lblas -lf2c" or  "-lsle", then the LAPACK SVD will be used
 *   which is vastly superior to the enclosed version.  Also, at
 *   runtime, one can switch back to the original SVD routine by
 *   setting svd_original to a nonzero value. The proper way to
 *   declare svd_original is:
 *
 *   \ \ extern int svd_original;
 * BUGS
 *   The original SVD has a tolerance defined in the source code file
 *   \bf{svd.c.}  You may wish to change this.
 * AUTHOR
 *   Gary William Flake (\url{\bf{gary.flake@usa.net}}{mailto:gary.flake@usa.net}).
 */


#ifndef __SVD_H__
#define __SVD_H__

#include "nodelib/dataset.h"
#include "nodelib/etc/version.h"
#include "nodelib/etc/options.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* Computes the singular value decomposition of the matrix, \em{U.}
   The contents of U are replaced such that \em{A = U*S*V'} where
   \em{A} represents the initial value of \em{U.}  \em{S} is
   understood to have only room for \em{ncol} elements.  The matrix
   \em{V} may be NULL, in which case, no values are returned for
   \em{V.} */

void svd(double *U, double *S, double *V, unsigned nrow, unsigned ncol);


/* Matrix pseudo-inversion, based on a singular value
   decomposition. The two matrices may point to the same memory. */

void pinv(double **A, double **Ainv, unsigned n);


/* Matrix pseudo-inversion for symmetric positive definite matrices
   based on a singular value decomposition.  The two matrices may
   point to the same memory. */

void spinv(double **A, double **Ainv, unsigned n);

/* Computes the \em{m} principal components of the \em{x} portion of the
   data contained in \em{data.}  The last three arguments are pointers
   to arrays of doubles that will be filled with useful values.  You do
   \bf{not} need to allocate space for these arrays, as it is done for you.
   After the call, \em{D} will point to an (\em{m} x \em{n}) matrix
   that contains the \em{m} largest principle components, sorted from
   largest to smallest, \em{S} will point to a vector that contains
   the associated eigenvalues, and \em{M} will point to a vector with
   the mean of the data (which you will need because PCA is properly
   defined on zero mean data.)  The return value is the number of nonzero
   components actually returned, so if all goes well, it should be equal
   to \em{m.} 

   It is an error for \em{m} to be larger than the dimensionality of
   the input data.  The space returned in \em{D,} \em{S,} and \em{M}
   must be deallocated with \bf{deallocate_array().} */

int pca(DATASET *data, unsigned m, double ***D, double **S, double **M);

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __SVD_H__ */
