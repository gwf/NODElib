/* dlasq2.f -- translated by f2c (version 19950808).
   You must link the resulting object file with the libraries:
	-lf2c -lm   (in that order)
*/

#include "f2c.h"

/* Subroutine */ int dlasq2_(m, q, e, qq, ee, eps, tol2, small2, sup, kend, 
	info)
integer *m;
doublereal *q, *e, *qq, *ee, *eps, *tol2, *small2, *sup;
integer *kend, *info;
{
    /* System generated locals */
    doublereal d__1, d__2, d__3, d__4;

    /* Builtin functions */
    double sqrt();
    integer i_dnnt();

    /* Local variables */
    static doublereal xinf;
    static integer n;
    static doublereal sigma, qemax;
    static integer iconv;
    extern /* Subroutine */ int dlasq3_();
    static integer iphase;
    static doublereal xx, yy;
    static integer off, isp, off1;


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

/*     DLASQ2 computes the singular values of a real N-by-N unreduced */
/*     bidiagonal matrix with squared diagonal elements in Q and */
/*     squared off-diagonal elements in E. The singular values are */
/*     computed to relative accuracy TOL, barring over/underflow or */
/*     denormalization. */

/*     Arguments */
/*     ========= */

/*  M       (input) INTEGER */
/*          The number of rows and columns in the matrix. M >= 0. */

/*  Q       (output) DOUBLE PRECISION array, dimension (M) */
/*          On normal exit, contains the squared singular values. */

/*  E       (workspace) DOUBLE PRECISION array, dimension (M) */

/*  QQ      (input/output) DOUBLE PRECISION array, dimension (M) */
/*          On entry, QQ contains the squared diagonal elements of the */
/*          bidiagonal matrix whose SVD is desired. */
/*          On exit, QQ is overwritten. */

/*  EE      (input/output) DOUBLE PRECISION array, dimension (M) */
/*          On entry, EE(1:N-1) contains the squared off-diagonal */
/*          elements of the bidiagonal matrix whose SVD is desired. */
/*          On exit, EE is overwritten. */

/*  EPS     (input) DOUBLE PRECISION */
/*          Machine epsilon. */

/*  TOL2    (input) DOUBLE PRECISION */
/*          Desired relative accuracy of computed eigenvalues */
/*          as defined in DLASQ1. */

/*  SMALL2  (input) DOUBLE PRECISION */
/*          A threshold value as defined in DLASQ1. */

/*  SUP     (input/output) DOUBLE PRECISION */
/*          Upper bound for the smallest eigenvalue. */

/*  KEND    (input/output) INTEGER */
/*          Index where minimum d occurs. */

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
/*     .. External Subroutines .. */
/*     .. */
/*     .. Intrinsic Functions .. */
/*     .. */
/*     .. Executable Statements .. */
    /* Parameter adjustments */
    --ee;
    --qq;
    --e;
    --q;

    /* Function Body */
    n = *m;

/*     Set the default maximum number of iterations */

    off = 0;
    off1 = off + 1;
    sigma = 0.;
    xinf = 0.;
    iconv = 0;
    iphase = 2;

/*     Try deflation at the bottom */

/*     1x1 deflation */

L10:
    if (n <= 2) {
	goto L20;
    }
/* Computing MAX */
    d__1 = qq[n], d__1 = max(d__1,xinf);
    if (ee[n - 1] <= max(d__1,*small2) * *tol2) {
	q[n] = qq[n];
	--n;
	if (*kend > n) {
	    *kend = n;
	}
/* Computing MIN */
	d__1 = qq[n], d__2 = qq[n - 1];
	*sup = min(d__1,d__2);
	goto L10;
    }

/*     2x2 deflation */

/* Computing MAX */
    d__1 = max(xinf,*small2), d__2 = qq[n] / (qq[n] + ee[n - 1] + qq[n - 1]) *
	     qq[n - 1];
    if (ee[n - 2] <= max(d__1,d__2) * *tol2) {
/* Computing MAX */
	d__1 = qq[n], d__2 = qq[n - 1], d__1 = max(d__1,d__2), d__2 = ee[n - 
		1];
	qemax = max(d__1,d__2);
	if (qemax != 0.) {
	    if (qemax == qq[n - 1]) {
/* Computing 2nd power */
		d__1 = (qq[n] - qq[n - 1] + ee[n - 1]) / qemax;
		xx = (qq[n] + qq[n - 1] + ee[n - 1] + qemax * sqrt(d__1 * 
			d__1 + ee[n - 1] * 4. / qemax)) * .5;
	    } else if (qemax == qq[n]) {
/* Computing 2nd power */
		d__1 = (qq[n - 1] - qq[n] + ee[n - 1]) / qemax;
		xx = (qq[n] + qq[n - 1] + ee[n - 1] + qemax * sqrt(d__1 * 
			d__1 + ee[n - 1] * 4. / qemax)) * .5;
	    } else {
/* Computing 2nd power */
		d__1 = (qq[n] - qq[n - 1] + ee[n - 1]) / qemax;
		xx = (qq[n] + qq[n - 1] + ee[n - 1] + qemax * sqrt(d__1 * 
			d__1 + qq[n - 1] * 4. / qemax)) * .5;
	    }
/* Computing MAX */
	    d__1 = qq[n], d__2 = qq[n - 1];
/* Computing MIN */
	    d__3 = qq[n], d__4 = qq[n - 1];
	    yy = max(d__1,d__2) / xx * min(d__3,d__4);
	} else {
	    xx = 0.;
	    yy = 0.;
	}
	q[n - 1] = xx;
	q[n] = yy;
	n += -2;
	if (*kend > n) {
	    *kend = n;
	}
	*sup = qq[n];
	goto L10;
    }

L20:
    if (n == 0) {

/*         The lower branch is finished */

	if (off == 0) {

/*         No upper branch; return to DLASQ1 */

	    return 0;
	} else {

/*         Going back to upper branch */

	    xinf = 0.;
	    if (ee[off] > 0.) {
		isp = i_dnnt(&ee[off]);
		iphase = 1;
	    } else {
		isp = -i_dnnt(&ee[off]);
		iphase = 2;
	    }
	    sigma = e[off];
	    n = off - isp + 1;
	    off1 = isp;
	    off = off1 - 1;
	    if (n <= 2) {
		goto L20;
	    }
	    if (iphase == 1) {
/* Computing MIN */
		d__1 = q[n + off], d__2 = q[n - 1 + off], d__1 = min(d__1,
			d__2), d__2 = q[n - 2 + off];
		*sup = min(d__1,d__2);
	    } else {
/* Computing MIN */
		d__1 = qq[n + off], d__2 = qq[n - 1 + off], d__1 = min(d__1,
			d__2), d__2 = qq[n - 2 + off];
		*sup = min(d__1,d__2);
	    }
	    *kend = 0;
	    iconv = -3;
	}
    } else if (n == 1) {

/*     1x1 Solver */

	if (iphase == 1) {
	    q[off1] += sigma;
	} else {
	    q[off1] = qq[off1] + sigma;
	}
	n = 0;
	goto L20;

/*     2x2 Solver */

    } else if (n == 2) {
	if (iphase == 2) {
/* Computing MAX */
	    d__1 = qq[n + off], d__2 = qq[n - 1 + off], d__1 = max(d__1,d__2),
		     d__2 = ee[n - 1 + off];
	    qemax = max(d__1,d__2);
	    if (qemax != 0.) {
		if (qemax == qq[n - 1 + off]) {
/* Computing 2nd power */
		    d__1 = (qq[n + off] - qq[n - 1 + off] + ee[n - 1 + off]) /
			     qemax;
		    xx = (qq[n + off] + qq[n - 1 + off] + ee[n - 1 + off] + 
			    qemax * sqrt(d__1 * d__1 + ee[off + n - 1] * 4. / 
			    qemax)) * .5;
		} else if (qemax == qq[n + off]) {
/* Computing 2nd power */
		    d__1 = (qq[n - 1 + off] - qq[n + off] + ee[n - 1 + off]) /
			     qemax;
		    xx = (qq[n + off] + qq[n - 1 + off] + ee[n - 1 + off] + 
			    qemax * sqrt(d__1 * d__1 + ee[n - 1 + off] * 4. / 
			    qemax)) * .5;
		} else {
/* Computing 2nd power */
		    d__1 = (qq[n + off] - qq[n - 1 + off] + ee[n - 1 + off]) /
			     qemax;
		    xx = (qq[n + off] + qq[n - 1 + off] + ee[n - 1 + off] + 
			    qemax * sqrt(d__1 * d__1 + qq[n - 1 + off] * 4. / 
			    qemax)) * .5;
		}
/* Computing MAX */
		d__1 = qq[n + off], d__2 = qq[n - 1 + off];
/* Computing MIN */
		d__3 = qq[n + off], d__4 = qq[n - 1 + off];
		yy = max(d__1,d__2) / xx * min(d__3,d__4);
	    } else {
		xx = 0.;
		yy = 0.;
	    }
	} else {
/* Computing MAX */
	    d__1 = q[n + off], d__2 = q[n - 1 + off], d__1 = max(d__1,d__2), 
		    d__2 = e[n - 1 + off];
	    qemax = max(d__1,d__2);
	    if (qemax != 0.) {
		if (qemax == q[n - 1 + off]) {
/* Computing 2nd power */
		    d__1 = (q[n + off] - q[n - 1 + off] + e[n - 1 + off]) / 
			    qemax;
		    xx = (q[n + off] + q[n - 1 + off] + e[n - 1 + off] + 
			    qemax * sqrt(d__1 * d__1 + e[n - 1 + off] * 4. / 
			    qemax)) * .5;
		} else if (qemax == q[n + off]) {
/* Computing 2nd power */
		    d__1 = (q[n - 1 + off] - q[n + off] + e[n - 1 + off]) / 
			    qemax;
		    xx = (q[n + off] + q[n - 1 + off] + e[n - 1 + off] + 
			    qemax * sqrt(d__1 * d__1 + e[n - 1 + off] * 4. / 
			    qemax)) * .5;
		} else {
/* Computing 2nd power */
		    d__1 = (q[n + off] - q[n - 1 + off] + e[n - 1 + off]) / 
			    qemax;
		    xx = (q[n + off] + q[n - 1 + off] + e[n - 1 + off] + 
			    qemax * sqrt(d__1 * d__1 + q[n - 1 + off] * 4. / 
			    qemax)) * .5;
		}
/* Computing MAX */
		d__1 = q[n + off], d__2 = q[n - 1 + off];
/* Computing MIN */
		d__3 = q[n + off], d__4 = q[n - 1 + off];
		yy = max(d__1,d__2) / xx * min(d__3,d__4);
	    } else {
		xx = 0.;
		yy = 0.;
	    }
	}
	q[n - 1 + off] = sigma + xx;
	q[n + off] = yy + sigma;
	n = 0;
	goto L20;
    }
    dlasq3_(&n, &q[off1], &e[off1], &qq[off1], &ee[off1], sup, &sigma, kend, &
	    off, &iphase, &iconv, eps, tol2, small2);
    if (*sup < 0.) {
	*info = n + off;
	return 0;
    }
    off1 = off + 1;
    goto L20;

/*     End of DLASQ2 */

} /* dlasq2_ */

