
/* Copyright (c) 1995 by G. W. Flake.
 *
 * NAME
 *   nodelib.h - the Neural Optimization Development Engine library
 * SYNOPSIS
 *   NODElib (which stands for Neural Optimization Development Engine 
 *   library) is a programming library for rapidly developing powerful
 *   neural network simulations.  Very few assumptions have been made
 *   regarding the system in which the code is to be used; thus, 
 *   NODElib is suitable as a back-end engine for most applications.
 *   
 *   But wait, there's more.
 *   
 *   NODElib is general enough that you can probably find many other
 *   uses for it.  The code is extremely modular, compact, and robust.
 *   It is written in an object oriented manner.  All of the library
 *   code, example and test program source, documentation, and
 *   supporting text is only on the order of about 20,000 lines, which
 *   means that NODElib is extremely compact.  This is important from
 *   the point of view of comprehending the code, and for the memory
 *   requirements of the library.
 *   
 *   To use NODElib, you need only use the include directive below,
 *   and compile your program with a '-lnode -lm'.
 * DESCRIPTION
 *   \section{Quick Start}
 *   To get up and running with NODElib as fast as possible, consult
 *   the examples directory, which contains several source code
 *   examples for using NODElib in a variety of ways.  In general,
 *   simple tasks will require simple programs, and complex tasks will
 *   require slightly complex programs.
 *   
 *   The Makefile in the examples directory can also be used as a
 *   template for your own Makefile.
 *   
 *   Keep in mind that the examples attempt to show a wide range of
 *   NODElib functionality; thus, you may do well to take an example
 *   program and strip out the functionality that you do not need
 *   (e.g., advance optimization or data source hooks, etc.).
 *   
 *   The next section gives a bird's eye view of NODElib, with the aim
 *   of giving you a general feel for the entire contents of NODElib.
 *   
 *   \section{Overview}
 *   In general, NODElib is divided into several modules that can be
 *   used more or less independently of each other.  These modules can
 *   be roughly categorized in one of four groups: core routines
 *   (required by all other modules), basic data types, data set
 *   handling routines and interfaces, and numerical routines.  A brief
 *   introduction to each category is provided below.
 *   
 *   \bf{Core Routines} - The core routines handle memory management
 *   and basic exception handling.  The key idea is that localizing
 *   these two tasks simplify debugging and allow for error messages
 *   (for all of NODElib) to be directed to a single source that can
 *   be easily selected by a user.
 *   
 *   \begin{itemize}
 *   
 *     \item \url{MISC}{misc.html} - miscellaneous but useful
 *     routines.  This module includes multidimensional array
 *     allocation and deallocation routines, a command-line parsing
 *     function, and random number generators with uniform and
 *     Gaussian distributions.
 *    
 *     \item \url{ULOG}{ulog.html} - standardized I/O routines with
 *     many features.  Because of this module, there is not a single
 *     plain printf() in all of NODElib.  All screen I/O is passed
 *     through the ULOG package, which means that you can arbitrarily
 *     set how verbose the messages are and where they should go
 *     (i.e., log files, error message window, etc.).
 *   
 *     \item \url{XALLOC}{xalloc.html} - smarter memory allocation.
 *     This module provides memory allocation with automatic error
 *     checking.  It also keeps track of the memory used by each
 *     allocated pointer and the total used for a process.  It
 *     supports debugging with checks for valid frees and gives a
 *     summary of outstanding pointers.
 *   
 *   \end{itemize}
 *   
 *   \bf{Basic Data Types} - The basic data types are used as building
 *   blocks by the other modules.  All of the basic data types in
 *   NODElib are generic in the sense that they accept a void pointer
 *   and a set of user hooks so that they can operate over any other
 *   data type.
 *   
 *   \begin{itemize} 
 *   
 *     \item \url{ARRAY}{array.html} - a generic array type that grows
 *     as needed.  Use this for stacks, queues, or linear lists of any
 *     type.  The bounds will grow transparently, so you never need to
 *     be concerned about hard coded limits.
 *   
 *     \item \url{HASH}{hash.html} - generic but expandable hash
 *     tables.  This module defines a hash table for objects that can
 *     be expressed as a void pointer.  The table size dynamically
 *     doubles in size if a load factor is exceeded, but without
 *     recomputation of the hash keys.  Very nifty.
 *   
 *     \item \url{LIST}{list.html} generic doubly linked lists.  This
 *     module defines a linked list for objects that can be expressed
 *     as a void pointer.  Just the basics are provided.
 *   
 *     \item \url{SCAN}{scan.html} - a simple reentrant text scanner.
 *     These routines only distinguish delimiter characters from
 *     comment characters and white space characters.  In other words,
 *     these scanners are powerful enough to scan languages such as
 *     the Bourne-shell or lisp, but could not handle C language
 *     comments.  However, we can scan character arrays and files
 *     identically.
 *   
 *     \item \url{SERIES}{series.html} - data stream handling.  This
 *     modules allows one to access data in flat-files in a unified
 *     manner.  For time series analysis, a one-dimensional stream of
 *     data is efficiently stored in memory but can still be easily
 *     accessed in terms of delayed coordinates as if it actually
 *     consists of multi-dimensional vectors.  This module can also be
 *     used for numerical pattern classification as well, making it a
 *     suitable data type for neural networks to operate on.
 *   
 *   \end{itemize}
 *   
 *   \bf{Data Set Interfaces} - Most of the numerical methods in
 *   NODElib are defined with respect to an abstract ``dataset''.  In
 *   some applications, you may want a dataset to be a chunk of
 *   memory in your C code, or a flat memory-mapped binary file, or
 *   perhaps a delayed coordinate embedding of ASCII time series data.
 *   NODElib can handle all of these cases because it has an abstract
 *   DATASET type that defines the interface for how data from a
 *   dataset should be accessed.
 *   
 *   The details for how NODElib implements DATASETs may not be too
 *   interesting to you; however, you may wish to scan the list below
 *   to get a feel for the types of DATASETs that are implemented.
 *   The first two modules define the DATASET and DATASET_METHOD
 *   types; the latter type defines what methods are required to
 *   implement the DATASET interface.  All other modules in this list
 *   are specific implementation of DATASET_METHODs.
 *   
 *   \begin{itemize} 
 *   
 *     \item \url{DATASET}{dataset.html} - A generic dataset type with
 *     custom methods.  The data type and routines contained in this
 *     module represent a generic interface that is applicable to a
 *     broad class data sources.  Practically any data source can be
 *     massaged into a DATASET, which means that all of NODElib can
 *     use the types.
 *   
 *     \item \url{DSMETHOD}{dsmethod.html} - methods for various
 *     DATASET instance types.  The DATASET_METHODs defined in this
 *     module can be used to transform other types into DATASETs.
 *     Methods currently in this module include ones for C matrices,
 *     the SERIES type, among others
 *   
 *     \item \url{DSDBLPTR}{dsdblptr.html} - DATASET_METHOD for double
 *     pointer matrix types.  These routines define methods for
 *     accessing matrices that are dynamically allocated as pointers
 *     to pointers of doubles.  With this module, a DATASET can
 *     consist of a single matrix or two matrices.
 *   
 *     \item \url{DSFIFO}{dsfifo.html} - DATASET_METHOD for fixed-size
 *     FIFO..  A FIFO (first in, first out) allows you to hold the
 *     most current training patterns in a fixed size set.  Whenever a
 *     new pattern is added, the oldest pattern is overwritten.  This
 *     is handy for incremental learning.
 *   
 *     \item \url{DSFILE}{dsfile.html} - DATASET_METHOD for flat
 *     binary files.  These routines define methods for accessing
 *     files.  With this module, a DATASET can consist of a large
 *     binary file.  Patterns are loaded on a need-only basis, so
 *     memory is preserved.
 *   
 *     \item \url{DSISUBSET}{dsisubset.html} - DATASET_METHOD subset
 *     type.  Given an existing DATASET, one can define a new subset
 *     of the first data set.  The actual subset is determined by a
 *     user specified vector of indices. This is useful for
 *     incremental techniques applied to huge amounts of data.
 *   
 *     \item \url{DSMATRIX}{dsmatrix.html} - DATASET_METHOD for matrix
 *     types.  These routines define methods for accessing matrices.
 *     With this module, a DATASET can consist of a single matrix or
 *     two matrices.
 *   
 *     \item \url{DSSUBSET}{dssubset.html} - DATASET_METHOD subset
 *     type.  Given an existing DATASET, one can define a new subset
 *     of the first data set.  The actual subset is determined by a
 *     user specifed range.  This is useful for incremental techniques
 *     applied to huge amounts of data.
 *   
 *     \item \url{DSUNION}{dsunion.html} - DATASET_METHOD union of
 *     other DATASETs.  Given multiple existing DATASETs, one can
 *     define a new DATASET that is the union of original ones.  This
 *     is useful for when multiple data sources have to be
 *     conceptually merged (e.g., training on multiple files as if
 *     theory were one).
 *   
 *   \end{itemize}
 *   
 *   \bf{Numerical Routines} - The numerical routines are the heart of
 *   NODElib.  If you are reading this, then you probably downloaded
 *   NODElib specifically for the neural network or support vector
 *   machine code.  In this case, jump directly to those packages.
 *   Otherwise, you may wish to skim the contents below.
 *   
 *   \begin{itemize} 
 *   
 *     \item \url{DENSE}{dense.html} - basic density estimation
 *     routines.  Given a DATASET, the routines in this package allow
 *     you to construct a simple density estimator with the kernel
 *     function of your choice.
 *   
 *     \item \url{ERRFUNC}{errfunc.html} - error functions for
 *     optimization routines.  Error functions currently in this
 *     module include the standard Quadratic, the logistic error
 *     function, and Huber's error function.
 *   
 *     \item \url{KMEANS}{kmeans.html} - perform k-means clustering on
 *     a DATASET.  The routines in this module will compute clusters
 *     of a DATASET with the k-means clustering algorithm.  The
 *     resulting clusters are returned in another DATASET.
 *   
 *     \item \url{NN}{nn.html} - a generic neural network package.
 *     The neural network package allows for generic feedforward
 *     neural networks to be connected with arbitrary activation
 *     functions, net input functions, connections types, error
 *     functions, and learning algorithms.
 *   
 *     \item \url{OPTIMIZE}{optimize.html} - numerical optimization
 *     routines.  This module provides a generic and uniform interface
 *     to optimization routines that can be used on a wide variety of
 *     problems.  This package currently includes multi-dimensional
 *     search algorithms (steepest descent, conjugate gradient, and
 *     quasi-Newton) and line search routines (cubic interpolation,
 *     golden section, and a hybrid of the first two).
 *   
 *     \item \url{SVD}{svd.html} - singular value decomposition and
 *     friends.  Included here is a basic SVD call and some basic
 *     applications of the SVD, such as pseudo matrix inversion and
 *     principal component analysis.
 *   
 *     \item \url{SVM}{svm.html} - support vector machines and the
 *     SMORCH algorithm..  This package defines support vector
 *     machines (SVMs) for both classification and regression
 *     problems.  The SVMs can use a wide variety of kernel functions.
 *     Optimization of the SVMs is performed by a variation of John
 *     Platt's sequential minimal optimization (SMO) algorithm.  This
 *     version of SMO is generalized for regression, uses kernel
 *     caching, and incorporates several heuristics; for these
 *     reasons, we refer to the optimization algorithm as SMORCH.
 *     SMORCH has been shown to be over an order magnitude faster than
 *     SMO, QP, and decomposition.
 *   
 *   \end{itemize}
 * 
 * EXAMPLES
 *   NODElib contains several example programs in the examples
 *   subdirectory of the source distribution.  The examples do not
 *   exhaustively show how to use every feature of NODElib but, in
 *   general, they should help a new user quickly come up to speed
 *   with the library.
 * 
 *   A partial list of example programs includes:
 *   \begin{itemize}
 *     
 *     \item \url{\bf{aa.c}}{../examples/aa.c} - Train a NN
 *     auto-associator.  Can use a variety of optimization routines.
 *     Data is taken from an ASCII file.
 *     
 *     \item \url{\bf{classify.c}}{../examples/classify.c} - Train a
 *     NN classifier.  Can use a variety of optimization routines.
 *     Data is taken from an ASCII file.
 *     
 *     \item \url{\bf{cluster.c}}{../examples/cluster.c} - An example
 *     of using the k-means code.
 *     
 *     \item \url{\bf{cnls.c}}{../examples/cnls.c} - Shows how to
 *     build an exotic NN architecture.  In this case, a CNLS network
 *     is built with sublayers, a Euclidean net input function, a
 *     pairwise product net input function, and other NN features.
 *     This also shows how to build a SERIES DATASET on the fly.
 *     
 *     \item \url{\bf{dwjacob.c}}{../examples/dwjacob.c} - This
 *     program tests the J-prop features NODElib.  After training an
 *     auto-associator, a function of the Jacobian matrix is
 *     differentiated numerically and analytically, so that the
 *     results can be compared.
 *     
 *     \item \url{\bf{hopfield.c}}{../examples/hopfield.c} - This
 *     convoluted example shows how to build a Hopfield network with
 *     NODElib.  I don't recommend using NODElib in this way, but only
 *     wish to show that that static feedback networks are possible.
 *     
 *     \item \url{\bf{hwnn.c}}{../examples/hwnn.c} - Hello World for
 *     NN.  This is the simplest example of how to use an NN.
 *     However, this example does use short-circuit links, so it is
 *     slightly advanced.
 *     
 *     \item \url{\bf{optother.c}}{../examples/optother.c} - This
 *     example shows how to use NODElib's fancy command-line parsing
 *     functions.
 *     
 *     \item \url{\bf{rbf.c}}{../examples/rbf.c} - Constructs an RBFN
 *     on logistic map data.  The training is ``single step'' in that
 *     it uses the k-means and a SVD to optimize the weights.
 *     
 *     \item \url{\bf{search.c}}{../examples/search.c} - Shows how to
 *     use NODElib's line search routines to find the minimum of a
 *     scalar function.
 *     
 *     \item \url{\bf{smorch.c}}{../examples/smorch.c} - This is a
 *     more advanced version of John Platt's sequencial minimal
 *     optimization (SMO) code, called SMORCH.  It uses caching, many
 *     heuristics, and is generalize for regression.  See the SVM
 *     documentation for a complete description, as this is the one
 *     example that is meant to stand on its own.
 *     
 *     \item \url{\bf{smlp.c}}{../examples/smlp.c} - Similar to rbf.c,
 *     but uses an SMLP.
 *     
 *     \item \url{\bf{xor.c}}{../examples/xor.c} - The fruit-fly of
 *     neural networks.
 *     
 *     \item \url{\bf{xorhess.c}}{../examples/xorhess.c} - Trains a NN
 *     on XOR data, analytically and numerically calculates the
 *     Hessian matrix, and compares the results.
 *     
 *   \end{itemize}
 * 
 * TEST PROGRAMS
 *   NODElib has a basic set of test programs in the test directory.
 *   These test programs aren't particularly thorough; they are just
 *   there for testing minor new features.  Nevertheless, you may
 *   find some of the useful, as they tend to exercise little known
 *   features of the library.
 *   
 * REFERENCES

 *   While NODElib is lacking formal user and developer manuals (other
 *   than this online documentation), there exists a few publications
 *   that describe some of the more subtle features of NODElib.  In no
 *   particular order:
 *   \begin{itemize}
 *   
 *     \item G. W. Flake and S. Lawrence.  Efficient SVM Regression
 *     Training with SMO. Submitted to \it{Machine Learning}, 2000.
 *   
 *     \item G. W. Flake.  The Calculus of Jacobian Adaptation.
 *     Submitted to \it{Neural Computation}, 2000.
 *   
 *     \item G. W. Flake and B. A. Pearlmuter.  Differentiating
 *     Functions of the Jacobian with Respect to the Weights.  In
 *     S. A. Solla, T. K. Leen, and K.-R. Müller, editors, 
 *     \it{Advances in Neural Information Processing Systems}, volume 12. The MIT
 *     Press, 2000.
 *   
 *     \item G. W. Flake.  Square Unit Augmented, Radially Extended,
 *     Multilayer Perceptrons.  In G. Orr, K.-R. Müller, and R. Caruana,
 *     editors, \it{Tricks of the Trade: How to Make Algorithms Really}
 *     \it{Work},  LNCS State-of-the-Art-Surveys. Springer-Verlag, 1998.
 *   
 *   \end{itemize}
 * 
 * DOWNLOAD
 *   The unofficial home page of NODElib is located at:
 *     \qurl{http://flakentstein.net/nodelib/html}.
 * 
 *   The latest version of NODElib can be downloaded from:
 *     \qurl{http://flakentstein.net/lib/nodelib.tgz}.
 * 
 *   The author's home page is:
 *     \qurl{http://flakenstein.net/}.
 * 
 * COPYING
 *   Copyright (c) 1992-2005 by Gary William Flake.
 *   
 *   NODElib is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published
 *   by the Free Software Foundation; either version 2 of the License,
 *   or (at your option) any later version.
 *   
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *   
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * AUTHOR
 *   Gary William Flake (\url{\bf{gary.flake@usa.net}}{mailto:gary.flake@usa.net}).
 */

#ifndef __NODELIB_H__
#define __NODELIB_H__

#include "nodelib/array.h"
#include "nodelib/dataset.h"
#include "nodelib/dense.h"
#include "nodelib/dsdblptr.h"
#include "nodelib/dsfifo.h"
#include "nodelib/dsfile.h"
#include "nodelib/dsisubset.h"
#include "nodelib/dsmatrix.h"
#include "nodelib/dsmethod.h"
#include "nodelib/dssubset.h"
#include "nodelib/dsunion.h"
#include "nodelib/errfunc.h"
#include "nodelib/hash.h"
#include "nodelib/kmeans.h"
#include "nodelib/list.h"
#include "nodelib/misc.h"
#include "nodelib/nn.h"
#include "nodelib/optimize.h"
#include "nodelib/scan.h"
#include "nodelib/series.h"
#include "nodelib/svd.h"
#include "nodelib/svm.h"
#include "nodelib/ulog.h"
#include "nodelib/xalloc.h"

#endif /* __NODELIB_H__ */

