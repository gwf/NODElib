/* dlasq1.f -- translated by f2c (version 19950808).
   You must link the resulting object file with the libraries:
	-lf2c -lm   (in that order)
*/

#include "f2c.h"

/* Table of constant values */

static doublereal c_b8 = .125;
static integer c__1 = 1;
static integer c__0 = 0;

/* Subroutine */ int dlasq1_(n, d__, e, work, info)
integer *n;
doublereal *d__, *e, *work;
integer *info;
{
    /* System generated locals */
    integer i__1, i__2;
    doublereal d__1, d__2, d__3, d__4;

    /* Builtin functions */
    double pow_dd(), sqrt();

    /* Local variables */
    static integer kend, ierr;
    extern /* Subroutine */ int dlas2_();
    static integer i__, j, m;
    static doublereal sfmin, sigmn;
    extern /* Subroutine */ int dcopy_();
    static doublereal sigmx;
    extern /* Subroutine */ int dlasq2_();
    static doublereal small2;
    static integer ke;
    static doublereal dm;
    extern doublereal dlamch_();
    static doublereal dx;
    extern /* Subroutine */ int dlascl_();
    static integer ny;
    extern /* Subroutine */ int xerbla_(), dlasrt_();
    static doublereal thresh, tolmul;
    static logical restrt;
    static doublereal scl, eps, tol, sig1, sig2, tol2;


/*  -- LAPACK routine (version 2.0) -- */
/*     Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd., */
/*     Courant Institute, Argonne National Lab, and Rice University */
/*     September 30, 1994 */

/*     .. Scalar Arguments .. */
/*     .. */
/*     .. Array Arguments .. */
/*     .. */

/*     Purpose */
/*     ======= */

/*     DLASQ1 computes the singular values of a real N-by-N bidiagonal */
/*     matrix with diagonal D and off-diagonal E. The singular values are 
*/
/*     computed to high relative accuracy, barring over/underflow or */
/*     denormalization. The algorithm is described in */

/*     "Accurate singular values and differential qd algorithms," by */
/*     K. V. Fernando and B. N. Parlett, */
/*     Numer. Math., Vol-67, No. 2, pp. 191-230,1994. */

/*     See also */
/*     "Implementation of differential qd algorithms," by */
/*     K. V. Fernando and B. N. Parlett, Technical Report, */
/*     Department of Mathematics, University of California at Berkeley, */
/*     1994 (Under preparation). */

/*     Arguments */
/*     ========= */

/*  N       (input) INTEGER */
/*          The number of rows and columns in the matrix. N >= 0. */

/*  D       (input/output) DOUBLE PRECISION array, dimension (N) */
/*          On entry, D contains the diagonal elements of the */
/*          bidiagonal matrix whose SVD is desired. On normal exit, */
/*          D contains the singular values in decreasing order. */

/*  E       (input/output) DOUBLE PRECISION array, dimension (N) */
/*          On entry, elements E(1:N-1) contain the off-diagonal elements 
*/
/*          of the bidiagonal matrix whose SVD is desired. */
/*          On exit, E is overwritten. */

/*  WORK    (workspace) DOUBLE PRECISION array, dimension (2*N) */

/*  INFO    (output) INTEGER */
/*          = 0:  successful exit */
/*          < 0:  if INFO = -i, the i-th argument had an illegal value */
/*          > 0:  if INFO = i, the algorithm did not converge;  i */
/*                specifies how many superdiagonals did not converge. */

/*  ===================================================================== 
*/

/*     .. Parameters .. */
/*     .. */
/*     .. Local Scalars .. */
/*     .. */
/*     .. External Functions .. */
/*     .. */
/*     .. External Subroutines .. */
/*     .. */
/*     .. Intrinsic Functions .. */
/*     .. */
/*     .. Executable Statements .. */
    /* Parameter adjustments */
    --work;
    --e;
    --d__;

    /* Function Body */
    *info = 0;
    if (*n < 0) {
	*info = -2;
	i__1 = -(*info);
	xerbla_("DLASQ1", &i__1, 6L);
	return 0;
    } else if (*n == 0) {
	return 0;
    } else if (*n == 1) {
	d__[1] = abs(d__[1]);
	return 0;
    } else if (*n == 2) {
	dlas2_(&d__[1], &e[1], &d__[2], &sigmn, &sigmx);
	d__[1] = sigmx;
	d__[2] = sigmn;
	return 0;
    }

/*     Estimate the largest singular value */

    sigmx = 0.;
    i__1 = *n - 1;
    for (i__ = 1; i__ <= i__1; ++i__) {
/* Computing MAX */
	d__2 = sigmx, d__3 = (d__1 = e[i__], abs(d__1));
	sigmx = max(d__2,d__3);
/* L10: */
    }

/*     Early return if sigmx is zero (matrix is already diagonal) */

    if (sigmx == 0.) {
	goto L70;
    }

    i__1 = *n;
    for (i__ = 1; i__ <= i__1; ++i__) {
	d__[i__] = (d__1 = d__[i__], abs(d__1));
/* Computing MAX */
	d__1 = sigmx, d__2 = d__[i__];
	sigmx = max(d__1,d__2);
/* L20: */
    }

/*     Get machine parameters */

    eps = dlamch_("EPSILON", 7L);
    sfmin = dlamch_("SAFE MINIMUM", 12L);

/*     Compute singular values to relative accuracy TOL */
/*     It is assumed that tol**2 does not underflow. */

/* Computing MAX */
/* Computing MIN */
    d__3 = 100., d__4 = pow_dd(&eps, &c_b8);
    d__1 = 10., d__2 = min(d__3,d__4);
    tolmul = max(d__1,d__2);
    tol = tolmul * eps;
/* Computing 2nd power */
    d__1 = tol;
    tol2 = d__1 * d__1;

    thresh = sigmx * sqrt(sfmin) * tol;

/*     Scale matrix so the square of the largest element is */
/*     1 / ( 256 * SFMIN ) */

    scl = sqrt(1. / (sfmin * 256.));
/* Computing 2nd power */
    d__1 = tolmul;
    small2 = 1. / (d__1 * d__1 * 256.);
    dcopy_(n, &d__[1], &c__1, &work[1], &c__1);
    i__1 = *n - 1;
    dcopy_(&i__1, &e[1], &c__1, &work[*n + 1], &c__1);
    dlascl_("G", &c__0, &c__0, &sigmx, &scl, n, &c__1, &work[1], n, &ierr, 1L)
	    ;
    i__1 = *n - 1;
    i__2 = *n - 1;
    dlascl_("G", &c__0, &c__0, &sigmx, &scl, &i__1, &c__1, &work[*n + 1], &
	    i__2, &ierr, 1L);

/*     Square D and E (the input for the qd algorithm) */

    i__1 = (*n << 1) - 1;
    for (j = 1; j <= i__1; ++j) {
/* Computing 2nd power */
	d__1 = work[j];
	work[j] = d__1 * d__1;
/* L30: */
    }

/*     Apply qd algorithm */

    m = 0;
    e[*n] = 0.;
    dx = work[1];
    dm = dx;
    ke = 0;
    restrt = FALSE_;
    i__1 = *n;
    for (i__ = 1; i__ <= i__1; ++i__) {
	if ((d__1 = e[i__], abs(d__1)) <= thresh || work[*n + i__] <= tol2 * (
		dm / (doublereal) (i__ - m))) {
	    ny = i__ - m;
	    if (ny == 1) {
		goto L50;
	    } else if (ny == 2) {
		dlas2_(&d__[m + 1], &e[m + 1], &d__[m + 2], &sig1, &sig2);
		d__[m + 1] = sig1;
		d__[m + 2] = sig2;
	    } else {
		kend = ke + 1 - m;
		dlasq2_(&ny, &d__[m + 1], &e[m + 1], &work[m + 1], &work[m + *
			n + 1], &eps, &tol2, &small2, &dm, &kend, info);

/*                 Return, INFO = number of unconverged superd
iagonals */

		if (*info != 0) {
		    *info += i__;
		    return 0;
		}

/*                 Undo scaling */

		i__2 = m + ny;
		for (j = m + 1; j <= i__2; ++j) {
		    d__[j] = sqrt(d__[j]);
/* L40: */
		}
		dlascl_("G", &c__0, &c__0, &scl, &sigmx, &ny, &c__1, &d__[m + 
			1], &ny, &ierr, 1L);
	    }
L50:
	    m = i__;
	    if (i__ != *n) {
		dx = work[i__ + 1];
		dm = dx;
		ke = i__;
		restrt = TRUE_;
	    }
	}
	if (i__ != *n && ! restrt) {
	    dx = work[i__ + 1] * (dx / (dx + work[*n + i__]));
	    if (dm > dx) {
		dm = dx;
		ke = i__;
	    }
	}
	restrt = FALSE_;
/* L60: */
    }
    kend = ke + 1;

/*     Sort the singular values into decreasing order */

L70:
    dlasrt_("D", n, &d__[1], info, 1L);
    return 0;

/*     End of DLASQ1 */

} /* dlasq1_ */

