/* dscal.f -- translated by f2c (version of 23 April 1993  18:34:30).
   You must link the resulting object file with the libraries:
	-lf2c -lm   (in that order)
*/

#include "f2c.h"

/* Subroutine */ int dscal_(n, da, dx, incx)
integer *n;
doublereal *da, *dx;
integer *incx;
{
    /* System generated locals */
    integer i__1;

    /* Local variables */
    static integer i, m, ix, mp1;


/*     scales a vector by a constant. */
/*     uses unrolled loops for increment equal to one. */
/*     jack dongarra, linpack, 3/11/78. */
/*     modified to correct problem with negative increment, 8/21/90. */


    /* Parameter adjustments */
    --dx;

    /* Function Body */
    if (*n <= 0) {
	return 0;
    }
    if (*incx == 1) {
	goto L20;
    }

/*        code for increment not equal to 1 */

    ix = 1;
    if (*incx < 0) {
	ix = (-(*n) + 1) * *incx + 1;
    }
    i__1 = *n;
    for (i = 1; i <= i__1; ++i) {
	dx[ix] = *da * dx[ix];
	ix += *incx;
/* L10: */
    }
    return 0;

/*        code for increment equal to 1 */


/*        clean-up loop */

L20:
    m = *n % 5;
    if (m == 0) {
	goto L40;
    }
    i__1 = m;
    for (i = 1; i <= i__1; ++i) {
	dx[i] = *da * dx[i];
/* L30: */
    }
    if (*n < 5) {
	return 0;
    }
L40:
    mp1 = m + 1;
    i__1 = *n;
    for (i = mp1; i <= i__1; i += 5) {
	dx[i] = *da * dx[i];
	dx[i + 1] = *da * dx[i + 1];
	dx[i + 2] = *da * dx[i + 2];
	dx[i + 3] = *da * dx[i + 3];
	dx[i + 4] = *da * dx[i + 4];
/* L50: */
    }
    return 0;
} /* dscal_ */

