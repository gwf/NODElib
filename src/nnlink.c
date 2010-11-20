
/* Copyright (c) 1995 by G. W. Flake. */

#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdarg.h>

#ifdef SUNOS
#include <varargs.h>
char *vsprintf(char *, char *, va_list);
#endif

#include "nodelib/misc.h"
#include "nodelib/nn.h"
#include "nodelib/array.h"
#include "nodelib/scan.h"

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static int scan_link_addr(NN *nn, int *addr, SCAN *s)
{
  char *t;

  if((t = scan_get(s)) == NULL)
    goto BADTOKEN;
  if(isdigit(t[0])) { /* A whole layer */
    addr[0] = atoi(t);
    addr[1] = -1;
    return(nn_check_valid_layer(nn, addr[0]));
  }
  else if(t[0] == '(') {
    if((t = scan_get(s)) == NULL)
      goto BADTOKEN;
    if(!isdigit(t[0]))
      goto BADTOKEN;
    addr[0] = atoi(t);
    if((t = scan_get(s)) == NULL)
      goto BADTOKEN;
    if(!isdigit(t[0]))
      goto BADTOKEN;
    addr[1] = atoi(t);
    if((t = scan_get(s)) == NULL)
      goto BADTOKEN;
    if(t[0] != ')')
      goto BADTOKEN;
    return(nn_check_valid_slab(nn, addr[0], addr[1]));
  }
 BADTOKEN:
  ulog(ULOG_ERROR, "nn_link: unexpected token in format: '%s'.", t);
  return(1);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static int scan_link_type(NN *nn, char *type, SCAN *s)
{
  char *t;

  *type = 'l';
  if((t = scan_get(s)) == NULL)
    goto BADTOKEN;
  if(t[0] != '-')
    goto BADTOKEN;
  if((t = scan_get(s)) == NULL)
    goto BADTOKEN;
  if(t[0] != '-') {
    if(!isalpha(t[0]))
      goto BADTOKEN;
    else if(t[1] == 0)
      *type = *t;
    else
      goto BADTOKEN;
    if((t = scan_get(s)) == NULL)
      goto BADTOKEN;
    if(t[0] != '-')
      goto BADTOKEN;
  }
  if((t = scan_get(s)) == NULL)
    goto BADTOKEN;
  if(t[0] != '>')
    goto BADTOKEN;
  return(0);
 BADTOKEN:
  ulog(ULOG_ERROR, "nn_link: unexpected token in format: '%s'.", t);
  return(1);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* Some possible link types: linear, quadratic, xquadratic, diagonal,
   product, copy, offdiag...

   FORMAT := ADDRS LINK ADDRS
   ADDRS  := ADDR | ADDR ADDRS
   ADDR   := (# #) | #
   #      := [0-9]+
   LINK   := -[a-z]->
 */

static int scan_link_format(NN *nn, char *f, NN_LAYERLIST **srcp,
			    NN_LAYERLIST **dstp, char *netf)
{
  NN_LAYERLIST *src, *dst;
  int addr[2];
  SCAN *s;
  char *t;

  s = scan_create(0, f);
  s->delims = "()->";
  src = dst = NULL;
  
  /* Get all of the source slabs... */
  do{
    if(scan_link_addr(nn, addr, s))
      goto BADTOKEN;
    if(addr[1] != -1)
      src = nn_layerlist_cons(&nn->layers[addr[0]].slabs[addr[1]], src);
    else
      src = nn_layerlist_cons(&nn->layers[addr[0]], src);
    t = scan_peek(s);
  } while(t != NULL && t[0] != '-');

  /* Get the link type... */
  if(scan_link_type(nn, netf, s))
    goto BADTOKEN;

  /* Get all of the destination slabs... */
  do{
    if(scan_link_addr(nn, addr, s))
      goto BADTOKEN;
    if(addr[1] != -1)
      dst = nn_layerlist_cons(&nn->layers[addr[0]].slabs[addr[1]], dst);
    else
      dst = nn_layerlist_cons(&nn->layers[addr[0]], dst);
    t = scan_peek(s);
  } while(t != NULL);

  *srcp = src;
  *dstp = dst;
  scan_destroy(s);
  return(0);
    
 BADTOKEN:
  scan_destroy(s);
  nn_layerlist_free(src);
  nn_layerlist_free(dst);
  return(1);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

NN_LINK *nn_link(NN *nn, char *format, ...)
{
  va_list args;
  char *buffer;
  unsigned numin, numout, numaux, weightbits;
  NN_LAYERLIST *src, *dst, *l;
  char netf[2] = "l";
  NN_NETFUNC *nf;
  NN_LINK *link;
  int sanity;

  va_start(args, format);
  buffer = xmalloc(strlen(format) * 3 * sizeof(char));
  vsprintf(buffer, format, args);
  format = buffer;

  /* Scan the format line and do some simple checking... */
  src = dst = NULL;
  if(scan_link_format(nn, format, &src, &dst, netf))
    goto BADFORMAT;
  if((nf = nn_find_netfunc(netf)) == NULL) {
    ulog(ULOG_ERROR, "nn_link: unknown link type '%c'.", netf[0]);
    goto BADFORMAT;
  }
  if((sanity = nf->sanity(nn, src, dst, &numin, &numout, &numaux)) == -1) {
    ulog(ULOG_ERROR, "nn_link: source/dest type badness.");
    goto BADFORMAT;
  }

  /* Start to allocate the link.  Assume the first source and the first
   * dest contain any and all size information that we need.
   */
  link = xmalloc(sizeof(NN_LINK));
  link->A = link->dA = NULL;
  link->u = link->du = link->v = link->dv = link->w = link->dw = NULL;
  link->a = link->da = link->b = link->db = NULL;
  link->RA = link->RdA = NULL;
  link->Ru = link->Rdu = link->Rv = link->Rdv = link->Rw = link->Rdw = NULL;
  link->Ra = link->Rda = link->Rb = link->Rdb = NULL;
  link->source = src;
  link->dest = dst;
  link->nfunc = nf;
  link->format = xmalloc((strlen(format) + 1) * sizeof(char));
  strcpy(link->format, format);
  /* Could be a PI connection which has no weight... */
  weightbits = sanity;
  link->numin = numin;
  link->numout = numout;
  link->numaux = numaux;
  link->numweights = 0;

  if(weightbits & NN_WMATRIX) {
    link->A = allocate_array(3, sizeof(double), numout, numin, numin);
    link->dA = allocate_array(3, sizeof(double), numout, numin, numin);
    link->RA = allocate_array(3, sizeof(double), numout, numin, numin);
    link->RdA = allocate_array(3, sizeof(double), numout, numin, numin);
    nn->numweights += numout * numin * numin;
    link->numweights += numout * numin * numin;
  }
  if(weightbits & NN_WVECTOR_1) {
    link->u = allocate_array(2, sizeof(double), numout, numin);
    link->du = allocate_array(2, sizeof(double), numout, numin);
    link->Ru = allocate_array(2, sizeof(double), numout, numin);
    link->Rdu = allocate_array(2, sizeof(double), numout, numin);
    nn->numweights += numout * numin;
    link->numweights += numout * numin;
  }
  if(weightbits & NN_WVECTOR_2) {
    link->v = allocate_array(2, sizeof(double), numout, numin);
    link->dv = allocate_array(2, sizeof(double), numout, numin);
    link->Rv = allocate_array(2, sizeof(double), numout, numin);
    link->Rdv = allocate_array(2, sizeof(double), numout, numin);
    nn->numweights += numout * numin;
    link->numweights += numout * numin;
  }
  if(weightbits & NN_WUSER) {
    link->w = allocate_array(2, sizeof(double), numout, numaux);
    link->dw = allocate_array(2, sizeof(double), numout, numaux);
    link->Rw = allocate_array(2, sizeof(double), numout, numaux);
    link->Rdw = allocate_array(2, sizeof(double), numout, numaux);
    nn->numweights += numout * numaux;
    link->numweights += numout * numaux;
  }
  if(weightbits & NN_WSCALAR_1) {
    link->a = allocate_array(1, sizeof(double), numout);
    link->da = allocate_array(1, sizeof(double), numout);
    link->Ra = allocate_array(1, sizeof(double), numout);
    link->Rda = allocate_array(1, sizeof(double), numout);
    nn->numweights += numout;
    link->numweights += numout;
  }
  if(weightbits & NN_WSCALAR_2) {
    link->b = allocate_array(1, sizeof(double), numout);
    link->db = allocate_array(1, sizeof(double), numout);
    link->Rb = allocate_array(1, sizeof(double), numout);
    link->Rdb = allocate_array(1, sizeof(double), numout);
    nn->numweights += numout;
    link->numweights += numout;
  }

  /* Now that the link is built, insert it into the source and destination
   * layers.
   */
  for(l = src; l != NULL; l = l->cdr)
    l->layer->out = nn_linklist_cons(link, l->layer->out);
  for(l = dst; l != NULL; l = l->cdr)
    l->layer->in = nn_linklist_cons(link, l->layer->in);

  nn->numlinks++;
  nn->links = xrealloc(nn->links, nn->numlinks * sizeof(NN_LINK *));
  nn->links[nn->numlinks - 1] = link;

  /* The following two statements must be together; otherwise,
   * the count on the number of weight will be wrong.
   */
  link->need_grads = 1;
  nn_unlock_link(nn, nn->numlinks - 1);

  xfree(buffer);
  return(link);

 BADFORMAT:
  ulog(ULOG_ERROR, "nn_link: offending format: \"%s\".", format);
  xfree(buffer);
  nn_layerlist_free(src);
  nn_layerlist_free(dst);
  return(NULL);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
