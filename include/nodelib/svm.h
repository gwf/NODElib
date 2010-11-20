
/* Copyright (c) 1998 by G. W. Flake.
 *
 * NAME
 *   svm.h - support vector machines and the SMORCH algorithm.
 * SYNOPSIS
 *   This package defines support vector machines (SVMs) for both
 *   classification and regression problems.  The SVMs can use a wide
 *   variety of kernel functions.  Optimization of the SVMs is
 *   performed by a variation of John Platt's sequential minimal
 *   optimization (SMO) algorithm.  This version of SMO is generalized
 *   for regression, uses kernel caching, and incorporates several
 *   heuristics; for these reasons, we refer to the optimization
 *   algorithm as SMORCH.  SMORCH has been shown to be over an order
 *   magnitude faster than SMO, QP, and decomposition.
 * DESCRIPTION
 *   \section{Quick Start}
 *   If you want to use SVMs and SMORCH, but don't want to think about
 *   it too much, read this section, read about the mandatory fields
 *   of the SMORCH data type, skip ahead to the
 *   \url{HEURISTICS}{svm.html#HEURISTICS} section to learn about the
 *   runtime options, and use the demo program, \bf{smorch}, which
 *   should be in the examples subdirectory of the NODElib
 *   distribution.
 *   
 *   You only need to consult this entire document if you wish to
 *   modify the internals of this package for research purposes.
 *   
 *   The quickest way to get going with this package is to just use
 *   the \bf{smorch} program.  Essentially, the \bf{smorch} program is
 *   nothing but a convenient wrapper around these routines.  It
 *   accepts a number of command-line options for selecting where your
 *   training data comes from, and for selecting how the optimization
 *   procedure should behave.
 *   
 *   The optimization parameters are summarized under the SMORCH data
 *   type and in the \url{HEURISTICS}{svm.html#HEURISTICS} section.
 *   The remainder of this Quick Start subsection will only describe
 *   the command line options of \bf{smorch} not covered elsewhere in
 *   this document.
 *   
 *   In general, you can have \bf{smorch} read data from an ASCII file
 *   or a binary file.  ASCII files work just as you would think;
 *   Numbers can look like anything that scanf() can read, and they
 *   can be broken by whitespace in any manner you choose.  The
 *   command line options \bf{-xdim} and \bf{-ydim} specify the input
 *   and output dimensionality, respectively.  Lines that begin with
 *   "^ *#" are interpreted as comments and are skipped.
 *   
 *   Binary files are a little more subtle and behave as follows:
 *   \begin{itemize}
 *     \item \bf{-dtype double} -  your data file must be a flat
 *     binary file of doubles.
 *   
 *     \item \bf{-dtype short} - your data file is a flat binary file
 *     of shorts, but the values will be divided by 1000.0.  This is
 *     useful if you have a huge dataset and you want to sacrifice
 *     precision for quantity.
 *   
 *     \item \bf{-dtype map} - your data file is a flat binary file of
 *     doubles.  However, this data set will not be read into memory.
 *     Instead, the data will be memory mapped from the binary file.
 *     This is useful if you have a gigabyte sized data set.
 *   \end{itemize}
 *   In all cases for binary files, the \bf{-xdim} and \bf{-ydim} 
 *   options behave just as they do for ASCII files.  For all file types,
 *   you'll want to use \bf{-yindex UINT} as an option if you are
 *   training on data sets that have multiple outputs.
 *   
 *   If you are training on a regression file, and you are reading
 *   data from an ASCII file, you can use \bf{-xdelta UINT} and
 *   \bf{-offset UINT} to specify a delayed coordinate embedding of
 *   the data.  This is A Good Thing because it preserves memory, but
 *   it forces the input vectors to be constructed at runtime each
 *   time they are used.  \bf{-xdelta UINT} specifies the space
 *   between successive inputs in the time series, and \bf{-offset UINT} 
 *   specifies the distance between the last input and the single target 
 *   output value in the time series.  See the SERIES type for more
 *   information on how to interpret these values for time series data.
 *   
 *   Finally, you can use \bf{-dump} to force the program to dump an
 *   output file named \bf{smorch.tst} which contains the results of
 *   testing the trained SVM on the training set.  Each line in the
 *   dump file will contain the input vector, the target output, and
 *   the SVM output.
 *   
 *   When in doubt, run \bf{smorch -help} for a list of command line
 *   options.
 *  
 * HEURISTICS
 *   SMORCH uses several tricks to speed up convergence.  Most of the
 *   heuristics are summarized in a recent paper:
 *   \begin{itemize}
 *     \item G. W. Flake and S. Lawrence.  Efficient SVM Regression
 *     Training with SMO. Submitted to \it{Machine Learning}, 2000.
 *     [ \url{PostScript}{http://www.neci.nj.nec.com/homepages/flake/smorch.ps} ]          
 *   \end{itemize}
 *   The main trick is that kernel outputs are cached.  However, a
 *   simple LRU policy fails miserably, so our caching policy works
 *   around this by effectively turning off cache "tickles" in certain
 *   situations.  The other heuristics are designed to exploit the
 *   fact that kernel outputs are cached by reducing the cost of each
 *   SMORCH step, or by reducing the total number of steps. 
 *   
 *   The heuristics are summarized below.  The item label in the list
 *   corresponds to the field in the SMORCH data structure that must
 *   be set in order to activate the heuristic.  The end of each
 *   heuristic description also lists the command line option in the
 *   \bf{smorch} program that activates the heuristic.
 *   \begin{itemize}
 *     \item \bf{ultra_clever} - Instead of incrementally updating all
 *     of the SVM outputs every time a Lagrange multiplier changes,
 *     this option forces the changed values of the multipliers to be
 *     saved over time.  When an SVM output is required, the code
 *     looks to see if it is faster to sum up the changes, or to sum
 *     up the non-zero multipliers.  Which ever method is faster is
 *     then done on demand.  Thus, we can exploit statistical
 *     irregularities found when only a small set of exemplars are
 *     ever considered.  (\bf{-clever} in \bf{smorch}).
 *   
 *     \item \bf{best_step} - With this option, instead of using
 *     Platt's second choice heuristic for choosing the second
 *     multiplier to optimize, we explicitly evaluate how using each
 *     candidate for the second multiplier reduces the objective
 *     function.  For cached exemplars, this can be done in constant
 *     time for each candidate.  Thus, it's fast with caching and it
 *     truly picks the second multiplier that gives the greatest gain.
 *     Overall, this seems to reduce the number of SMORCH steps and
 *     epochs required to reach convergence.  (\bf{-best} in
 *     \bf{smorch}).
 *   
 *     \item \bf{lazy_loop} - In the original SMO, if no second
 *     multiplier can be found in the working set, then SMO searches
 *     over all multipliers.  This is bad for several reasons: (1) it
 *     messes up a cache (if you are using one with a strict LRU
 *     policy) and (2) it's not really necessary unless you are
 *     testing for convergence.  With this option, we only test all
 *     multipliers if we need to check for convergence.  (\bf{-lazy}
 *     in \bf{smorch}).
 *   
 *     \item \bf{subset_size} - For large data sets, you can get a
 *     huge gain in speed by setting this equal to your cache size.
 *     Here's what happens: Instead of just solving a bunch of 2 x 2
 *     subproblems, this option breaks the entire QP problem into an N
 *     x N subproblem, which is then decomposed into multiple 2 x 2
 *     subproblems.  Thus, this is a form of recursive decomposition.
 *     The net effect is that you get a gain in convergence speed from
 *     constraining the problem in this manner, but you also make the
 *     cache easier to maintain.  The current manner of constructing
 *     the N x N subproblem is very ad hoc, i.e., it is far from
 *     optimal; however, this still seems to work well.  (\bf{-ssz
 *     UINT} in \bf{smorch}).
 *   
 *     \item \bf{worst_first} - Don't use this.  It doesn't work.
 *     This anti-heuristic attempts to optimize multipliers that break
 *     the KKT condition by the greatest amount.  When I was first
 *     learning about SVMs, this seemed like a good idea; in reality,
 *     this makes the optimization code concentrate on the noise will
 *     ill results.  (\bf{-wfirst} in \bf{smorch}).
 *   \end{itemize}
 *   
 *   If you have looked closely at the source code, you may have seen
 *   references to some tube shrinking heuristics.  This is
 *   essentially a poor man's homotopy.  It's a work in progress, so
 *   ignore those options.
 * SMORCH EXAMPLES
 *   Suppose you have an ASCII data file consisting of 1000 exemplars
 *   with each exemplar consisting of 100 input features followed by
 *   the target class.  The suggested way of solving this problem
 *   would be to use:
 *   \begin{itemize}
 *   \item smorch -kernel linear -xdim 100 -fname MYFILE -C 10
 *                -cs 1000 -clever -best -lazy
 *   \end{itemize}
 *   The above command will use linear kernels.  However, if you wanted
 *   to use Gaussian kernels with a variance of 0.5, you would use:
 *   \begin{itemize}
 *   \item smorch -kernel gauss -aux 0.5 -xdim 100 -fname MYFILE -C 10
 *                -cs 1000 -clever -best -lazy
 *   \end{itemize}
 *   Now, imagine that everything is the same as above, but now you it
 *   is a regression problem that you are working on.  Then, you would
 *   use:
 *   \begin{itemize}
 *   \item smorch -kernel gauss -aux 0.5 -xdim 100 -fname MYFILE -C 10
 *                -cs 1000 -clever -best -lazy -regress -regeps 0.1
 *   \end{itemize}
 *   Finally, if you had on the order of 10,000 exemplars instead, a
 *   better set of options would be:
 *   \begin{itemize}
 *   \item smorch -kernel gauss -aux 0.5 -xdim 100 -fname MYFILE -C 10
 *                -cs 1000 -clever -best -lazy -regress -regeps 0.1
 *                -ssz 200
 *   \end{itemize}
 *   When in doubt, use all of the recommended heuristics except the
 *   command line option \bf{-ssz UINT}.  If you are working with a
 *   huge dataset, then use \bf{-ssz UINT}
 * TO DO
 *   In an ideal world, the SVM and NN packages would be unified in
 *   such a way that an SVM is just another NN that is a linear
 *   combination of kernels.  In this framework, SMORCH would just
 *   be another algorithm in the OPTIMIZER package.  Some day,
 *   NODElib will be restructured in this way.  But not very soon.
 * BUGS
 *   This is highly experimental code and it is the subject of active
 *   research.  As such, there are probably numerous bugs.  Moreover,
 *   this package may be radically changed in the future.
 * AUTHOR
 *   Gary William Flake (\url{\bf{gary.flake@usa.net}}{mailto:gary.flake@usa.net}).
 */


#ifndef __SVM_H__
#define __SVM_H__

#include <stdio.h>
#include <stdlib.h>

#include "nodelib/etc/version.h"
#include "nodelib/etc/options.h"

#include "nodelib/list.h"
#include "nodelib/dataset.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* The SVM data type contains the minimal amount of information required
   to define a support vector machine.  You probably do not need to
   know about the internals of this data type because most of what
   you may want to do with an SVM can be done with one of the functions
   in this package.   Morevover, you never actually create an SVM
   yourself because it is the return type of \bf{smorch_train()}. */

typedef struct SVM {
  unsigned xdim;          /* The input dimensionality.        */
  unsigned sz;            /* The number of support vectors.   */
  unsigned regression;    /* Is this a regression SVM?        */
  double b;               /* The output bias value.           */
  double *alpha;          /* The Lagrange multipliers.        */
  double **x;             /* The input vectors.               */
  double *y;              /* The target output values.        */
  double aux;             /* Auxiliary parameter for kernels. */
  /*
   * The kernel function.
   */
  double (*kernel)(double *x1, double *x2,
		   double aux, int dim);
} SVM;



/* The SMORCH_CACHE data type is an internal structure used to save kernel
   outputs for execution efficiency.  If your cache is set up to have
   a size of N, then this data structure will hold an N x N matrix
   of kernel values.   Again, you probably don't need to look at this
   data structure at all; the information here is only for the curious. */

typedef struct SMORCH_CACHE {
  double **vals;      /* The cached values. */  
  char **status;      /* Indicates if a value is already computed. */
  unsigned *index;    /* The cache index for some exemplar. */
  unsigned *invindex; /* The dataset index of a cache index. */
  unsigned size;      /* The size of the cache. */
  unsigned used;      /* The number of cache cells used. */
  unsigned limit;     /* The number of dataset exemplars. */
  unsigned notickle;  /* When set, prevents changes in the cache. */
  LIST *order;        /* The LRU cache elements. */
  LIST *free;         /* A list of unused cache nodes. */
  LIST_NODE *nodes;   /* The array of cache nodes. */
} SMORCH_CACHE;


/* The SMORCH data type is used to completely define how calls to
   \bf{smorch_train()} behaves.  If you want to use this package in a
   sophisticated manner, then you need to pay attention to the fields
   marked as \bf{MANDATORY} or \bf{OPTIONAL}.  (If you are using the
   SMORCH data type at this level, then you are probably not using the
   included \bf{smorch} example program.)

   Since you are still reading this, you probably want to know how to
   use this data type in greater detail.  Here's the scoop.  The first
   thing you should do is to declare a SMORCH in your code.  Do this
   so that you have sane initial values for the fields with:
   \begin{verbatim}
       SMORCH smorch = SMORCH_DEFAULT;
   \end{verbatim}
   After this declaration, you can set the fields of \em{smorch} to
   your desired values.  The only thing that you must set is the source
   of your training data:
   \begin{verbatim}
       smorch.data = my_data_set;
   \end{verbatim}
   where \em{my_data_set} is a proper DATASET.  After that you can call:
   \begin{verbatim}
       SVM *svm = smorch_train(&smorch);
   \end{verbatim}
   And that's it!  The returned result in \em{svm} contains all of your
   support vectors and optimal Lagrange multipliers.  With \em{svm}, you
   can evaluate your SVM with respect to an arbitrary input, or save
   everything to disk for later use.

   However, using a SMORCH in this manner will probably give you
   undesirable results.  To really use a SMORCH, you need to set the
   OPTIONAL runtime parameters to reasonable values.  A short list of
   things that you'll want to think about are:
   \begin{itemize}

     \item \em{smorch.kernel} - right now, this can be Gaussian,
     polynomial, hyperbolic tangent, or linear.  To add your own
     kernel function, look at the source file \bf{svmkern.c}.

     \item \em{smorch.C} - the maximum allowed magnitude of a Lagrange
     multiplier.  This reflects how many misclassification you will
     allow your SVM to have.  Small values means generalization is
     better but at the expense of the expressiveness of the SVM.  An
     infinite value means that you have noise free data that can be
     classified with your choice of the kernel.

     \item \em{smorch.aux} - The auxiliary parameter for the kernel
     function.  See the kernel functions for how this should be
     interpreted.

     \item \em{smorch.regression} - Set this to non-zero if you are
     optimizing a regression problem.

     \item \em{smorch.regeps} - This is the value of epsilon for the
     epsilon insensitive regression error function.

     \item \em{smorch.yindex} - If \em{smorch.dataset} has more than
     one output, and you want to train on something besides the first
     output component, then set this to the index of the desired
     target values.  (0 is the first output.)

     \item \em{smorch.eps} - This is you machine epsilon, more or
     less.  I fudge things a little, so it isn't quite that, but don't
     worry.

     \item \em{smorch.tol} - The output tolerance.  if you set this to
     0.1, then the optimization code will consider 0.9 to be equal to
     1.0.  This is good to for speeding things up.  The default is
     0.001.

     \item \em{smorch.cache_size} - If you have \em{N} exemplars in
     your dataset, and you can afford to store an \em{N^2} matrix, then
     set this to \em{N}.  Otherwise, set it to whatever memory size
     you can afford.  This will \bf{significantly} speed up
     convergence.

     \item \em{smorch.hook} - Set this to a function that you want to
     call after every \bf{smorch_train()} epoch.  For example, you may
     want to print the epoch number and the value of the objective
     function. 
   \end{itemize}
   
   There are a few other optimization options.  These are covered in
   the \url{HEURISTICS}{svm.html#HEURISTICS} section. */

typedef struct SMORCH {
  /* 
   * The following fields define the basic structure of the problem
   * and the data set.
   */

  /* The input dimensionality (automatically set). */
  unsigned xdim;
  /* The number of exemplars (automatically set). */
  unsigned sz;
  /* MANDATORY: the training dataset. */
  DATASET *data;
  /* OPTIONAL: the kernel function. */
  double (*kernel)(double *x1, double *x2,
		   double aux, int dim);

  /*
   * The next set of fields are basic optimization options.
   */

  /* OPTIONAL: the size of the cache (remember, this is squared). */
  unsigned cache_size; 
  /* OPTIONAL: the output in the dataset that is the target. */
  unsigned yindex;
  /* OPTIONAL: if greater than zero, the decomposition subset size. */
  unsigned subset_size;
  /* OPTIONAL: the box constraint for Lagrange multipliers. */
  double C;
  /* OPTIONAL: classification tolerance value. */
  double tol;
  /* OPTIONAL: machine floating point epsilon. */
  double eps;
  /* OPTIONAL: the auxiliary parameter for the kernel functions. */
  double aux;

  /*
   * Regression specific fields:
   */

  /* OPTIONAL: is this a regression problem? */
  unsigned regression;
  /* OPTIONAL: value for the epsilon-insensitive regression loss function. */
  double regeps;

  /*
   * The next four fields are special training options.
   */

  /* OPTIONAL: use on-demand incremental SVM output calculation? */
  unsigned ultra_clever;  
  /* OPTIONAL: look for the best pairing in the cache? */
  unsigned best_step;
  /* OPTIONAL: attempt to optimize exemplars that violate KKT conditions
   * by the greatest margin.  This is a bad idea.
   */
  unsigned worst_first;
  /* OPTIONAL: avoid thrashing problem. */
  unsigned lazy_loop;

  /*
   * The next set of fields are very experimental.  Don't use them.
   */

  /* OPTIONAL: use a shrinking tube for machine tolerance? */
  unsigned tube;
  /* OPTIONAL: shrinking factor for each tube iteration. */
  double   tube_rate;
  /* OPTIONAL: value to halt shrinking tube.  Don't ask.  I can't explain it. */
  double   tube_final;

  /*
   * The next set of fields are hooks that you can set to do something
   * periodically.
   */

  /* OPTIONAL: a hook that is called on every SMORCH iteration. */
  int (*hook)(struct SMORCH *smorch);
  /* OPTIONAL: the hook that is called when SMORCH is finished. */
  int (*finalhook)(struct SMORCH *smorch);
  /* OPTIONAL: a field that you can pass your own data to. */
  void *obj;
  
  /*
   * The epoch number, the number of Lagrange multipliers changed at
   * the last SMORCH epoch, and the last known value of the objective functions.
   */
  unsigned epoch, num_changed;
  double objective;

  /*
   * The SVM output bias, the Lagrange multipliers, and the target
   * outputs.
   */
  double b;
  double *alpha;
  double *y;

  /* 
   * The actual SVM outputs (if known), the kernel values of
   * K(x_i,x_i), and some temporary scratch space.
   */
  double *out;
  double *kii;
  double *temp;

  /*
   * The next two arrays allow us to index directly into the two lists
   * that follow.
   */
  LIST_NODE **nonzero_node;
  LIST_NODE **nonbound_node;

  /*
   * The list of nonzero Lagrange multipliers, the list of non-bounded
   * Lagrange multipliers, and a free list of nodes.
   */
  LIST *nonzero;
  LIST *nonbound;
  LIST *freelist;

  /* The cache structure used for this SMORCH instantiation. */
  SMORCH_CACHE *cache;

  /* Index arrays for random sampling and sub-sampling. */
  unsigned *random_index;
  unsigned *sub_index;

  /* 
   * These fields are used for on-demand incremental SVM output
   * calculation.  They store incremental changes to the Lagrange
   * multipliers so that SVM output evaluation can use the fastest
   * method possible.
   */
  unsigned time;
  unsigned *alpha_index;
  unsigned *update_time;
  double   *delta_alpha;
  double   *store_b;

  /* This is useless.  it just stores the number of kernel evaluations
   * required for the last SVM output evaluation.
   */
  unsigned total_output_kern;

  /* This is used internally to force SMORCH to consider all exemplars
   * when looking for something to optimize.
   */
  unsigned examine_all;

  /* Some cache statistics. */
  unsigned cache_hit, cache_miss, cache_avoid;

  /* Just as the name says. */
  unsigned prev_examine_all;

  /* Internal field used to indicate if input vectors need to be
   * copied.
   */
  unsigned safe_ds_pointers;
} SMORCH;

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* Call the SMORCH algorithm as specified by the \em{smorch} argument,
   and return an optmized SVM.  See the documentation for the SMORCH
   data type, and the \url{HEURISTICS}{svm.html#HEURISTICS} section for 
   more details. */

SVM *smorch_train(SMORCH *smorch);


/* Evaluates the objective function of the SVM being optimized by this
   SMORCH instance.   Note that this function sums over all non-zero
   Lagrange multipliers, so it can be very slow.  Instead, you can peak
   at \em{smorch->objective} inside of a hook, which will give you
   the objective function value that has been incrementally updated.
   This latter method is preferred because it takes constant time. */

double smorch_objective(SMORCH *smorch);


/* Evaluates the linear constraints of the SVM being optimized by this
   SMORCH instance.  At all times, the constraints should be obeyed
   within machine tolerance.  You should only call this if you doubt
   that last statement because this function must sum over all
   non-zero Lagrange multiplier, which can take a considerable amount
   of time. */

double smorch_constraint(SMORCH *smorch);


/* Evaluates \em{svm} with the input vector \em{x}.  Make sure that
   \em{x} is of a dimensionality that is identical to the input
   dimensionality of the training set used to optimize \em{svm}. */

double svm_output(SVM *svm, double *x);


/* Writes a description of \em{svm} to the FILE pointer referred to by
   \em{fp}.   The input functions \bf{svm_read_fp()} and \bf{svm_read()}
   can be used to read in the SVM at a later time. */

void svm_write_fp(SVM *svm, FILE *fp);


/* Writes a description of \em{svm} to the file named by \em{fname}.
   The input functions \bf{svm_read_fp()} and \bf{svm_read()} can be
   used to read in the SVM at a later time. */

int svm_write(SVM *svm, char *fname);


/* Reads and returns an SVM that resides in the FILE pointer referred
   to by \em{fp}.  */

SVM *svm_read_fp(FILE *fp);


/* Reads and returns an SVM that resides in the file named by
   \em{fname}.  */

SVM *svm_read(char *fname);


/* Frees all memory associated with \em{svm}. */

void svm_destroy(SVM *svm);


/* A linear kernal defined as \em{x1 . x2}. */

double svm_kernel_linear(double *x1, double *x2, double aux, int dim);


/* A Gaussian kernel defined as \em{exp(-||x1-x2||^2 / aux^2)}. */

double svm_kernel_gauss(double *x1, double *x2, double aux, int dim);


/* A polynomial kernal defined as \em{(x1 . x2 + 1)^aux}. */

double svm_kernel_poly(double *x1, double *x2, double aux, int dim);


/* A hyperbolic tangent kernel defined as \em{tanh(x1 . x2 - aux)}. */

double svm_kernel_tanh(double *x1, double *x2, double aux, int dim);


/* A Coulomb  kernel defined inspired from physics. */

double svm_kernel_coulomb(double *x1, double *x2, double aux, int dim);


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifdef SVM_OWNER
#undef SVM_OWNER

const SMORCH SMORCH_DEFAULT = {
  0, 0,
  NULL, svm_kernel_linear, 0, 0, 0,
  10, 10e-3, 10e-10, 0.5,

  0, 0.01,
  0, 0, 0, 0, 0,
  2.0, 10.0,

  NULL, NULL, NULL,
  /* - - - */
  0, 0, 0.0, 0.0, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL,
  0, NULL, NULL, NULL, NULL,
  0, 0,
  0, 0, 0, 0,
  0
};

#else

extern SMORCH SMORCH_DEFAULT;

#endif

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __SVM_H__ */
