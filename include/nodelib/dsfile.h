
/* Copyright (c) 1996 by G. W. Flake.
 *
 * NAME
 *   dsfile.h - DATASET_METHOD for flat binary files.
 * SYNOPSIS
 *   These routines define methods for accessing files.  With this
 *   module, a DATASET can consist of a large binary file.  Patterns
 *   are loaded on a need-only basis, so memory is preserved.
 * AUTHOR
 *   Gary William Flake (\url{\bf{gary.flake@usa.net}}{mailto:gary.flake@usa.net}).
 * SEE ALSO
 *   \bf{dsmethod}(3), and \bf{dataset}(3).
 */

#ifndef __DSFILE_H__
#define __DSFILE_H__

#include <stdio.h>
#include "nodelib/misc.h"
#include "nodelib/etc/version.h"
#include "nodelib/etc/options.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* The DSM_FILE type specififies how to interpret a binary file as a
   DATA_SET.  You need to set at least the first five fields.  The
   first two give the size of the x and y vectors.  The next two are
   the number of bytes that need to be read for the x and y vectors.
   The \em{offset} is the number of bytes to skip between the x and
   the y vectors, which may be a negative number.  The \em{step}
   specifies the distance in bytes between successive records in the
   file, and \em{skip} specifies the number of bytes to initially skip
   in the file.
   
   The two conversion routines should be used to map the binary data
   in the doubles. The value of the \em{obj} field is passed as the
   last argument.  If you need to implement some sort of delayed
   embedding, then you must do it in these routines.
   
   If you specify a \em{type} (as found in nodelib/misc.h) and if
   the size of your source type times \em{x_width} is equal to
   \em{x_read_width}, then \bf{dataset_x()} and\bf{dataset_y()}
   will automagically do the conversion for you. */

typedef struct DSM_FILE {
  /*
   * User modifiable fields.
   */
  unsigned x_width, y_width; 
  unsigned x_read_width, y_read_width;
  int offset;
  unsigned step, skip;
  int (*convert_x)(void *source, double *dest, void *obj);
  int (*convert_y)(void *source, double *dest, void *obj);
  void *obj;
  SL_TYPE type;
  /*
   * Private fields.
   */
  void *xbuf, *ybuf;
  double *xdata, *ydata;
  unsigned size, numpat, patsz, initp;
  FILE *fp;
  char *mmapptr;
} DSM_FILE;

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* This function allocates memory for a DSM_FILE structure, sets the
   fields to reasonable initial values, and checks that the requested
   file is valid and, if so, opens it for reading. */

DSM_FILE *dsm_file(char *fname);


/* After creating a DSM_FILE, one must set the fields to appropriate
   values.  Once they are set, this function is used to set up buffer
   space and to perform other miscellaneous tasks that are necessary
   before any of the other routines can be used. */

int dsm_file_initiate(DSM_FILE *dsmf);


/* Free up any memory that was allocated for the DSM_FILE, and closes
   the FILE pointer. */

void dsm_destroy_file(DSM_FILE *dsmf);


/* Internal function to retrieve x data from a file.  Don't use this
   function.  This is only to be used by the DATASET method handlers. */

double *dsm_file_x(void *instance, unsigned index);


/* Internal function to retrieve y data from a file.  Don't use this
   function.  This is only to be used by the DATASET method handlers. */

double *dsm_file_y(void *instance, unsigned index);

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __DSFILE_H__ */
