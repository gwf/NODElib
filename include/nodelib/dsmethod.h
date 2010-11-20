
/* Copyright (c) 1996 by G. W. Flake.
 *
 * NAME
 *   dsmethod.h - methods for various DATASET instance types
 * SYNOPSIS
 *   The DATASET_METHODs defined in this module can be used to
 *   transform other types into DATASETs.  Methods currently in
 *   this module include ones for C matrices and the SERIES type.
 * DESCRIPTION
 *   You probably only need to know about the predefined method
 *   types which are listed below in the Miscellaneous Items
 *   subsection found at the end of this section.  If you wish to
 *   write your own DATASET method, then see the  documentation
 *   for the DATASET_METHOD type.
 *
 *   A DATASET_METHOD type should be defined once for each instance
 *   type of a DATASET.  Thus, there should be one defined for SERIES,
 *   and another defined for C matrices, and maybe a third to access
 *   some specific database file format, etc.  In each case, functions
 *   to answer the five possible query types in a DATASET_METHOD need
 *   to be defined.  The only potentially tricky part of writing
 *   a DATASET_METHOD is to come up with a reasonable convention for
 *   who owns the memory returned by the \em{x()} and \em{y()}
 *   calls.
 *
 *   All of the methods defined in this package assume that they
 *   ``own'' the memory.  Thus, they frequently reuse this memory
 *   and only free it when the instance itself is destroyed.  See
 *   the implementation of \bf{series}(3) to see how I have
 *   handled this problem.
 * AUTHOR
 *   Gary William Flake (\url{\bf{gary.flake@usa.net}}{mailto:gary.flake@usa.net}).
 * SEE ALSO
 *   \bf{series}(3), and \bf{dataset}(3).
 */

#ifndef __DSMETHOD_H__
#define __DSMETHOD_H__

#include "nodelib/etc/version.h"
#include "nodelib/etc/options.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* The function table for a specific instance needs to have five
   functions defined: one to return the number of patterns in the
   DATASET (\em{size()}), two functions to return the dimensionality
   of the input and output patterns (\em{x_size()} and \em{y_size()}), and
   two to return specific input and output patterns (\em{x()} and
   \em{y()}).

   Don't forget: If you are writing your own method, remember that
   \em{instance} is going to be passed as a (void *) type; thus,
   you will need to cast the pointer back into its ``real'' type
   when you receive it.  See the source code for \bf{dsmethod}(3)
   for more information. */

typedef struct DATASET_METHOD {
  unsigned (*size)(void *instance);
  unsigned (*x_size)(void *instance);
  unsigned (*y_size)(void *instance);
  double  *(*x)(void *instance, unsigned index);
  double  *(*y)(void *instance, unsigned index);
} DATASET_METHOD;

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __DSMETHOD_H__ */

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef NOEXTERN
#ifndef __DSMETHOD_EXTERN_H__
#define __DSMETHOD_EXTERN_H__

/* h2man:include Currently defined DATASET_METHODs include:
  \begin{itemize}
  \item \bf{dsm_series_method:}
     Method for accessing SERIES types.
  \item \bf{dsm_matrix_method:}
     Method for accessing one or two C matrices.
  \item \bf{dsm_dblptr_method:}
     Method for accessing one or two C double pointer matrices.
  \item \bf{dsm_file_method:}
     Method for accessing enormous binary files.
  \item \bf{dsm_subset_method:}
     Method for accessing a nearly contiguous portion of another DATASET.
  \item \bf{dsm_isubset_method:}
     Method for accessing an indexed portion of another DATASET.
  \item \bf{dsm_fifo_method:}
     Method for accessing incrementally made fifo sets.
  \end{itemize}
 */

/* h2man:skipbeg */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef OWNER

DATASET_METHOD dsm_series_method = {
  dsm_series_size,
  dsm_series_x_size,
  dsm_series_y_size,
  dsm_series_x,
  dsm_series_y
};

DATASET_METHOD dsm_matrix_method = {
  dsm_matrix_size,
  dsm_matrix_x_size,
  dsm_matrix_y_size,
  dsm_matrix_x,
  dsm_matrix_y
};

DATASET_METHOD dsm_dblptr_method = {
  dsm_dblptr_size,
  dsm_dblptr_x_size,
  dsm_dblptr_y_size,
  dsm_dblptr_x,
  dsm_dblptr_y
};

DATASET_METHOD dsm_file_method = {
  dsm_file_size,
  dsm_file_x_size,
  dsm_file_y_size,
  dsm_file_x,
  dsm_file_y
};

DATASET_METHOD dsm_subset_method = {
  dsm_subset_size,
  dsm_subset_x_size,
  dsm_subset_y_size,
  dsm_subset_x,
  dsm_subset_y
};

DATASET_METHOD dsm_isubset_method = {
  dsm_isubset_size,
  dsm_isubset_x_size,
  dsm_isubset_y_size,
  dsm_isubset_x,
  dsm_isubset_y
};

DATASET_METHOD dsm_fifo_method = {
  dsm_fifo_size,
  dsm_fifo_x_size,
  dsm_fifo_y_size,
  dsm_fifo_x,
  dsm_fifo_y
};

DATASET_METHOD dsm_union_method = {
  dsm_union_size,
  dsm_union_x_size,
  dsm_union_y_size,
  dsm_union_x,
  dsm_union_y
};

#else /* OWNER */

extern DATASET_METHOD dsm_series_method;
extern DATASET_METHOD dsm_matrix_method;
extern DATASET_METHOD dsm_dblptr_method;
extern DATASET_METHOD dsm_file_method;
extern DATASET_METHOD dsm_subset_method;
extern DATASET_METHOD dsm_isubset_method;
extern DATASET_METHOD dsm_fifo_method;
extern DATASET_METHOD dsm_union_method;

#endif /* OWNER */

#undef OWNER

/* h2man:skipend */

#ifdef __cplusplus
}
#endif /* __cplusplus */


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#endif /* __DSMETHOD_EXTERN_H__ */
#endif /* NOEXTERN */
