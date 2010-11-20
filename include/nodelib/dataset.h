
/* Copyright (c) 1996 by G. W. Flake.
 *
 * NAME
 *   dataset.h - A generic dataset type with custom methods
 * SYNOPSIS
 *   The data type and routines contained in this module represent
 *   a generic interface that is applicable to a broad class data
 *   sources.  Practically any data source can be massaged into
 *   a DATASET, which means that all of NODElib can use the types.
 * DESCRIPTION
 *   The \bf{dataset} routines allow one to access arbitrary data types
 *   in a consistant and uniform manner.  Thus, files, matrices, and time
 *   series can all ``look'' the same if they are massaged into the
 *   DATASET type.  The routines listed in this document do not actually
 *   do anything useful except call the appropriate methods with the
 *   instance as one of the arguments.  You should really look in the
 *   \bf{dsmethod}(3) documentation for more information on method
 *   types.
 * BUGS
 *   None are possible, since all of the methods reside in other packages.
 * AUTHOR
 *   Gary William Flake (\url{\bf{gary.flake@usa.net}}{mailto:gary.flake@usa.net}).
 * SEE ALSO
 *   \bf{series}(3), and \bf{dsmethod}(3).
 */

#ifndef __DATASET_H__
#define __DATASET_H__

#include "nodelib/dsmethod.h"
#include "nodelib/etc/version.h"
#include "nodelib/etc/options.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* The \em{instance} field is a pointer to the real data type which
   contains data.  The \em{method} field is a pointer to the virtual
   functions to access the \em{instance} field. */

typedef struct DATASET {
  void *instance;
  DATASET_METHOD *method;
} DATASET;

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* Creates and returns a pointer to a DATASET structure.  You need to 
   specify \em{instance} and \em{method} so that the routines
   described below can work. */

DATASET *dataset_create(DATASET_METHOD *method, void *instance);


/* Destroys the dynamic memory allocated by a call to \bf{dataset()}
   and returns the value of \em{dataset->instance}. */

void *dataset_destroy(DATASET *dataset);


/* Returns the number of patterns inside the dataset refered to by
   the \em{dataset} argument. */

unsigned dataset_size(DATASET *dataset);


/* Returns the dimensionality of the input patterns refered to by
   the \em{dataset} argument. */

unsigned dataset_x_size(DATASET *dataset);


/* Returns the dimensionality of the output patterns refered to by
   the \em{dataset} argument. */

unsigned dataset_y_size(DATASET *dataset);


/* Returns the input pattern specified by \em{index} inside the
  \em{dataset} argument.  By convention, the first pattern should
  have an \em{index} of zero.  The memory returned by the call
  is ``owned'' by the \em{instance} of the \bf{dataset argument}.
  Hence, you will probably never attempt to free this memory
  yourself.  See the documentation for the specific method type
  for more details. */

double *dataset_x(DATASET *dataset, unsigned index);


/* Returns the output pattern specified by \em{index} inside the
  \em{dataset} argument.  By convention, the first pattern should
  have an \em{index} of zero.  The memory returned by the call
  is ``owned'' by the \em{instance} of the \bf{dataset argument}.
  Hence, you will probably never attempt to free this memory
  yourself.  See the documentation for the specific method type
  for more details. */

double *dataset_y(DATASET *dataset, unsigned index);


/* Similar to dataset_x() but places the data in \em{dst.}  If
   \em{dst} is NULL, then space is allocated with allocate_array().
   In either case, the address of the memory in which the data is
   stored is returned.  The caller owns the result, and must
   therefore deallocate it with deallocate_array() when done. */

double *dataset_x_copy(DATASET *dataset, unsigned index, double *dst);


/* Similar to dataset_y() but places the data in \em{dst.}  If
   \em{dst} is NULL, then space is allocated with allocate_array().
   In either case, the address of the memory in which the data is
   stored is returned.  The caller owns the result, and must
   therefore deallocate it with deallocate_array() when done. */

double *dataset_y_copy(DATASET *dataset, unsigned index, double *dst);

#ifdef __cplusplus
}
#endif /* __cplusplus */

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#endif /* __DATASET_H__ */


