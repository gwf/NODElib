/* dlasq4.f -- translated by f2c (version 19950808).
   You must link the resulting object file with the libraries:
	-lf2c -lm   (in that order)
*/

#include "f2c.h"

/* Table of constant values */

static doublereal c_b4 = .7;

/* Subroutine */ int dlasq4_(n, q, e, tau, sup)
integer *n;
doublereal *q, *e, *tau, *sup;
{
    /* System generated locals */
    integer i__1;
    doublereal d__1, d__2;

    /* Builtin functions */
    double pow_di();

    /* Local variables */
    static doublereal xinf, d__;
    static integer i__;
    static doublereal dm;
    static integer ifl;


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

/*     DLASQ4 estimates TAU, the smallest eigenvalue of a matrix. This */
/*     routine improves the input value of SUP which is an upper bound */
/*     for the smallest eigenvalue for this matrix . */

/*     Arguments */
/*     ========= */

/*  N       (input) INTEGER */
/*          On entry, N specifies the number of rows and columns */
/*          in the matrix. N must be at least 0. */

/*  Q       (input) DOUBLE PRECISION array, dimension (N) */
/*          Q array */

/*  E       (input) DOUBLE PRECISION array, dimension (N) */
/*          E array */

/*  TAU     (output) DOUBLE PRECISION */
/*          Estimate of the shift */

/*  SUP     (input/output) DOUBLE PRECISION */
/*          Upper bound for the smallest singular value */

/*  ===================================================================== 
*/

/*     .. Parameters .. */
/*     .. */
/*     .. Local Scalars .. */
/*     .. */
/*     .. Intrinsic Functions .. */
/*     .. */
/*     .. Executable Statements .. */
    /* Parameter adjustments */
    --e;
    --q;

    /* Function Body */
    ifl = 1;
/* Computing MIN */
    d__1 = min(*sup,q[1]), d__1 = min(d__1,q[2]), d__1 = min(d__1,q[3]), d__2 
	    = q[*n], d__1 = min(d__1,d__2), d__2 = q[*n - 1], d__1 = min(d__1,
	    d__2), d__2 = q[*n - 2];
    *sup = min(d__1,d__2);
    *tau = *sup * .9999;
    xinf = 0.;
L10:
    if (ifl == 5) {
	*tau = xinf;
	return 0;
    }
    d__ = q[1] - *tau;
    dm = d__;
    i__1 = *n - 2;
    for (i__ = 1; i__ <= i__1; ++i__) {
	d__ = d__ / (d__ + e[i__]) * q[i__ + 1] - *tau;
	if (dm > d__) {
	    dm = d__;
	}
	if (d__ < 0.) {
	    *sup = *tau;
/* Computing MAX */
	    d__1 = *sup * pow_di(&c_b4, &ifl), d__2 = d__ + *tau;
	    *tau = max(d__1,d__2);
	    ++ifl;
	    goto L10;
	}
/* L20: */
    }
    d__ = d__ / (d__ + e[*n - 1]) * q[*n] - *tau;
    if (dm > d__) {
	dm = d__;
    }
    if (d__ < 0.) {
	*sup = *tau;
/* Computing MAX */
	d__1 = xinf, d__2 = d__ + *tau;
	xinf = max(d__1,d__2);
	if (*sup * pow_di(&c_b4, &ifl) <= xinf) {
	    *tau = xinf;
	} else {
	    *tau = *sup * pow_di(&c_b4, &ifl);
	    ++ifl;
	    goto L10;
	}
    } else {
/* Computing MIN */
	d__1 = *sup, d__2 = dm + *tau;
	*sup = min(d__1,d__2);
    }
    return 0;

/*     End of DLASQ4 */

} /* dlasq4_ */

