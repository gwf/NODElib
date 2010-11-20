
/* Copyright (c) 1995 by G. W. Flake. */

#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <string.h>

#ifdef SUNOS
#include <varargs.h>
char *vsprintf(char *, char *, va_list);
#endif

#include "nodelib/nn.h"
#include "nodelib/misc.h"
#include "nodelib/array.h"
#include "nodelib/scan.h"
#include "nodelib/optimize.h"
#include "nodelib/errfunc.h"

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* This function will take a format string that look like:
                  "(1 2 3) 4 (5 6) (7)"
   and convert it to an array of unsigned ints that looks like:
                   4 3 1 2 1 1 2 3 4 5 6 7 
   Which is actually the number of layers, followed by the number
   of slabs in each layer, followed by the number of nodes in each slab. */

static unsigned *scan_net_format(char *f)
{
  unsigned i, tot, sz, *u;
  ARRAY *a;
  SCAN *s;
  char *t;

  a = array_create(512, unsigned);
  s = scan_create(0, f);
  s->whites = " \t";
  s->delims = "()";
  s->comments = "";
  while((t = scan_get(s)) != NULL) {
    if(isdigit(*t)) {
      array_append(a, atoi(t), unsigned);
      array_append(a, 0, unsigned);
    }
    else if(*t == '(') {
      while(1) {
	if((t = scan_get(s)) != NULL) {
	  if(isdigit(*t)) {
	    array_append(a, atoi(t), unsigned);
	  }
	  else if(*t == ')') {
	    array_append(a, 0, unsigned);
	    break;
	  }
	  else {
	    ulog(ULOG_ERROR, "nn_create: unexpected token"
		 " in format: '%s'.", f);
	    array_destroy(a);
	    scan_destroy(s);
	    return(NULL);
	  }
	}
	else {
	  ulog(ULOG_ERROR, "nn_create: unexpected EOL in format: '%s'.", f);
	  array_destroy(a);
	  scan_destroy(s);
	  return(NULL);
    } } }
    else {
      ulog(ULOG_ERROR, "nn_create: unexpected token in format: '%s'.", f);
      array_destroy(a);
      scan_destroy(s);
      return(NULL);
    }
  }

  if((sz = array_size(a)) == 0) {
    ulog(ULOG_ERROR, "nn_create: (nearly) empty format string: '%s'.", f);
    array_destroy(a);
    scan_destroy(s);
    return(NULL);
  }
  
  scan_destroy(s);

  /* Count the number of layers and append it to the array. */
  tot = 0;
  for(i = 0; i < sz; i++)
    if(array_fast_access(a, i, unsigned) == 0)
      tot++;
  array_append(a, tot, unsigned);

  /* Count the number of slabs in each layer and append each to the array. */
  tot = 0;
  for(i = 0; i < sz; i++) {
    if(array_fast_access(a, i, unsigned) == 0) {
      array_append(a, tot, unsigned);
      tot = 0;
    }
    else {
      tot++;
    }
  }
  
  /* Append to the array the number of nodes for each slab. */
  for(i = 0; i < sz; i++)
    if((tot = array_fast_access(a, i, unsigned)) != 0)
      array_append(a, tot, unsigned);

  /* Remove the sz bogus entries in the front of the array,
   * convert the array to a ptr, free up everything and return
   * the pointer .
   */
  for(i = 0; i < sz; i++)
    array_destroy_front(a);

  u = array_to_pointer(a);
  array_destroy(a);
  return(u);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

NN *nn_create(char *format, ...)
{
  NN *nn;
  va_list args;
  char *buffer;
  unsigned *nninfo, *ptr, i, j, numlayers, numslabs, numnodes;

  va_start(args, format);
  buffer = xmalloc(strlen(format) * 3 * sizeof(char));
  vsprintf(buffer, format, args);
  format = buffer;

  nninfo = scan_net_format(format);
  ptr = nninfo + nninfo[0] + 1;

  /* Allocate all of the layers. */

  nn = xcalloc(1, sizeof(NN));
  nn->numlayers = numlayers = nninfo[0];
  nn->layers = xcalloc(numlayers, sizeof(NN_LAYER));
  nn->links = NULL;
  nn->numlinks = 0;

  nn->info.train_set = nn->info.test_set = NULL;
  nn->info.error_function = opt_err_quadratic;
  nn->info.opt = OPTIMIZER_DEFAULT;
  nn->info.opt.owner = nn;
  nn->info.subsample = 0;
  nn->need_all_grads = 0;

  for(i = 0; i < numlayers; i++) {

    /* Allocate the slab for layer i. */

    nn->layers[i].numslabs = numslabs = nninfo[1 + i];
    nn->layers[i].slabs = xcalloc(numslabs, sizeof(NN_LAYER));
    nn->layers[i].need_grads = 1;

    /* Set the sz for each slab, and keep a total count. */
    numnodes = 0;
    for(j = 0; j < numslabs; j++) {
      nn->layers[i].slabs[j].idl = i;
      nn->layers[i].slabs[j].ids = j;
      nn->layers[i].slabs[j].numslabs = 0;
      nn->layers[i].slabs[j].sz = *ptr;
      numnodes += *ptr++;
      nn->layers[i].slabs[j].in = NULL;
      nn->layers[i].slabs[j].out = NULL;
      nn->layers[i].slabs[j].need_grads = 1;

      if(i == 0)
	nn->layers[i].slabs[j].afunc = nn_find_actfunc("none");
      else
	nn->layers[i].slabs[j].afunc = nn_find_actfunc("tanh");
    }
    
    nn->layers[i].idl = i;
    nn->layers[i].ids = -1;
    nn->layers[i].in = NULL;
    nn->layers[i].out = NULL;
    nn->layers[i].sz = numnodes;
    nn->layers[i].afunc = NULL;

    /* Now that we have a total, allocate space for the whole
     * layer, and set the slab space pointing within the layer space.
     * Cool, huh?
     */
    nn->layers[i].x   = xcalloc(numnodes, sizeof(double));
    nn->layers[i].y   = xcalloc(numnodes, sizeof(double));
    nn->layers[i].dx  = xcalloc(numnodes, sizeof(double));
    nn->layers[i].dy  = xcalloc(numnodes, sizeof(double));
    nn->layers[i].Rx  = xcalloc(numnodes, sizeof(double));
    nn->layers[i].Ry  = xcalloc(numnodes, sizeof(double));
    nn->layers[i].Rdx = xcalloc(numnodes, sizeof(double));
    nn->layers[i].Rdy = xcalloc(numnodes, sizeof(double));
    numnodes = 0;
    for(j = 0; j < numslabs; j++) {
      nn->layers[i].slabs[j].x   = nn->layers[i].x  + numnodes;
      nn->layers[i].slabs[j].y   = nn->layers[i].y  + numnodes;
      nn->layers[i].slabs[j].dx  = nn->layers[i].dx + numnodes;
      nn->layers[i].slabs[j].dy  = nn->layers[i].dy + numnodes;
      nn->layers[i].slabs[j].Rx  = nn->layers[i].Rx  + numnodes;
      nn->layers[i].slabs[j].Ry  = nn->layers[i].Ry  + numnodes;
      nn->layers[i].slabs[j].Rdx = nn->layers[i].Rdx + numnodes;
      nn->layers[i].slabs[j].Rdy = nn->layers[i].Rdy + numnodes;
      numnodes += nn->layers[i].slabs[j].sz;
    }
  }

  nn->numin = nn->layers[0].sz;
  nn->numout = nn->layers[numlayers - 1].sz;
  nn->numweights = 0;
  nn->x = nn->layers[0].x;
  nn->y = nn->layers[numlayers - 1].y;
  nn->t = xcalloc(nn->numout, sizeof(double));
  nn->dx = nn->layers[0].dx;
  nn->dy = nn->layers[numlayers - 1].dy;

  nn->Rx = nn->layers[0].Rx;
  nn->Ry = nn->layers[numlayers - 1].Ry;
  nn->Rdx = nn->layers[0].Rdx;
  nn->Rdy = nn->layers[numlayers - 1].Rdy;
  
  nn->weights = nn->grads = NULL;

  xfree(nninfo);
  xfree(buffer);
  return(nn);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void nn_destroy(NN *nn)
{
  unsigned i, j;

  /* Free up the memory from the links.  We aren't freeing the
   * weight-space at this point yet, nor the linked lists.
   */
  for(i = 0; i < nn->numlinks; i++) {
    if(nn->links[i]->A) {
      deallocate_array(nn->links[i]->A);
      deallocate_array(nn->links[i]->dA);
      deallocate_array(nn->links[i]->RA);
      deallocate_array(nn->links[i]->RdA);
    }
    if(nn->links[i]->u) {
      deallocate_array(nn->links[i]->u);
      deallocate_array(nn->links[i]->du);
      deallocate_array(nn->links[i]->Ru);
      deallocate_array(nn->links[i]->Rdu);
    }
    if(nn->links[i]->v) {
      deallocate_array(nn->links[i]->v);
      deallocate_array(nn->links[i]->dv);
      deallocate_array(nn->links[i]->Rv);
      deallocate_array(nn->links[i]->Rdv);
    }
    if(nn->links[i]->w) {
      deallocate_array(nn->links[i]->w);
      deallocate_array(nn->links[i]->dw);
      deallocate_array(nn->links[i]->Rw);
      deallocate_array(nn->links[i]->Rdw);
    }
    if(nn->links[i]->a) {
      deallocate_array(nn->links[i]->a);
      deallocate_array(nn->links[i]->da);
      deallocate_array(nn->links[i]->Ra);
      deallocate_array(nn->links[i]->Rda);
    }
    if(nn->links[i]->b) {
      deallocate_array(nn->links[i]->b);
      deallocate_array(nn->links[i]->db);
      deallocate_array(nn->links[i]->Rb);
      deallocate_array(nn->links[i]->Rdb);
    }
    /* Now do the linked lists for the sources and dests. */
    nn_layerlist_free(nn->links[i]->source);
    nn_layerlist_free(nn->links[i]->dest);
    
    xfree(nn->links[i]->format);
    xfree(nn->links[i]);
  }

  /* Now do the layers and sublayers. */
  for(i = 0; i < nn->numlayers; i++) {
    for(j = 0; j < nn->layers[i].numslabs; j++) {
      nn_linklist_free(nn->layers[i].slabs[j].out);
      nn_linklist_free(nn->layers[i].slabs[j].in);
    }
    nn_linklist_free(nn->layers[i].out);
    nn_linklist_free(nn->layers[i].in);
    if(nn->layers[i].slabs)
      xfree(nn->layers[i].slabs);
    xfree(nn->layers[i].x);
    xfree(nn->layers[i].y);
    xfree(nn->layers[i].dx);
    xfree(nn->layers[i].dy);
    xfree(nn->layers[i].Rx);
    xfree(nn->layers[i].Ry);
    xfree(nn->layers[i].Rdx);
    xfree(nn->layers[i].Rdy);
  }

  /* Do the weights and all of the rest. */
  if(nn->weights) xfree(nn->weights);
  if(nn->grads) xfree(nn->grads);
  if(nn->layers) xfree(nn->layers);
  if(nn->links) xfree(nn->links);
  xfree(nn->t);
  xfree(nn);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
