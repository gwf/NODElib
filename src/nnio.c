
/* Copyright (c) 1995 by G. W. Flake. */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "nodelib/misc.h"
#include "nodelib/nn.h"
#include "nodelib/scan.h"

char *nn_weight_fmt = "% .12e";

#ifndef SEEK_END
#define SEEK_END 2
#endif

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

NN *nn_read(const char *fname)
{
  NN *nn = NULL;
  SCAN *s = NULL;
  double *weights = NULL;
  FILE *fp;
  char magic[80], type, *line;
  unsigned major, minor, i, j, k, *need_grads = NULL;

  if((fp = fopen(fname, "r")) == NULL) {
    ulog(ULOG_WARN, "nn_read: unable to open '%s': %m.", fname);
    return(NULL);
  }
  if(fgets(magic, 80, fp) == NULL) {
    ulog(ULOG_WARN, "nn_read: could not read magic number of '%s': %m.", fname);
    return(NULL);
  }
  if(sscanf(magic, "#sn%d.%d:%c\n", &major, &minor, &type) != 3) {
    ulog(ULOG_WARN, "nn_read: bad magic number in '%s': %m.", fname);
    return(NULL);
  }
  if(type != 'a' && type != 'b') {
    ulog(ULOG_WARN, "nn_read: '%s' has a bad header: '%s'.", fname, magic);
    return(NULL);
  }
  if(major != NN_MAJOR_VER || minor != NN_MINOR_VER)
    ulog(ULOG_WARN, "nn_read: version numbers of '%s' and library differ.%t"
	 "library version = %d.%d, data file version = %d.%d.%t"
	 "cross your fingers.", fname, NN_MAJOR_VER, NN_MINOR_VER, major, minor);

  s = scan_create(1, fp);
  s->delims = "";
  s->whites = "\n";
  s->comments = "#";

  if((line = scan_get(s)) == NULL) goto bad_file;
  if((nn = nn_create(line)) == NULL)
    goto bad_file;
  for(i = 0; i < nn->numlayers; i++)
    for(j = 0; j < nn->layers[i].numslabs; j++) {
      if((line = scan_get(s)) == NULL) goto bad_file;
      if(nn_set_actfunc(nn, i, j, line) == NULL) goto bad_file;
      s->whites = "\n";
    }
  if((line = scan_get(s)) == NULL) goto bad_file;
  k = atoi(line);
  for(i = 0; i < k; i++) {
    if((line = scan_get(s)) == NULL) goto bad_file;
    if(nn_link(nn, line) == NULL) goto bad_file;
  }

  need_grads = allocate_array(1, sizeof(unsigned), nn->numlinks);
  s->whites = " \n";
  for(i = 0; i < k; i++) {
    if((line = scan_get(s)) == NULL) goto bad_file;
    need_grads[i] = atoi(line);
  }

  weights = allocate_array(1, sizeof(double), nn->numweights);
  if(type == 'a')
    for(i = 0; i < nn->numweights; i++) {
      if((line = scan_get(s)) == NULL) goto bad_file;
      weights[i] = atof(line);
    }
  else {
    rewind(fp);
    if(fseek(fp, -(long)sizeof(double) * nn->numweights, SEEK_END) == -1)
      goto bad_file;
    if(fread(weights, sizeof(double), nn->numweights, fp) != nn->numweights)
      goto bad_file;
  }
  nn_set_weights(nn, weights);
  deallocate_array(weights);

  for(i = 0; i < nn->numlinks; i++) {
    if(need_grads[i]) nn_unlock_link(nn, i);
    else nn_unlock_link(nn, i);
  }
  deallocate_array(need_grads);
  
  fclose(fp);
  scan_destroy(s);
  return(nn);


 bad_file:
  if(nn) nn_destroy(nn);
  if(s) scan_destroy(s);
  if(weights) deallocate_array(weights);
  if(need_grads) deallocate_array(need_grads);
  fclose(fp);
  ulog(ULOG_WARN, "nn_read: error on input file '%s'.", fname);
  return(NULL);  

}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static char verbose_message[] = "\
# \n\
# The next section contains all of the weights in verbose format.  The\n\
# format for the comment column is:\n\
# \n\
#   (SOURCE) (DESTINATION) VAR[index, index, index]\n\
# \n\
# where\n\
# \n\
#   SOURCE       two integers for layer no. and sublayer no.\n\
#   DESTINATION  two integers for layer no. and sublayer no.\n\
#   VAR          one of A, u, v, w, a, or b\n\
#   INDICES      three for A, two for u, v, and w, and one for a and b\n\
# \n\
# For the SOURCE or DESTINATION, if the sublayer number is -1 then the\n\
# link connects to (or from) an entire layer (i.e., not a sublayer).\n\
# \n\
# As for the indices, the first always represents an output in the\n\
# destination layer.  For 'A' the second two indices refer to a pair of\n\
# nodes in the source layer.  For 'u' and 'v', the second index refers\n\
# to a single node in the source layer.  And for 'w', the second index\n\
# always refers to one of the NUMAUX variables which are uniquely\n\
# determined by the net function type.\n\
# \n\
";

static void write_internal(NN *nn, FILE *fp, int bin, int verbose)
{
  double *weights = NULL;
  unsigned *need_grads = NULL;
  unsigned l, i, j, k, sz;

  /* Write out the magic numbers. */
  fprintf(fp, "#sn%d.%d:%c\n", NN_MAJOR_VER, NN_MINOR_VER, bin ? 'b' : 'a');
  
  if(!bin) {
    fprintf(fp, "#\n# This is a neural network with %u inputs, %u outputs,\n",
	    nn->numin, nn->numout);
    for(i = 0, sz = 0; i < nn->numlayers; i++)
      sz += nn->layers[i].numslabs;
    fprintf(fp, "# %u layers, %u internal slabs, %u links, and %u trainable "
	    "weights.\n#\n", nn->numlayers, sz, nn->numlinks, nn->numweights);

    fputs("# The next string is an architecture descriptor:\n", fp);
  }
  for(i = 0; i < nn->numlayers; i++) {
    fputc('(', fp);
    for(j = 0; j < nn->layers[i].numslabs; j++) {
      fprintf(fp, "%d", nn->layers[i].slabs[j].sz);
      if(j != nn->layers[i].numslabs - 1)
        fputc(' ', fp);
    }
    fputc(')', fp);
  }
  fputc('\n', fp);

  if(!bin)
    fputs("#\n# The next section describes for each sublayer the "
          "activation function\n# and the auxiliary variables:\n", fp);

  for(i = 0; i < nn->numlayers; i++)
    for(j = 0; j < nn->layers[i].numslabs; j++)
      fprintf(fp, "%s\n", nn->layers[i].slabs[j].afunc->name);
  
  if(!bin) {
    fputs("#\n# The next section describes for each link the "
          "net input function:\n", fp);
  }
  
  if(bin)
    fprintf(fp, "%u\n", nn->numlinks);
  else
    fprintf(fp, "%u # The number of links\n", nn->numlinks);

  for(i = 0; i < nn->numlinks; i++)
    fprintf(fp, "%s\n", nn->links[i]->format);

  /* Need to temporarily set need_grads field so that all weights
   * are retreived.
   */
  need_grads = allocate_array(1, sizeof(unsigned), nn->numlinks);
  for(i = 0; i < nn->numlinks; i++) {
    need_grads[i] = nn->links[i]->need_grads;
    if(!need_grads[i]) nn_unlock_link(nn, i);
  }

  if(!bin)
    fputs("#\n# The next section states if the gradient for "
          "each link should be calculated:\n", fp);
  for(i = 0; i < nn->numlinks; i++)
    fprintf(fp, "%u%c", need_grads[i],
	    (i < (nn->numlinks - 1)) ? ' ' : '\n');
 
  if(!bin && verbose) {
    fputs(verbose_message, fp);
    for(l = 0; l < nn->numlinks; l++) {
      if(nn->links[l]->A)
	for(i = 0; i < nn->links[l]->numout; i++)
	  for(j = 0; j < nn->links[l]->numin; j++)
	    for(k = 0; k < nn->links[l]->numin; k++) {
	      fprintf(fp, nn_weight_fmt, nn->links[l]->A[i][j][k]);
	      fprintf(fp, "\t# ");
	      fprintf(fp, "( % d, % d ) ", nn->links[l]->source->layer->idl,
		      nn->links[l]->source->layer->ids);
	      fprintf(fp, "( % d, % d ) ", nn->links[l]->dest->layer->idl,
		      nn->links[l]->dest->layer->ids);
	      fprintf(fp, "A[%d][%d][%d]\n", i, j, k);
	    }
      
      if(nn->links[l]->u)
	for(i = 0; i < nn->links[l]->numout; i++)
	  for(j = 0; j < nn->links[l]->numin; j++) {
	    fprintf(fp, nn_weight_fmt, nn->links[l]->u[i][j]);
	    fprintf(fp, "\t# ");
	    fprintf(fp, "( % d, % d ) ", nn->links[l]->source->layer->idl,
		    nn->links[l]->source->layer->ids);
	    fprintf(fp, "( % d, % d ) ", nn->links[l]->dest->layer->idl,
		    nn->links[l]->dest->layer->ids);
	    fprintf(fp, "u[%d][%d]\n", i, j);
	  }

      if(nn->links[l]->v)
	for(i = 0; i < nn->links[l]->numout; i++)
	  for(j = 0; j < nn->links[l]->numin; j++) {
	    fprintf(fp, nn_weight_fmt, nn->links[l]->v[i][j]);
	    fprintf(fp, "\t# ");
	    fprintf(fp, "( % d, % d ) ", nn->links[l]->source->layer->idl,
		    nn->links[l]->source->layer->ids);
	    fprintf(fp, "( % d, % d ) ", nn->links[l]->dest->layer->idl,
		    nn->links[l]->dest->layer->ids);
	    fprintf(fp, "v[%d][%d]\n", i, j);
	  }
      
      if(nn->links[l]->v)
	for(i = 0; i < nn->links[l]->numout; i++)
	  for(j = 0; j < nn->links[l]->numaux; j++) {
	    fprintf(fp, nn_weight_fmt, nn->links[l]->w[i][j]);
	    fprintf(fp, "\t# ");
	    fprintf(fp, "( % d, % d ) ", nn->links[l]->source->layer->idl,
		    nn->links[l]->source->layer->ids);
	    fprintf(fp, "( % d, % d ) ", nn->links[l]->dest->layer->idl,
		    nn->links[l]->dest->layer->ids);
	    fprintf(fp, "w[%d][%d]\n", i, j);
	  }
      
      if(nn->links[l]->a)
	for(i = 0; i < nn->links[l]->numout; i++) {
	  fprintf(fp, nn_weight_fmt, nn->links[l]->a[i]);
	  fprintf(fp, "\t# ");
	  fprintf(fp, "( % d, % d ) ", nn->links[l]->source->layer->idl,
		  nn->links[l]->source->layer->ids);
	  fprintf(fp, "( % d, % d ) ", nn->links[l]->dest->layer->idl,
		  nn->links[l]->dest->layer->ids);
	  fprintf(fp, "a[%d]\n", i);
	}
      
      if(nn->links[l]->b)
	for(i = 0; i < nn->links[l]->numout; i++) {
	  fprintf(fp, nn_weight_fmt, nn->links[l]->b[i]);
	  fprintf(fp, "\t# ");
	  fprintf(fp, "( % d, % d ) ", nn->links[l]->source->layer->idl,
		  nn->links[l]->source->layer->ids);
	  fprintf(fp, "( % d, % d ) ", nn->links[l]->dest->layer->idl,
		  nn->links[l]->dest->layer->ids);
	  fprintf(fp, "b[%d]\n", i);
	}
    }
  }
  else {
    if(!bin)
      fputs("#\n# The next section contains all of the weights:\n", fp);
    
    weights = allocate_array(1, sizeof(double), nn->numweights);
    nn_get_weights(nn, weights);
    for(i = 0; i < nn->numlinks; i++)
      if(need_grads[i]) nn_unlock_link(nn, i);

    if(!bin) {
      char tmp[80];
      sprintf(tmp, nn_weight_fmt, 1.0);
      j = 80 / (strlen(tmp) + 1);
      k = 0;
      for(i = 0; i < nn->numweights; i++) {
	fprintf(fp, nn_weight_fmt, weights[i]);
	k++; k %= j;
	if(k)
	  fputc(' ', fp);
	else
	  fputc('\n', fp);
      }
      if(k)
	fputc('\n', fp);
    }
    else
      fwrite(weights, sizeof(double), nn->numweights, fp);
    deallocate_array(weights);
  }

  deallocate_array(need_grads);

  fclose(fp);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

int nn_write(NN *nn, const char *fname)
{
  FILE *fp;
  
  if((fp = fopen(fname, "w")) == NULL) {
    ulog(ULOG_WARN, "nn_write: unable to open '%s': %m.", fname);
    return(1);
  }
  write_internal(nn, fp, 0, 0);
  return(0);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

int nn_write_verbose(NN *nn, const char *fname)
{
  FILE *fp;
  
  if((fp = fopen(fname, "w")) == NULL) {
    ulog(ULOG_WARN, "nn_write: unable to open '%s': %m.", fname);
    return(1);
  }
  write_internal(nn, fp, 0, 1);
  return(0);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

int nn_write_binary(NN *nn, char *fname)
{
  FILE *fp;

  if((fp = fopen(fname, "w")) == NULL) {
    ulog(ULOG_WARN, "nn_write_binary: unable to open '%s': %m.", fname);
    return(1);
  }
  write_internal(nn, fp, 1, 0);
  return(0);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
