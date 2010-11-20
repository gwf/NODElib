
/* Copyright (c) 1995-1997 by G. W. Flake.
 *
 * NAME
 *   series.h - data stream handling
 * SYNOPSIS
 *   This modules allows one to access data in flat-files in a unified
 *   manner.  For time series analysis, a one-dimensional stream of
 *   data is efficiently stored in memory but can still be easily
 *   accessed in terms of delayed coordinates as if it actually
 *   consists of multi-dimensional vectors.  This module can also be
 *   used for numerical pattern classification as well, making it a
 *   suitable data type for neural networks to operate on.
 * AUTHOR
 *   Gary William Flake (\url{\bf{gary.flake@usa.net}}{mailto:gary.flake@usa.net}).
 */

#ifndef __SERIES_H__
#define __SERIES_H__

#include <stdio.h>
#include "nodelib/array.h"
#include "nodelib/etc/version.h"
#include "nodelib/etc/options.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* A SERIES contains a stream of data and a means to interpret it.
   If a stream of data has been retrieved from a plant or time series,
   then we usually want to use it in a manner that is a bit more involved
   then simply looking at successive units.  For example, if we were
   analyzing stock market ticker values over time, F(t), then one
   possible way of looking at the data would be with an input window of
   previous history, containing three inputs, each seperated by five
   time steps, and we wish to compare this to a point ten time steps
   beyond the last input.  Additionally, the output could be a vector,
   with each element seperated by seven time steps.

   The following example assumes that the variable length deltas are
   not used.  Suppose that we want successive patterns to be every six
   steps over the time series.  This gives us:
   \begin{verbatim}
   \bf
      series  ->  00000000001111111111222222222233333333334
      in time     01234567890123456789012345678901234567890
                  -----------------------------------------
      first       x    x    x         y      y
      iteration   0    1    2         0      1
                   <-5-><-5-><---10---><--7-->
                  ----------------------------
      second            x    x    x         y      y
      iteration         0    1    2         0      1
                   --6--><-5-><-5-><---10---><--7-->
                        ----------------------------
      third                   x    x    x         y      y
      iteration               0    1    2         0      1
                         --6--><-5-><-5-><---10---><--7-->
                              ----------------------------
   \rm
   \end{verbatim}
   If either of \em{var_x_deltas} or \em{var_y_deltas} is non-NULL,
   then the packing of the returned patterns is performed differently.
   Using just \em{var_x_deltas} as an example, the internal routines
   expect \em{(x_width - 1)} values to be in \em{var_x_deltas} with
   each succesive value greater than the previous.  The returned pattern
   is such that if the first element in the returned array, \em{x[0]},
   is equal to F(t), then \em{x[i]} is equal to
   F(t - \em{var_x_deltas[i - 1]}).  The \em{var_y_deltas} field works
   similarly.

   \bf{Warning}: If you are not using the variable length deltas, then  
   for x[i] and x[j], i < j implies that x[i] is "older" than x[j].
   However, if you are using variable length deltas, then x[i] would be
   "younger" than x[j].  When in doubt, look at the output of the
   test program \bf{tseries} which should illustrate how things work. */

typedef struct SERIES {
  unsigned  x_width,  x_delta;
  unsigned  y_width,  y_delta;
  int       offset,   step;
  ARRAY    *data,    *x_pat,    *y_pat;
  unsigned *var_x_deltas, *var_y_deltas;
  /*
   * Private fields.
   */
  unsigned patsz, numpats, initp, xsz, ysz;
} SERIES;

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* Create an empty series. */

SERIES *series_create(void);


/* Destroy a series, returning all associated memory to the system. */

void series_destroy(SERIES *ser);


/* Sets internal pointers such that \em{ser} has the appearance of
   being empty.  Memory is actually retained in case data is inserted into
   the SERIES structure.  No other values are altered, as all internal
   spacing fields are untouched. */

void series_clear(SERIES *ser);

/* Reads all of the ascii data in a file pointed to by \em{fp} until an
   EOF is encountered. The format of the ata is irrelevant as this routine
   will simply read every number that it can. The '#' character is considered
   a comment delimiter. */

SERIES *series_read_ascii_fp(FILE *fp);


/* This function is identical to \bf{series_read_ascii_fp()} except that the
   named file, \em{fname} is opened and read. */

SERIES *series_read_ascii(char *fname);


/* Writes every pattern in ascii format to the file pointed to by \em{fp}.
   It is the caller's responsibility to make sure that the FILE pointer is
   valid. */

void series_write_ascii_fp(SERIES *ser, FILE *fp);


/* This function is similar to \bf{series_write_ascii_fp() } axcept that the
   named file, \em{fname} is opened and written to.  On error a nonzero
   value is returned. */

int series_write_ascii(SERIES *ser, char *name);


/* This function must be called if the \em{var_x_deltas} or the
   \em{var_y_deltas} fields are changed after the SERIES first use. */

void series_reinitiate(SERIES *ser);


/* Returns the number of patterns in \em{ser}, which is solely determined
   by the structure field members x_width, x_delta, y_width, y_delta, offset,
   and step, and the number of data points stored in \em{ser}. See the
   explanation of the typedef SERIES for more details. */

unsigned series_get_num_pat(SERIES *ser);


/* Returns the number of points consumed for a single pattern in \em{ser}.
   In the example from the typedef documentation, each pattern size would
   be 28. */

unsigned series_get_pat_size(SERIES *ser);


/* Returns the x-portion of the pattern specified by \em{index} where zero
   is the first pattern. */

double *series_get_x_pat(SERIES *ser, unsigned index);


/* Returns the y-portion of the pattern specified by \em{index} where zero
   is the first pattern. */

double *series_get_y_pat(SERIES *ser, unsigned index);


/* Returns the \em{which} component of the x-portion of the pattern
   specified by \em{index}. */

double series_get_x(SERIES *ser, unsigned index, unsigned which);


/* Returns the \em{which} component of the y-portion of the pattern
   specified by \em{index}. */

double series_get_y(SERIES *ser, unsigned index, unsigned which);


/* Sets the \em{which} component of the x-portion of the series
   specified by \em{index}.  The return value indicates if an
   error occured. */

int series_set_x(SERIES *ser, unsigned index, unsigned which, double val);


/* Sets the \em{which} component of the y-portion of the pattern
   specified by \em{index}.  The return value indicates if an
   error occured. */

int series_set_y(SERIES *ser, unsigned index, unsigned which, double val);


/* Appends the \em{sz} data points pointed to by \em{data} onto the
   rear end of \em{ser}.  This function is useful for storing an
   indeterminate amount of data that you would later like to index like
   a SERIES. */

void series_append_pat(SERIES *ser, double *data, unsigned sz);


/* Appends a single data point to the rear end of a SERIES. */

void series_append_val(SERIES *ser, double data);


/* Compute and return the average of all elements in \em{ser},
   regardless of how \em{ser} is structured.  */

double series_average(SERIES *ser);


/* Compute and return the standard deviation of all elements in \em{ser},
   regardless of how \em{ser} is structured. */

double series_stddev(SERIES *ser);


/* Compute and return the minimuim of all elements in \em{ser},
   regardless of how \em{ser} is structured. */

double series_min(SERIES *ser);


/* Compute and return the maximum of all elements in \em{ser},
   regardless of how \em{ser} is structured. */

double series_max(SERIES *ser);

/* h2man:include The functions \bf{series_average()}, \bf{series_stddev()},
   \bf{series_min()}, and \bf{series_max()} will compute the named
   statitistic over the set of numbers which consists of all the data
   points in \em{ser}. */

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __SERIES_H__ */
