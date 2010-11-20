
/* Copyright (c) 1995 by G. W. Flake.
 *
 * NAME
 *   nn.h - a generic neural network package
 * SYNOPSIS
 *   The neural network package allows for generic feedforward neural
 *   networks to be connected with arbitrary activation functions, net
 *   input functions, connections types, error functions, and
 *   learning algorithms.
 * DESCRIPTION
 *   \section{Quick Start}
 *   Function calls in the \bf{nn}(3) package can be categorized
 *   into three classes: those used by all users, those used by advanced
 *   users, and those used by developers who wish to extend the
 *   functionality of the library.  With this in mind, knowing how you
 *   intend to use this package will allow you to skip a large portion
 *   of this document.  In fact, all but the most advanced developers
 *   can skip the type declaration section.  For each of the three types
 *   of users the following list should help you find what you need
 *   in this document.
 *   
 *   \begin{itemize}
 *   \item \bf{User functions:}
 *      \begin{itemize}
 *        \item nn_create()
 *        \item nn_create_rbf()
 *        \item nn_create_smlp()
 *        \item nn_link()
 *        \item nn_set_actfunc()
 *        \item nn_init()
 *        \item nn_train()
 *        \item nn_write()
 *        \item nn_write_binary()
 *        \item nn_read()
 *        \item nn_destroy()
 *        \item nn_shutdown()
 *      \end{itemize}
 *   \item \bf{Advanced Functions:}
 *      \begin{itemize}
 *        \item nn_solve()
 *        \item nn_Hv()
 *        \item nn_hessian()
 *        \item nn_offline_hessian()
 *        \item nn_jacobian()
 *      \end{itemize}
 *   \item \bf{Developer Functions:}
 *      \begin{itemize}
 *        \item nn_forward()
 *        \item nn_backward()
 *        \item nn_offline_test()
 *        \item nn_offline_grad()
 *        \item nn_register_actfunc()
 *        \item nn_register_netfunc()
 *      \end{itemize}
 *   \end{itemize}
 *
 *   \section{Overview}
 *   Before describing the remaining low-level details of the \bf{nn}(3)
 *   package, an intutitive definition of a NN (neural network object)
 *   is in order.  A NN consists of one or more layers of nodes.  The
 *   First layer is the input layer while the last layer is the
 *   output layer.  In a feedforward neural network computations are
 *   peformed in a bottom-up manner, starting with the input layer
 *   and ending with the output layer.
 *   
 *   Layers (a NN_LAYER object) can consist of slabs (sublayers).
 *   In actuality, slabs have the same internal structure as layers do,
 *   but this fact is just an implementation issue and it is not too
 *   important for typical users of this package to note the distinction.
 *   Dividing a layer into one or more slabs allows the user to select a
 *   different activation function (a NN_ACTFUNC object) on per slab basis,
 *   or to make links (a NN_LINK object) to only a subset of the nodes in
 *   a layer.  When the input or output layer of a NN has more than one
 *   slab then the first slab is considered the input (or output) of that
 *   layer.
 *   
 *   In the most popular type of neural network arhitecture, a multilayer
 *   perceptron, links between layers simply define linear functions
 *   from the outputs of a source layer to the inputs of a destination
 *   layer.  This is the default behavior for this package as well;  however,
 *   in this package many other types of functions from a vector-space to a
 *   scalar can be used as well.  Thus, it is possible to construct a NN
 *   which behaves like a radial basis function network, a higher-order
 *   network, a Gaussian mixture of experts, or just about about any other
 *   type of approximation technique that can be viewed from a framework of
 *   composing many functions of vector-spaces to scalar-spaces.  In this
 *   documentation we refer to the functions that map vectors to scalars
 *   as \em{net functions} (NN_NETFUNC).
 *   
 *   The generic nature of the \bf{nn}(3) package is further enhanced
 *   by its extensibility.  New activation functions and net functions can
 *   be added to the package without recompiling the whole library through
 *   two ``register'' functions described towards the end of this
 *   document.
 *   
 *   This next section will described all of the data types in complete
 *   detail.  Most users will only ever need to know about the NN
 *   and the NN_TRAININFO types;  however, we give complete descriptions
 *   for those who may extend the library by adding activation funtions,
 *   net functions, error functions, and learning algorithms.
 * AUTHOR
 *   Gary William Flake (\url{\bf{gary.flake@usa.net}}{mailto:gary.flake@usa.net}).
 * SEE ALSO
 *   \bf{optimize}(3), \bf{errfuncs}(3), 
 *    and \bf{dataset}(3).
 */

#ifndef __NN_H__
#define __NN_H__

#include "nodelib/array.h"
#include "nodelib/dataset.h"
#include "nodelib/optimize.h"
#include "nodelib/etc/version.h"
#include "nodelib/etc/options.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#define NN_MAJOR_VER 0
#define NN_MINOR_VER 0

#define NN_WUSER     (1 << 6)

#define NN_WMATRIX   (1 << 5)

#define NN_WVECTOR_1 (1 << 4)
#define NN_WVECTOR_2 (1 << 3)
#define NN_WVECTOR   NN_WVECTOR_1

#define NN_WSCALAR_1 (1 << 2)
#define NN_WSCALAR_2 (1 << 1)
#define NN_WSCALAR   NN_WSCALAR_1

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* Stoopid forward declarations.  When will C have a better way of
   doing this? */

struct NN_LINK;
struct NN_LINKLIST;
struct NN_ACTFUNC;
struct NN_NETFUNC;
struct NN_LAYER;
struct NN_LAYERLIST;
struct NN_TRAININFO;
struct NN;

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* The NN_LINK structure defines a functional mapping from the source
   layers to the destination layers.  There will usually be a single
   source and destination layer;  however, the number of legal source
   and destination layers is soley determined by the net function
   for this layer.  For all of the weights the first index always
   specifies the output destination.   The range of the indices for
   the \em{A} matrix is \em{A[numout][numin][numin]}, thus each
   output node (relative to this layer) has a (numin x numin) matrix
   if the \em{A} matrix is used.
   
   Similarly, the range for the \em{B} matrix is
   \em{B[numout][numin][numaux]}, where \em{numaux} is determined
   by the net input function type.

   The two vector weights, \em{u}, and \em{v}, are indexed in the
   range of \em{u[numout][numin]} and  \em{v[numout][numin]}, while the
   scalars are only index by \em{numout}.

   The only field in this structure that you will ever usually modify
   is the \em{lock} field. */

typedef struct NN_LINK {
  /*
   * Sizes of the source, destination,  the number of 
   * auxiliary weights, and the total number of weights
   * used by this link.
   */
  unsigned numin, numout, numaux, numweights;
  /*
   * Weights and the derivatives of the error with respect
   * to the weights.  A is a matrix, u, v and w are
   * vectors, and  a and b are scalars.  All or none
   * of these may be used, depending on the net input
   * function.
   */
  double ***A,  **u,  **v,  **w,  *a,  *b;
  double ***dA, **du, **dv, **dw, *da, *db;
  /*
   * Auxillary variables for computing the Hessian times
   * an arbitrary vector, using Barak Pearlmutter's
   * Rv{.} technique.
   */
  double ***RA,  **Ru,  **Rv,  **Rw,  *Ra,  *Rb;
  double ***RdA, **Rdu, **Rdv, **Rdw, *Rda, *Rdb;
  /*
   * Linked lists for the source and the destination
   * layers.
   */
  struct NN_LAYERLIST *source, *dest;
  /*
   * A pointer to the net input function struture.
   */
  struct NN_NETFUNC *nfunc;
  /*
   * The format string from which this link was created.
   * We save this so that it can be used to write the
   * neural network descripter to a file.
   */
  char *format;
  /*
   * Should these weights be considered fixed?  Note
   * that this is only a suggestion, as a user defined
   * net function can ignore this.  However, the builtin
   * net functions always honor this flag.
   */
  unsigned need_grads : 1;
} NN_LINK;

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* This structure is only used to keep a linked list of NN_LINK
   structures. */

typedef struct NN_LINKLIST {
  NN_LINK *link;
  struct NN_LINKLIST *cdr;
} NN_LINKLIST;

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* The NN_ACTFUNC type defines a possible activation function used in
   a layer or a slab.  The two function pointers in the structure
   should point to the functions that compute the activation function
   and the activation function's derivative.  The activation output
   is supplied to the derivative function so that mathematical shortcuts
   can be exploited in computing the derivatives. */   

typedef struct NN_ACTFUNC {
  char *name;
  double (*func)(double input);
  double (*deriv)(double input, double output);
  double (*second_deriv)(double input, double output,
			 double deriv);
} NN_ACTFUNC;

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* The NN_NETFUNC type (along with a NN_LINK) defines a mapping from a
   vector-space to a scalar, which ultimately forms the net input of
   a node in a NN_LAYER.  The \em{forward} function pointer computes
   the net input for each node in a destination layer.  The \em{backward}
   function pointer is responsible for the computing the partial
   derivatives of the error function with respect to the weights in the
   NN_LINK and the partial derivatives of the error with respect to the
   outputs of the nodes in the source NN_LAYER structures.

   The sanity function is called when a \bf{nn_link()} function call
   is issued to perform simple error checking on the source and
   destination NN_LAYER structures.  Typical constraints could be that
   there is only a single source NN_LAYER, a single destination
   NN_LAYER, or that the source and destination NN_LAYER structures
   are identical in size.  The \em{sanity} function must also assign
   proper values to the \em{numin}, \em{numout}, and \em{numaux}
   pointers. The return result of the \em{sanity} function should be
   a -1 if the \bf{nn_link()} call can not be completed (i.e. there was
   an error in the compatibility of the source and destination NN_LAYERS)
   or a positive integer which indicates the required weight terms to
   be allocated.  See the documentation for \bf{nn_register_netfunc()}
   for more details and an example. */

typedef struct NN_NETFUNC {
  char *name;
  void (*forward)(struct NN *nn, struct NN_LINK *link,
		  struct NN_LAYER *dst);
  void (*backward)(struct NN *nn, struct NN_LINK *link,
		   struct NN_LAYER *src);
  void (*Rforward)(struct NN *nn, struct NN_LINK *link,
		   struct NN_LAYER *dst);
  void (*Rbackward)(struct NN *nn, struct NN_LINK *link,
		    struct NN_LAYER *src);
  int  (*sanity)(struct NN *nn,
		 struct NN_LAYERLIST *source,
                 struct NN_LAYERLIST *destination,
                 unsigned *numin, unsigned *numout,
                 unsigned *numaux);
} NN_NETFUNC;

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* The NN_LAYER structure is used to store the net inputs and activations
   (and their gradients) of the nodes in the layer. */

typedef struct NN_LAYER {
  /*
   * The number of nodes and sublayers in this layer, and 
   * a bit vector to indicate what weights are used.
   */
  unsigned sz, numslabs, idw;
  /*
   * The position of this layer in the NN.  The value
   * of ids is -1 when this is a layer (i.e. not a slab).
   */
  int idl, ids;
  /*
   * The net input, the activation output, and the partial
   * derivatives of the error function with respect to the
   * net input and the activation output.
   */
  double *x, *y, *dx, *dy;
  /*
   * Auxillary variables for computing the Hessian times
   * an arbitrary vector, using Barak Pearlmutter's
   * Rv{.} technique.
   */
  double *Rx, *Ry, *Rdx, *Rdy;
  /*
   * Lists of NN_LINKS.  The out field contains the links
   * leading away from this layer while the in field contains
   * links which come into this layer.
   */
  NN_LINKLIST *out, *in;
  /*
   * The activation function for this layer.
   */
  NN_ACTFUNC *afunc;
  /*
   * An array of the sublayers of this layer, or NULL if it
   * it a slab.
   */
  struct NN_LAYER *slabs;
  /*
   * Does this layer or any of its ancestors need the
   * gradient calculated?
   */
  unsigned need_grads : 1;
} NN_LAYER;

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* This structure is only used to keep a linked list of NN_LAYER
   structures. */

typedef struct NN_LAYERLIST {
  NN_LAYER *layer;
  struct NN_LAYERLIST *cdr;
} NN_LAYERLIST;

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* The NN_TRAININFO structure is the only component of the NN
   structure that typical users will have to modify.  Most of the
   fields should be self explanatory.  The \em{error_function}
   field refers to a function which computes the overall objective
   function that the learning \em{algorithm} will attempt to
   minimize.  See the \bf{errfuncs}(3) and \bf{optimize}(3)
   manual pages for more information on error functions and
   optimization algorithms.

   The \em{online_hook} is called at the end of each step of an
   online optimization procedure, so this is only currently useful in
   the backprop routine.

   The \em{subsample} field determines if all of the training
   data is used in the \bf{nn_offline_test()} and \bf{nn_offline_grad()}
   functions. If its value is equal to zero, then all data is used.
   If its value is between zero and one, then that number is the
   probability that any particular pattern will be used.  If its value
   is greater than one, then that number is the actual number of patterns
   that will be used.  The \em{subsampseed} field is the random
   seed that is used for all calls to the two offline procedures.
   At the end of \bf{nn_offline_grad()} its value is incremented
   by one. */

typedef struct NN_TRAININFO {
  DATASET *train_set, *test_set;
  double (*error_function)(double output, double target,
                           double *derivative,
                           double *second_derivative);
  double subsample;
  double error, rmse, ol_error, ol_mse;
  double stc_eta_0, stc_tau;
  OPTIMIZER opt;
} NN_TRAININFO;

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* The NN structure is the mother of all of the other structures and is
   the pointer type that is returned by \bf{nn_create()} function.
   Moreover, almost all of the function in this module require a NN
   pointer as the first argument.

   Here are some examples on how to access the components of the
   NN structure:
  
   To access the third activation output of the second slab of the
   first layer use:

     \indent \em{nn->layers[0].slabs[1].y[2]}

   To access the second order weight of the first link to the second
   destination node of the third and fourth source nodes use:

     \indent \em{nn->links[0].A[1][2][3]}

   To access the first order weights of the first link to the
   second destination node of the third source node use:

     \indent \em{nn->links[0].u[1][2]}

   To access the bias of the second node of the first link use:

     \indent \em{ nn->links[0].a[1]}

   And so on...
 */

typedef struct NN {
  /*
   * The number of network inputs, network outputs, the total
   * number of weights, the number of layers and the number
   * of links in this NN.
   */
  unsigned numin, numout, numweights, numlayers, numlinks;
  /*
   * The x and y fields point to the network input and output,
   * while dx and dy are the partial derivatives of the error
   * function with respect to the network input and outputs.
   * The t field is the target.
   */
  double *x, *y, *dx, *dy, *t;
  /*
   * Auxillary variables for computing the Hessian times
   * an arbitrary vector, using Barak Pearlmutter's
   * Rv{.} technique.
   */
  double *Rweights, *Rgrads, *Rx, *Ry, *Rdx, *Rdy;
  /*
   * An array of NN_LAYERS.
   */
  NN_LAYER *layers;
  /*
   * An array of NN_LINK pointers.  The ordering used is the
   * same as the order of the nn_link() function calls.
   */
  NN_LINK **links;
  /*
   * How to train this NN.
   */
  NN_TRAININFO info;

  double **grads, **weights;
  unsigned need_all_grads : 1;
} NN;

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* This function creates a NN structure with a fixed number of layers
   and nodes.  The \em{format} string consists of a sequence of layer
   specifications, which in turn can be either an integer or a
   parenthesized list of integers.  If the layer specification is a single
   integer, then the layer will have the number of nodes as specified
   by the integer, and a single sublayer to contain the nodes.  If the
   layer specification is a list such as "(1 2 3)" then the layer will
   have three sublayers with 1, 2, and 3 nodes.  Note that "x" is the same
   "(x)".

   Here is a detailed and annotated examples:
   \indent  \bf{"(1 5) 20 (10 5 10) 1"}
   
   This is a neural network with 4 layers of nodes.  The first node layer
   has two sublayers, with 1 and 5 nodes respectively.  The second node
   layer has twenty nodes.  The third node layer has three sublayers, with
   10, 5, and 10 nodes in each sublayer, respectively.  The last layer
   has a single node.

   The format string may use any special characters that printf() uses with
   the additional arguments corresponding to "%d" or "%s" in the format. */

NN *nn_create(char *format, ...);


/* This function connects layers or sublayers to other layers or sublayers.
   While typical uses of the \em{format} parameter will be simple,
   explaining all legal formats is rather difficult and requires a grammar
   specification:
   \begin{verbatim}
   \bf
          FORMAT := LIST -FLAG-> LIST
          LIST   := ITEM LIST
                 := ITEM
          ITEM   := <Integer>
                 := (<Integer> <Integer>)
          FLAG   := [a-z]
                 := <empty>
   \rm
   \end{verbatim}
   The LIST to the left of the "-FLAG->" denotes a list of source layers
   or sublayers, while the LIST on the right are the sink (or destination)
   layers or sublayers.  Typically, only one ITEM for a source and one
   for a sink is supplied.  However, user defined net input functions can
   have multiple sources and sinks and interpret then in any way they
   see fit.

   If an ITEM consists of a single integer, then the supplied number denotes
   a layer, with the first layer (the input layer) being layer 0.  An
   integer pair, such as "(2 4)" denotes a layer number and sublayer number.
   Again, the first sublayer in a layer is 0.  Thus, networks can be
   selectively wired together.

   The FLAG denotes the net input function type.  Currently supported
   flags include:
   \begin{itemize}

   \item \bf{a} - An alias link that shares weights.  A link of this type
               expects multiple sources and a single destination or
               multiple destinations and a single source.  Whatever
               layers are given in multiples have shared affine linear
               weights.

   \item \bf{c} - A copy connection.  The single source and sink must be the
               same size.  This net function type uses no weights.

   \item \bf{d} - A quadratic net input function with a diagonal matrix.  This
               expects a single source and sink.  The \em{v} terms
               contain the pair wise products, while the remaining terms
               are identical to a linear net input function.

   \item \bf{e} - A Euclidean distance net function.  This expects a single
               source and sink.  The \em{u} terms are used for the
               centers, while the \em{a} terms are used for the variances.

   \item \bf{k} - A copy connection that does not back-propagate gradient
               information.  The single source and sink must be the
               same size.  This net function type uses no weights.

   \item \bf{l} - A linear net input function which expects a single source
               and sink.  If no flag is given, then this is the default
               net function used.  The \em{u} terms are used for the
               linear weights, and the \em{a} terms are the scalar biases.

   \item \bf{n} - A normalizing link which expects a single source and sink
               of the same size.  This net function uses no weights.  It sets
               the sink values equal to the source values normalized by the
               sum of the source values.

   \item \bf{p} - A pair-wise product connection.  Given two sources and a
               single sink, all of which must be the same size, this will
               compute the pair-wise product of the outputs of the sources.
               This link type consumes no real weights.

   \item \bf{q} - A quadratic net input function which expects a single source
               and sink.  The \em{A} terms contain the second order
               weights.  The remaining terms are identical to a linear net
               input function.

   \item \bf{s} - A scalar connection.  The single source and sink must be the
               same size.  There is one scalar multiplicative weight,
               \em{a,} forming pair-wise connections from the source and
               the sink nodes.  When used as a self-connection, this forms
               the basis of simple memory.
   \end{itemize}

   The \bf{nn_link()} function returns a pointer the the newly created
   NN_LINK.  Keeping tract of this value may be useful if you want to
   examine the individual values of weights, or if you want to set the
   lock field of a link. */

NN_LINK *nn_link(NN *nn, char *format, ...);

/* This function is used to change the activation function of a
   sublayer.  The two unsigned arguments denote the layer number and
   sublayer number, where 0 is the first of for each.  The last argument
   is the name of the activation function.  Note that one activation
   function may have more than one name.  This is a feature, not a bug.
   Currently installed activation functions include (identical functions
   that differ only in name are given on the same line):
   \begin{itemize}
     \item \bf{cos}, \bf{cosine}
     \item \bf{exp}
     \item \bf{gauss}, \bf{gaussian}
     \item \bf{lin}, \bf{linear}, \bf{none}
     \item \bf{logistic}, \bf{sigmoid}
     \item \bf{sin}, \bf{sine}
     \item \bf{tanh}
   \end{itemize}

   By default, \bf{tanh} is used for all sublayers.  A pointer to the
   NN_ACTFUNC structure is returned, although why you would need this
   value is not clear.

   The format string may use any special characters that printf() uses with
   the additional arguments corresponding to "%d" or "%s" in the format. */

NN_ACTFUNC *nn_set_actfunc(NN *nn, unsigned layer, /*\*/
                           unsigned slab, char *name);


/* This function will initialize the weights of the supplied NN to
   uniform random value between the -abs(\em{wmax}) and abs(\em{wmax}). */

void nn_init(NN *nn, double wmax);


/* This function will train a NN according to the \em{info} field
   values.  Look at the documentation for the NN_TRAININFO type for
   more details.  The most important user field to set is the
   \em{nn->info.algorithm} field, which should be the function
   pointer to the optimization algorithm.  You will probably also
   want to choose appropriate values for each of the following:

   \begin{itemize}
     \item \it{nn->info.opt.min_epochs}
     \item \it{nn->info.opt.max_epochs}
     \item \it{nn->info.opt.error_tol}
     \item \it{nn->info.opt.delta_error_tol}
   \end{itemize}

   (And for online learning:)

   \begin{itemize}
     \item \it{nn->info.opt.rate }
     \item \it{nn->info.opt.momentum}
   \end{itemize}

   After calling \bf{nn_train()} you can examine the following
   fields for information on how the training went:

   \begin{itemize}
     \item \it{nn->info.opt.fcalls}
     \item \it{nn->info.opt.gcalls}
     \item \it{nn->info.opt.badness}
     \item \it{nn->info.opt.epoch}
     \item \it{nn->info.opt.error}
   \end{itemize}

   Advanced users can also set any of the following fields for
   greater control. See the \bf{optimize}(3) manual pages
   for more information on setting these fields:

   \begin{itemize}
     \item \it{nn->info.opt.hook}
     \item \it{nn->info.opt.hook_freq}
     \item \it{nn->info.opt.stepf}
   \end{itemize}   

   Keep in mind that the hook function is passed a (void *)
   data type, which means that you must include the appropriate
   casts in your hook functions.  Also note that it is useless
   to set any of the following fields, because \bf{nn_train()}
   will override your choices:

   \begin{itemize}
     \item \it{nn->info.opt.size}
     \item \it{nn->info.opt.weights}
     \item \it{nn->info.opt.grads}
     \item \it{nn->info.opt.funcf}
     \item \it{nn->info.opt.gradf}
     \item \it{nn->info.opt.haltf}
     \item \it{nn->info.opt.obj}
   \end{itemize}        

   with \em{obj} being set to the value of \em{nn}, and
   \em{haltf} being set to a function that does cross
   validation with \em{nn->info.test_pats}.

   */

int nn_train(NN *nn);


/* If the supplied NN has a linear output layer that also has a linear
   link (numbered \em{linknum}) coming into it from somewhere and if
   the supplied DATASET and NN have compatible I/O dimensions, then
   this function will solve the linear weights exactly with respect to
   the data in \em{set.}  The routine uses a singular value
   decomposition to compute the pseudo-inverse for the least squares
   solution.  Note that if you are using any error function other than
   the quadratic, then the LMS solution will only be an approximation
   to what you really want.  Zero is returned on success, non-zero
   otherwise. */

int nn_solve(NN *nn, DATASET *set, unsigned linknum);


/* If the supplied NN has a linear output layer that also has any
   linear, quadratic, or quadratic diagonal links coming into it from
   somewhere and if the supplied DATASET and NN have compatible I/O
   dimensions, then this function will solve for all linear weights
   exactly with respect to the data in \em{set.}  The routine uses a
   singular value decomposition to compute the pseudo-inverse for the
   least squares solution.  Note that if you are using any error
   function other than the quadratic, then the LMS solution will only
   be an approximation to what you really want.  Zero is returned on
   success, non-zero otherwise. */

int nn_solve_all(NN *nn, DATASET *set);


/* This function will contruct an NN that has all of the properties of
   a radial basis function network with \em{nbasis} Gaussian basis
   functions.  The centers of the RBFN will either be clustered from
   the supplied DATASET with the k-means clustering algorithm or they
   will come from a random subset of the data.  The behavior for this
   is determined by \em{nn_rbf_centers_random.}  The variances of all
   of the basis functions will be set to the constant \em{var,} if
   \em{var} is greater than zero, or according to the nearest neighbor
   heuristic, if \em{var} is less than or equal to zero (times
   |\em{var}| if(\em{var} != 0)).  The linear weights will be solved
   for exactly by calling the \bf{nn_solve()} routine.  Other options
   are controled by the globals \em{nn_rbf_basis_normalized,}
   \em{nn_kmeans_online,} \em{nn_kmeans_maxiters,}
   \em{nn_kmeans_clusinit,} and \em{nn_kmeans_minfrac.}  See the
   global docs for more details. */

NN *nn_create_rbf(unsigned nbasis, double var, DATASET *set);


/* Similar to \bf{nn_create_rbf()} but will build an SMLP. */

NN *nn_create_smlp(unsigned nbasis, double var, DATASET *set);


/* This function will free up all memory associate with a NN. */

void nn_destroy(NN *nn);


/* Ths function will write a ASCII readable descriptor file of the
   supplied NN that can be read back in at a later time.  The
   file format is verbose with comments, but you probably don't
   want to edit weight values, as they actual format depends
   on the architecture of the network. A nonzero value is returned
   if an error occured, while zero is returned if all went well.*/

int nn_write(NN *nn, const char *fname);


/* This function is similar to \bf{nn_write()} but is so verbose
   that the user should be able to completely reconstruct the
   network by only consuilting the output file. */

int nn_write_verbose(NN *nn, const char *fname);


/* This function is similar to \bf{nn_write()} with the difference
   that no comments are written, anf that weight values will be
   written in binary.  A nonzero value is returned if an error
   occured, while zero is returned if there were no errrors. */

int nn_write_binary(NN *nn, char *fname);


/* This function will read the contents of any file that was create
   with either \bf{nn_write()} or \bf{nn_write_binary()}, and return
   the NN described by the file.  NULL is returned on error. */

NN *nn_read(const char *fname);


/* This function will eliminate internal hash tables used by the
   package, and free up other miscellaneous items as well.  It is
   not strictly necessary to call this function perform terminating
   a program, but if you need the library to clean up after itself,
   then this is the way to do it. */

void nn_shutdown(void);

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* Compute a forward pass through the neural network with the
   specified inputs. */

void nn_forward(NN *nn, double *input);


/* Compute a backward pass through a neural network.  The
   \em{error_gradient} argument needs to have the gradients of the
   error function with repect to the network's outputs.  Doing the
   backwards pass in this manner allows two neural networks to be
   joined together so that one can be trained without target values,
   as in model based control. */

void nn_backward(NN *nn, double *error_gradient);


/* Performs a feedforward pass on every pattern in \em{set}.  The
   \em{hook} function is called for every individual feedforward pass,
   which allows you to perform a function on every single pattern
   in \em{set}. */

double nn_offline_test(NN *nn, DATASET *set, int (*hook)(NN *nn));


/* Like \bf{nn_offline_test()}, but also does the backward passes
   as well.  The accumulated gradient is saved. */

double nn_offline_grad(NN *nn, DATASET *set, int (*hook)(NN *nn));


/* This function allows you to add a new activation function to the
   library without recompiling everything.  See the source code
   in the file \bf{nnafunc.c} to see how the functions should be
   written. */

void nn_register_actfunc(char *name, /*\*/
			 double (*func)(double input), /*\*/
			 double (*deriv)(double input, /*\*/
					 double output), /*\*/
                         double (*second_deriv)(double input, /*\*/
						double output, /*\*/
						double deriv));


/* This function allows you to add a new net-input function to the
   library without recompiling everything.  See the source code
   in the file \bf{nnnfunc.c} to see how the functions should be
   written. */

void nn_register_netfunc(char *name, /*\*/
			 void (*forward)(struct NN *nn, /*\*/
					 struct NN_LINK *link, /*\*/
					 struct NN_LAYER *dst), /*\*/
                         void (*backward)(struct NN *nn, /*\*/
					  struct NN_LINK *link, /*\*/
					  struct NN_LAYER *src), /*\*/
			 void (*Rforward)(struct NN *nn, /*\*/
					  struct NN_LINK *link, /*\*/
					  struct NN_LAYER *dst), /*\*/
                         void (*Rbackward)(struct NN *nn, /*\*/
					   struct NN_LINK *link, /*\*/
					   struct NN_LAYER *src), /*\*/
			 int (*sanity)(struct NN *nn, /*\*/
				       struct NN_LAYERLIST *source, /*\*/
				       struct NN_LAYERLIST *destination, /*\*/
				       unsigned int *numin, /*\*/
				       unsigned int *numout, /*\*/
				       unsigned int *numaux));


/* Computes the product of the Hessian matrix of second derivatives and
   an arbitrary vector.  The vector that is multiplied to the Hessian
   is located in \em{nn->Rweights} and the result of the product
   can be found in \em{nn->Rgrads}.  This routine uses Barak
   Pearlmutter's Rv{.} technique to perform the computation. */   

void nn_Hv(NN *nn, double *input, double *target, double *v);


/* This routines uses \bf{nn_Hv()} to extract the columns of the
   Hessian matrix of second derivates of the neural network's error
   function with respect to the weights. */

int nn_hessian(NN *nn, double *input, double *target, double **H);


/* This routines uses \bf{nn_hessian()} to compute the
   Hessian matrix of second derivates of the neural network's error
   function with respect to the weights over an entire DATASET. */

int nn_offline_hessian(NN *nn, DATASET *set, double **H);


/* Evaluates the Jacobian matrix of \em{nn} at \em{input} and places
   the result in \em{J} which is assumed to have as many elements as
   the product of the number of input and outputs of \em{nn}.  The
   value of the entry \em{J[i * NCOL + j]} is set equal to the partial
   derivative of the \em{i}th output with respect to the \em{j}th
   input. */

void nn_jacobian(NN *nn, double *input, double *J);


void nn_Rforward(NN *nn, double *Rinput, double *Rweights);

void nn_Rbackward(NN *nn, double *Rdoutput);



void nn_get_weights(NN *nn, double *w);

void nn_set_weights(NN *nn, double *w);

void nn_get_grads(NN *nn, double *g);

void nn_set_grads(NN *nn, double *g);



void nn_get_Rweights(NN *nn, double *Rw);

void nn_set_Rweights(NN *nn, double *Rw);

void nn_get_Rgrads(NN *nn, double *Rg);

void nn_set_Rgrads(NN *nn, double *Rg);



void nn_lock_link(NN *nn, unsigned linknum);

void nn_unlock_link(NN *nn, unsigned linknum);


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* A routine to do search-then-converge step size for online learning.
   This is used by setting \em{nn->info.opt.stepf} to the pointer
   of this function. */

double nn_lnsrch_search_then_converge(OPTIMIZER *opt, /*\*/
				      double *dir, double stepsz);

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* h2man:include  There are several global variables that can be used
   to change the default behavior of this package:

   \begin{itemize}
   \item \bf{double} \em{nn_offline_bignum_skip} ;
   This can be used to tune the behavior of the offline routines,
   nn_offline_test() and nn_offline_grad(). By default, this is
   initialized to 0.0.  However, if it is non-zero, then any pattern
   with an input greater in magnitude than this value will be skipped.
   Also, if a target value exceeds this threshold, then the error
   gradiant for that output is set to zero in the backward pass in
   nn_offline_grad().  Because this effects the sum of errors, the
   error value returned is normalized by the number of valid outputs
   for all patterns.

   \item \bf{int}  \em{nn_rbf_centers_random} ;
   If nonzero, then \bf{nn_create_rbf()} will set the centers to a random
   subset of the passed DATASET.  Default is zero.

   \item \bf{int}  \em{nn_rbf_smlp_random} ;
   If nonzero, then \bf{nn_create_smlp()} will set the centers to a random
   subset of the passed DATASET.  Default is zero.
   
   \item \bf{int}  \em{nn_rbf_basis_normalized} ;
   If nonzero, then \bf{nn_create_rbf()} will build an RBF that has normalized
   basis functions. Default is zero.
   
   \item \bf{int}  \em{nn_kmeans_online} ;
   If kmeans is called for \bf{nn_create_rbf()} or
   \bf{nn_create_smlp(),} then these routines will call the online
   version if this is nonzero.  Default is 0.

   \item \bf{int}  \em{nn_kmeans_maxiters} ;
   Controls the number of kmeans iterations used for both of
   \bf{nn_create_rbf()} and \bf{nn_create_smlp().}  Default is 100.

   \item \bf{int}  \em{nn_kmeans_clusinit} ;
   Controls the type of kmeans initialization used for both of
   \bf{nn_create_rbf()} and \bf{nn_create_smlp().}  See kmeans() for
   valid values.  Default is 0;

   \item \bf{double}  \em{nn_kmeans_minfrac} ;
   Controls the minfrac used for both of \bf{nn_create_rbf()} and
   \bf{nn_create_smlp()}.  See kmeans() for more details.  Default is
   0.0.
   \end{itemize}
 
*/

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* h2man:skipbeg */

/* Mostly private functions. */

NN_ACTFUNC *nn_find_actfunc(char *name);
NN_NETFUNC *nn_find_netfunc(char *name);

unsigned nn_layerlist_len(NN_LAYERLIST *list);
NN_LAYERLIST *nn_layerlist_cons(NN_LAYER *layer, NN_LAYERLIST *cdr);
void nn_layerlist_free(NN_LAYERLIST *list);

unsigned nn_linklist_len(NN_LINKLIST *list);
NN_LINKLIST *nn_linklist_cons(NN_LINK *link, NN_LINKLIST *cdr);
void nn_linklist_free(NN_LINKLIST *list);

int nn_check_valid_layer(NN *nn, unsigned l);
int nn_check_valid_slab(NN *nn, unsigned l, unsigned s);

#ifndef NN_SOLVE_OWNER
extern int nn_kmeans_online;
extern int nn_kmeans_maxiters;
extern int nn_kmeans_clusinit;
extern double nn_kmeans_minfrac;
extern int nn_rbf_centers_random;
extern int nn_smlp_centers_random;
extern int nn_rbf_basis_normalized;
#endif

/* h2man:skipend */

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __NN_H__ */

