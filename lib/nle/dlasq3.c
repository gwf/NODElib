/* dlasq3.f -- translated by f2c (version 19950808).
   You must link the resulting object file with the libraries:
	-lf2c -lm   (in that order)
*/

#include "f2c.h"

/* Table of constant values */

static integer c__1 = 1;
static integer c_n1 = -1;

/* Subroutine */ int dlasq3_(n, q, e, qq, ee, sup, sigma, kend, off, iphase, 
	iconv, eps, tol2, small2)
integer *n;
doublereal *q, *e, *qq, *ee, *sup, *sigma;
integer *kend, *off, *iphase, *iconv;
doublereal *eps, *tol2, *small2;
{
    /* System generated locals */
    integer i__1, i__2;
    doublereal d__1, d__2, d__3, d__4;

    /* Builtin functions */
    double sqrt();

    /* Local variables */
    static logical ldef;
    static integer icnt;
    static doublereal tolx, toly, tolz;
    static integer k1end, k2end;
    static doublereal d__;
    static integer i__;
    static doublereal qemax;
    extern /* Subroutine */ int dcopy_();
    static integer maxit, n1, n2;
    static doublereal t1;
    extern /* Subroutine */ int dlasq4_();
    static integer ic, ke;
    static doublereal dm;
    static integer ip, ks;
    static doublereal xx, yy;
    static logical lsplit;
    static integer ifl;
    static doublereal tau;
    static integer isp;


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

/*     DLASQ3 is the workhorse of the whole bidiagonal SVD algorithm. */
/*     This can be described as the differential qd with shifts. */

/*     Arguments */
/*     ========= */

/*  N       (input/output) INTEGER */
/*          On entry, N specifies the number of rows and columns */
/*          in the matrix. N must be at least 3. */
/*          On exit N is non-negative and less than the input value. */

/*  Q       (input/output) DOUBLE PRECISION array, dimension (N) */
/*          Q array in ping (see IPHASE below) */

/*  E       (input/output) DOUBLE PRECISION array, dimension (N) */
/*          E array in ping (see IPHASE below) */

/*  QQ      (input/output) DOUBLE PRECISION array, dimension (N) */
/*          Q array in pong (see IPHASE below) */

/*  EE      (input/output) DOUBLE PRECISION array, dimension (N) */
/*          E array in pong (see IPHASE below) */

/*  SUP     (input/output) DOUBLE PRECISION */
/*          Upper bound for the smallest eigenvalue */

/*  SIGMA   (input/output) DOUBLE PRECISION */
/*          Accumulated shift for the present submatrix */

/*  KEND    (input/output) INTEGER */
/*          Index where minimum D(i) occurs in recurrence for */
/*          splitting criterion */

/*  OFF     (input/output) INTEGER */
/*          Offset for arrays */

/*  IPHASE  (input/output) INTEGER */
/*          If IPHASE = 1 (ping) then data is in Q and E arrays */
/*          If IPHASE = 2 (pong) then data is in QQ and EE arrays */

/*  ICONV   (input) INTEGER */
/*          If ICONV = 0 a bottom part of a matrix (with a split) */
/*          If ICONV =-3 a top part of a matrix (with a split) */

/*  EPS     (input) DOUBLE PRECISION */
/*          Machine epsilon */

/*  TOL2    (input) DOUBLE PRECISION */
/*          Square of the relative tolerance TOL as defined in DLASQ1 */

/*  SMALL2  (input) DOUBLE PRECISION */
/*          A threshold value as defined in DLASQ1 */

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
    icnt = 0;
    tau = 0.;
    dm = *sup;
    tolx = *sigma * *tol2;
    tolz = max(*small2,*sigma) * *tol2;

/*     Set maximum number of iterations */

    maxit = *n * 100;

/*     Flipping */

    ic = 2;
    if (*n > 3) {
	if (*iphase == 1) {
	    i__1 = *n - 2;
	    for (i__ = 1; i__ <= i__1; ++i__) {
		if (q[i__] > q[i__ + 1]) {
		    ++ic;
		}
		if (e[i__] > e[i__ + 1]) {
		    ++ic;
		}
/* L10: */
	    }
	    if (q[*n - 1] > q[*n]) {
		++ic;
	    }
	    if (ic < *n) {
		dcopy_(n, &q[1], &c__1, &qq[1], &c_n1);
		i__1 = *n - 1;
		dcopy_(&i__1, &e[1], &c__1, &ee[1], &c_n1);
		if (*kend != 0) {
		    *kend = *n - *kend + 1;
		}
		*iphase = 2;
	    }
	} else {
	    i__1 = *n - 2;
	    for (i__ = 1; i__ <= i__1; ++i__) {
		if (qq[i__] > qq[i__ + 1]) {
		    ++ic;
		}
		if (ee[i__] > ee[i__ + 1]) {
		    ++ic;
		}
/* L20: */
	    }
	    if (qq[*n - 1] > qq[*n]) {
		++ic;
	    }
	    if (ic < *n) {
		dcopy_(n, &qq[1], &c__1, &q[1], &c_n1);
		i__1 = *n - 1;
		dcopy_(&i__1, &ee[1], &c__1, &e[1], &c_n1);
		if (*kend != 0) {
		    *kend = *n - *kend + 1;
		}
		*iphase = 1;
	    }
	}
    }
    if (*iconv == -3) {
	if (*iphase == 1) {
	    goto L180;
	} else {
	    goto L80;
	}
    }
    if (*iphase == 2) {
	goto L130;
    }

/*     The ping section of the code */

L30:
    ifl = 0;

/*     Compute the shift */

    if (*kend == 0 || *sup == 0.) {
	tau = 0.;
    } else if (icnt > 0 && dm <= tolz) {
	tau = 0.;
    } else {
/* Computing MAX */
	i__1 = 5, i__2 = *n / 32;
	ip = max(i__1,i__2);
	n2 = (ip << 1) + 1;
	if (n2 >= *n) {
	    n1 = 1;
	    n2 = *n;
	} else if (*kend + ip > *n) {
	    n1 = *n - (ip << 1);
	} else if (*kend - ip < 1) {
	    n1 = 1;
	} else {
	    n1 = *kend - ip;
	}
	dlasq4_(&n2, &q[n1], &e[n1], &tau, sup);
    }
L40:
    ++icnt;
    if (icnt > maxit) {
	*sup = -1.;
	return 0;
    }
    if (tau == 0.) {

/*     dqd algorithm */

	d__ = q[1];
	dm = d__;
	ke = 0;
	i__1 = *n - 3;
	for (i__ = 1; i__ <= i__1; ++i__) {
	    qq[i__] = d__ + e[i__];
	    d__ = d__ / qq[i__] * q[i__ + 1];
	    if (dm > d__) {
		dm = d__;
		ke = i__;
	    }
/* L50: */
	}
	++ke;

/*     Penultimate dqd step (in ping) */

	k2end = ke;
	qq[*n - 2] = d__ + e[*n - 2];
	d__ = d__ / qq[*n - 2] * q[*n - 1];
	if (dm > d__) {
	    dm = d__;
	    ke = *n - 1;
	}

/*     Final dqd step (in ping) */

	k1end = ke;
	qq[*n - 1] = d__ + e[*n - 1];
	d__ = d__ / qq[*n - 1] * q[*n];
	if (dm > d__) {
	    dm = d__;
	    ke = *n;
	}
	qq[*n] = d__;
    } else {

/*     The dqds algorithm (in ping) */

	d__ = q[1] - tau;
	dm = d__;
	ke = 0;
	if (d__ < 0.) {
	    goto L120;
	}
	i__1 = *n - 3;
	for (i__ = 1; i__ <= i__1; ++i__) {
	    qq[i__] = d__ + e[i__];
	    d__ = d__ / qq[i__] * q[i__ + 1] - tau;
	    if (dm > d__) {
		dm = d__;
		ke = i__;
		if (d__ < 0.) {
		    goto L120;
		}
	    }
/* L60: */
	}
	++ke;

/*     Penultimate dqds step (in ping) */

	k2end = ke;
	qq[*n - 2] = d__ + e[*n - 2];
	d__ = d__ / qq[*n - 2] * q[*n - 1] - tau;
	if (dm > d__) {
	    dm = d__;
	    ke = *n - 1;
	    if (d__ < 0.) {
		goto L120;
	    }
	}

/*     Final dqds step (in ping) */

	k1end = ke;
	qq[*n - 1] = d__ + e[*n - 1];
	d__ = d__ / qq[*n - 1] * q[*n] - tau;
	if (dm > d__) {
	    dm = d__;
	    ke = *n;
	}
	qq[*n] = d__;
    }

/*        Convergence when QQ(N) is small (in ping) */

    if ((d__1 = qq[*n], abs(d__1)) <= *sigma * *tol2) {
	qq[*n] = 0.;
	dm = 0.;
	ke = *n;
    }
    if (qq[*n] < 0.) {
	goto L120;
    }

/*     Non-negative qd array: Update the e's */

    i__1 = *n - 1;
    for (i__ = 1; i__ <= i__1; ++i__) {
	ee[i__] = e[i__] / qq[i__] * q[i__ + 1];
/* L70: */
    }

/*     Updating sigma and iphase in ping */

    *sigma += tau;
    *iphase = 2;
L80:
    tolx = *sigma * *tol2;
    toly = *sigma * *eps;
    tolz = max(*sigma,*small2) * *tol2;

/*     Checking for deflation and convergence (in ping) */

L90:
    if (*n <= 2) {
	return 0;
    }

/*        Deflation: bottom 1x1 (in ping) */

    ldef = FALSE_;
    if (ee[*n - 1] <= tolz) {
	ldef = TRUE_;
    } else if (*sigma > 0.) {
	if (ee[*n - 1] <= *eps * (*sigma + qq[*n])) {
	    if (ee[*n - 1] * (qq[*n] / (qq[*n] + *sigma)) <= *tol2 * (qq[*n] 
		    + *sigma)) {
		ldef = TRUE_;
	    }
	}
    } else {
	if (ee[*n - 1] <= qq[*n] * *tol2) {
	    ldef = TRUE_;
	}
    }
    if (ldef) {
	q[*n] = qq[*n] + *sigma;
	--(*n);
	++(*iconv);
	goto L90;
    }

/*        Deflation: bottom 2x2 (in ping) */

    ldef = FALSE_;
    if (ee[*n - 2] <= tolz) {
	ldef = TRUE_;
    } else if (*sigma > 0.) {
	t1 = *sigma + ee[*n - 1] * (*sigma / (*sigma + qq[*n]));
	if (ee[*n - 2] * (t1 / (qq[*n - 1] + t1)) <= toly) {
	    if (ee[*n - 2] * (qq[*n - 1] / (qq[*n - 1] + t1)) <= tolx) {
		ldef = TRUE_;
	    }
	}
    } else {
	if (ee[*n - 2] <= qq[*n] / (qq[*n] + ee[*n - 1] + qq[*n - 1]) * qq[*n 
		- 1] * *tol2) {
	    ldef = TRUE_;
	}
    }
    if (ldef) {
/* Computing MAX */
	d__1 = qq[*n], d__2 = qq[*n - 1], d__1 = max(d__1,d__2), d__2 = ee[*n 
		- 1];
	qemax = max(d__1,d__2);
	if (qemax != 0.) {
	    if (qemax == qq[*n - 1]) {
/* Computing 2nd power */
		d__1 = (qq[*n] - qq[*n - 1] + ee[*n - 1]) / qemax;
		xx = (qq[*n] + qq[*n - 1] + ee[*n - 1] + qemax * sqrt(d__1 * 
			d__1 + ee[*n - 1] * 4. / qemax)) * .5;
	    } else if (qemax == qq[*n]) {
/* Computing 2nd power */
		d__1 = (qq[*n - 1] - qq[*n] + ee[*n - 1]) / qemax;
		xx = (qq[*n] + qq[*n - 1] + ee[*n - 1] + qemax * sqrt(d__1 * 
			d__1 + ee[*n - 1] * 4. / qemax)) * .5;
	    } else {
/* Computing 2nd power */
		d__1 = (qq[*n] - qq[*n - 1] + ee[*n - 1]) / qemax;
		xx = (qq[*n] + qq[*n - 1] + ee[*n - 1] + qemax * sqrt(d__1 * 
			d__1 + qq[*n - 1] * 4. / qemax)) * .5;
	    }
/* Computing MAX */
	    d__1 = qq[*n], d__2 = qq[*n - 1];
/* Computing MIN */
	    d__3 = qq[*n], d__4 = qq[*n - 1];
	    yy = max(d__1,d__2) / xx * min(d__3,d__4);
	} else {
	    xx = 0.;
	    yy = 0.;
	}
	q[*n - 1] = *sigma + xx;
	q[*n] = yy + *sigma;
	*n += -2;
	*iconv += 2;
	goto L90;
    }

/*     Updating bounds before going to pong */

    if (*iconv == 0) {
	*kend = ke;
/* Computing MIN */
	d__1 = dm, d__2 = *sup - tau;
	*sup = min(d__1,d__2);
    } else if (*iconv > 0) {
/* Computing MIN */
	d__1 = qq[*n], d__2 = qq[*n - 1], d__1 = min(d__1,d__2), d__2 = qq[*n 
		- 2], d__1 = min(d__1,d__2), d__1 = min(d__1,qq[1]), d__1 = 
		min(d__1,qq[2]);
	*sup = min(d__1,qq[3]);
	if (*iconv == 1) {
	    *kend = k1end;
	} else if (*iconv == 2) {
	    *kend = k2end;
	} else {
	    *kend = *n;
	}
	icnt = 0;
	maxit = *n * 100;
    }

/*     Checking for splitting in ping */

    lsplit = FALSE_;
    for (ks = *n - 3; ks >= 3; --ks) {
	if (ee[ks] <= toly) {
/* Computing MIN */
	    d__1 = qq[ks + 1], d__2 = qq[ks];
/* Computing MIN */
	    d__3 = qq[ks + 1], d__4 = qq[ks];
	    if (ee[ks] * (min(d__1,d__2) / (min(d__3,d__4) + *sigma)) <= tolx)
		     {
		lsplit = TRUE_;
		goto L110;
	    }
	}
/* L100: */
    }

    ks = 2;
    if (ee[2] <= tolz) {
	lsplit = TRUE_;
    } else if (*sigma > 0.) {
	t1 = *sigma + ee[1] * (*sigma / (*sigma + qq[1]));
	if (ee[2] * (t1 / (qq[1] + t1)) <= toly) {
	    if (ee[2] * (qq[1] / (qq[1] + t1)) <= tolx) {
		lsplit = TRUE_;
	    }
	}
    } else {
	if (ee[2] <= qq[1] / (qq[1] + ee[1] + qq[2]) * qq[2] * *tol2) {
	    lsplit = TRUE_;
	}
    }
    if (lsplit) {
	goto L110;
    }

    ks = 1;
    if (ee[1] <= tolz) {
	lsplit = TRUE_;
    } else if (*sigma > 0.) {
	if (ee[1] <= *eps * (*sigma + qq[1])) {
	    if (ee[1] * (qq[1] / (qq[1] + *sigma)) <= *tol2 * (qq[1] + *sigma)
		    ) {
		lsplit = TRUE_;
	    }
	}
    } else {
	if (ee[1] <= qq[1] * *tol2) {
	    lsplit = TRUE_;
	}
    }

L110:
    if (lsplit) {
/* Computing MIN */
	d__1 = qq[*n], d__2 = qq[*n - 1], d__1 = min(d__1,d__2), d__2 = qq[*n 
		- 2];
	*sup = min(d__1,d__2);
	isp = -(*off + 1);
	*off += ks;
	*n -= ks;
/* Computing MAX */
	i__1 = 1, i__2 = *kend - ks;
	*kend = max(i__1,i__2);
	e[ks] = *sigma;
	ee[ks] = (doublereal) isp;
	*iconv = 0;
	return 0;
    }

/*     Coincidence */

    if (tau == 0. && dm <= tolz && *kend != *n && *iconv == 0 && icnt > 0) {
	i__1 = *n - ke;
	dcopy_(&i__1, &e[ke], &c__1, &qq[ke], &c__1);
	qq[*n] = 0.;
	i__1 = *n - ke;
	dcopy_(&i__1, &q[ke + 1], &c__1, &ee[ke], &c__1);
	*sup = 0.;
    }
    *iconv = 0;
    goto L130;

/*     A new shift when the previous failed (in ping) */

L120:
    ++ifl;
    *sup = tau;

/*     SUP is small or */
/*     Too many bad shifts (ping) */

    if (*sup <= tolz || ifl >= 2) {
	tau = 0.;
	goto L40;

/*     The asymptotic shift (in ping) */

    } else {
/* Computing MAX */
	d__1 = tau + d__;
	tau = max(d__1,0.);
	if (tau <= tolz) {
	    tau = 0.;
	}
	goto L40;
    }

/*     the pong section of the code */

L130:
    ifl = 0;

/*     Compute the shift (in pong) */

    if (*kend == 0 && *sup == 0.) {
	tau = 0.;
    } else if (icnt > 0 && dm <= tolz) {
	tau = 0.;
    } else {
/* Computing MAX */
	i__1 = 5, i__2 = *n / 32;
	ip = max(i__1,i__2);
	n2 = (ip << 1) + 1;
	if (n2 >= *n) {
	    n1 = 1;
	    n2 = *n;
	} else if (*kend + ip > *n) {
	    n1 = *n - (ip << 1);
	} else if (*kend - ip < 1) {
	    n1 = 1;
	} else {
	    n1 = *kend - ip;
	}
	dlasq4_(&n2, &qq[n1], &ee[n1], &tau, sup);
    }
L140:
    ++icnt;
    if (icnt > maxit) {
	*sup = -(*sup);
	return 0;
    }
    if (tau == 0.) {

/*     The dqd algorithm (in pong) */

	d__ = qq[1];
	dm = d__;
	ke = 0;
	i__1 = *n - 3;
	for (i__ = 1; i__ <= i__1; ++i__) {
	    q[i__] = d__ + ee[i__];
	    d__ = d__ / q[i__] * qq[i__ + 1];
	    if (dm > d__) {
		dm = d__;
		ke = i__;
	    }
/* L150: */
	}
	++ke;

/*     Penultimate dqd step (in pong) */

	k2end = ke;
	q[*n - 2] = d__ + ee[*n - 2];
	d__ = d__ / q[*n - 2] * qq[*n - 1];
	if (dm > d__) {
	    dm = d__;
	    ke = *n - 1;
	}

/*     Final dqd step (in pong) */

	k1end = ke;
	q[*n - 1] = d__ + ee[*n - 1];
	d__ = d__ / q[*n - 1] * qq[*n];
	if (dm > d__) {
	    dm = d__;
	    ke = *n;
	}
	q[*n] = d__;
    } else {

/*     The dqds algorithm (in pong) */

	d__ = qq[1] - tau;
	dm = d__;
	ke = 0;
	if (d__ < 0.) {
	    goto L220;
	}
	i__1 = *n - 3;
	for (i__ = 1; i__ <= i__1; ++i__) {
	    q[i__] = d__ + ee[i__];
	    d__ = d__ / q[i__] * qq[i__ + 1] - tau;
	    if (dm > d__) {
		dm = d__;
		ke = i__;
		if (d__ < 0.) {
		    goto L220;
		}
	    }
/* L160: */
	}
	++ke;

/*     Penultimate dqds step (in pong) */

	k2end = ke;
	q[*n - 2] = d__ + ee[*n - 2];
	d__ = d__ / q[*n - 2] * qq[*n - 1] - tau;
	if (dm > d__) {
	    dm = d__;
	    ke = *n - 1;
	    if (d__ < 0.) {
		goto L220;
	    }
	}

/*     Final dqds step (in pong) */

	k1end = ke;
	q[*n - 1] = d__ + ee[*n - 1];
	d__ = d__ / q[*n - 1] * qq[*n] - tau;
	if (dm > d__) {
	    dm = d__;
	    ke = *n;
	}
	q[*n] = d__;
    }

/*        Convergence when is small (in pong) */

    if ((d__1 = q[*n], abs(d__1)) <= *sigma * *tol2) {
	q[*n] = 0.;
	dm = 0.;
	ke = *n;
    }
    if (q[*n] < 0.) {
	goto L220;
    }

/*     Non-negative qd array: Update the e's */

    i__1 = *n - 1;
    for (i__ = 1; i__ <= i__1; ++i__) {
	e[i__] = ee[i__] / q[i__] * qq[i__ + 1];
/* L170: */
    }

/*     Updating sigma and iphase in pong */

    *sigma += tau;
L180:
    *iphase = 1;
    tolx = *sigma * *tol2;
    toly = *sigma * *eps;

/*     Checking for deflation and convergence (in pong) */

L190:
    if (*n <= 2) {
	return 0;
    }

/*        Deflation: bottom 1x1 (in pong) */

    ldef = FALSE_;
    if (e[*n - 1] <= tolz) {
	ldef = TRUE_;
    } else if (*sigma > 0.) {
	if (e[*n - 1] <= *eps * (*sigma + q[*n])) {
	    if (e[*n - 1] * (q[*n] / (q[*n] + *sigma)) <= *tol2 * (q[*n] + *
		    sigma)) {
		ldef = TRUE_;
	    }
	}
    } else {
	if (e[*n - 1] <= q[*n] * *tol2) {
	    ldef = TRUE_;
	}
    }
    if (ldef) {
	q[*n] += *sigma;
	--(*n);
	++(*iconv);
	goto L190;
    }

/*        Deflation: bottom 2x2 (in pong) */

    ldef = FALSE_;
    if (e[*n - 2] <= tolz) {
	ldef = TRUE_;
    } else if (*sigma > 0.) {
	t1 = *sigma + e[*n - 1] * (*sigma / (*sigma + q[*n]));
	if (e[*n - 2] * (t1 / (q[*n - 1] + t1)) <= toly) {
	    if (e[*n - 2] * (q[*n - 1] / (q[*n - 1] + t1)) <= tolx) {
		ldef = TRUE_;
	    }
	}
    } else {
	if (e[*n - 2] <= q[*n] / (q[*n] + ee[*n - 1] + q[*n - 1]) * q[*n - 1] 
		* *tol2) {
	    ldef = TRUE_;
	}
    }
    if (ldef) {
/* Computing MAX */
	d__1 = q[*n], d__2 = q[*n - 1], d__1 = max(d__1,d__2), d__2 = e[*n - 
		1];
	qemax = max(d__1,d__2);
	if (qemax != 0.) {
	    if (qemax == q[*n - 1]) {
/* Computing 2nd power */
		d__1 = (q[*n] - q[*n - 1] + e[*n - 1]) / qemax;
		xx = (q[*n] + q[*n - 1] + e[*n - 1] + qemax * sqrt(d__1 * 
			d__1 + e[*n - 1] * 4. / qemax)) * .5;
	    } else if (qemax == q[*n]) {
/* Computing 2nd power */
		d__1 = (q[*n - 1] - q[*n] + e[*n - 1]) / qemax;
		xx = (q[*n] + q[*n - 1] + e[*n - 1] + qemax * sqrt(d__1 * 
			d__1 + e[*n - 1] * 4. / qemax)) * .5;
	    } else {
/* Computing 2nd power */
		d__1 = (q[*n] - q[*n - 1] + e[*n - 1]) / qemax;
		xx = (q[*n] + q[*n - 1] + e[*n - 1] + qemax * sqrt(d__1 * 
			d__1 + q[*n - 1] * 4. / qemax)) * .5;
	    }
/* Computing MAX */
	    d__1 = q[*n], d__2 = q[*n - 1];
/* Computing MIN */
	    d__3 = q[*n], d__4 = q[*n - 1];
	    yy = max(d__1,d__2) / xx * min(d__3,d__4);
	} else {
	    xx = 0.;
	    yy = 0.;
	}
	q[*n - 1] = *sigma + xx;
	q[*n] = yy + *sigma;
	*n += -2;
	*iconv += 2;
	goto L190;
    }

/*     Updating bounds before going to pong */

    if (*iconv == 0) {
	*kend = ke;
/* Computing MIN */
	d__1 = dm, d__2 = *sup - tau;
	*sup = min(d__1,d__2);
    } else if (*iconv > 0) {
/* Computing MIN */
	d__1 = q[*n], d__2 = q[*n - 1], d__1 = min(d__1,d__2), d__2 = q[*n - 
		2], d__1 = min(d__1,d__2), d__1 = min(d__1,q[1]), d__1 = min(
		d__1,q[2]);
	*sup = min(d__1,q[3]);
	if (*iconv == 1) {
	    *kend = k1end;
	} else if (*iconv == 2) {
	    *kend = k2end;
	} else {
	    *kend = *n;
	}
	icnt = 0;
	maxit = *n * 100;
    }

/*     Checking for splitting in pong */

    lsplit = FALSE_;
    for (ks = *n - 3; ks >= 3; --ks) {
	if (e[ks] <= toly) {
/* Computing MIN */
	    d__1 = q[ks + 1], d__2 = q[ks];
/* Computing MIN */
	    d__3 = q[ks + 1], d__4 = q[ks];
	    if (e[ks] * (min(d__1,d__2) / (min(d__3,d__4) + *sigma)) <= tolx) 
		    {
		lsplit = TRUE_;
		goto L210;
	    }
	}
/* L200: */
    }

    ks = 2;
    if (e[2] <= tolz) {
	lsplit = TRUE_;
    } else if (*sigma > 0.) {
	t1 = *sigma + e[1] * (*sigma / (*sigma + q[1]));
	if (e[2] * (t1 / (q[1] + t1)) <= toly) {
	    if (e[2] * (q[1] / (q[1] + t1)) <= tolx) {
		lsplit = TRUE_;
	    }
	}
    } else {
	if (e[2] <= q[1] / (q[1] + e[1] + q[2]) * q[2] * *tol2) {
	    lsplit = TRUE_;
	}
    }
    if (lsplit) {
	goto L210;
    }

    ks = 1;
    if (e[1] <= tolz) {
	lsplit = TRUE_;
    } else if (*sigma > 0.) {
	if (e[1] <= *eps * (*sigma + q[1])) {
	    if (e[1] * (q[1] / (q[1] + *sigma)) <= *tol2 * (q[1] + *sigma)) {
		lsplit = TRUE_;
	    }
	}
    } else {
	if (e[1] <= q[1] * *tol2) {
	    lsplit = TRUE_;
	}
    }

L210:
    if (lsplit) {
/* Computing MIN */
	d__1 = q[*n], d__2 = q[*n - 1], d__1 = min(d__1,d__2), d__2 = q[*n - 
		2];
	*sup = min(d__1,d__2);
	isp = *off + 1;
	*off += ks;
/* Computing MAX */
	i__1 = 1, i__2 = *kend - ks;
	*kend = max(i__1,i__2);
	*n -= ks;
	e[ks] = *sigma;
	ee[ks] = (doublereal) isp;
	*iconv = 0;
	return 0;
    }

/*     Coincidence */

    if (tau == 0. && dm <= tolz && *kend != *n && *iconv == 0 && icnt > 0) {
	i__1 = *n - ke;
	dcopy_(&i__1, &ee[ke], &c__1, &q[ke], &c__1);
	q[*n] = 0.;
	i__1 = *n - ke;
	dcopy_(&i__1, &qq[ke + 1], &c__1, &e[ke], &c__1);
	*sup = 0.;
    }
    *iconv = 0;
    goto L30;

/*     Computation of a new shift when the previous failed (in pong) */

L220:
    ++ifl;
    *sup = tau;

/*     SUP is small or */
/*     Too many bad shifts (in pong) */

    if (*sup <= tolz || ifl >= 2) {
	tau = 0.;
	goto L140;

/*     The asymptotic shift (in pong) */

    } else {
/* Computing MAX */
	d__1 = tau + d__;
	tau = max(d__1,0.);
	if (tau <= tolz) {
	    tau = 0.;
	}
	goto L140;
    }

/*     End of DLASQ3 */

} /* dlasq3_ */

