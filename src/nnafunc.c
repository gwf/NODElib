
/* Copyright (c) 1995 by G. W. Flake. */

#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

#include "nodelib/nn.h"
#include "nodelib/xalloc.h"
#include "nodelib/hash.h"

static HASH *afhash = NULL;
static void afinit(void);

#define SGN(x) ((x < 0) ? -1 : 1)

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

NN_ACTFUNC *nn_set_actfunc(NN *nn, unsigned layer, unsigned slab, char *name)
{
  NN_ACTFUNC *af;

  if(nn_check_valid_slab(nn, layer, slab))
    return(NULL);
  if((af = nn_find_actfunc(name)) == NULL) {
    ulog(ULOG_ERROR, "nn_set_actfunc: unknown function name: \"%s\"", name);
    return(NULL);
  }
  nn->layers[layer].slabs[slab].afunc = af;
  return(af);    
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void nn_register_actfunc(char *name,
			 double (*func)(double input),
			 double (*deriv)(double input, double output),
                         double (*second_deriv)(double input,
						double output,
						double deriv))
{
  NN_ACTFUNC af, *afx;

  if(afhash == NULL) afinit();
  af.name = name;
  if((afx = hash_search(afhash, &af)) != NULL) {
    /* Found named act func, so just change the function ptrs. */
    afx->func = func;
    afx->deriv = deriv;
    afx->second_deriv = second_deriv;
  }
  else {
    afx = xmalloc(sizeof(NN_ACTFUNC));
    afx->name = name;
    afx->func = func;
    afx->deriv = deriv;
    afx->second_deriv = second_deriv;
    hash_insert(afhash, afx);
  }
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

NN_ACTFUNC *nn_find_actfunc(char *name)
{
  NN_ACTFUNC af;
  
  if(afhash == NULL) afinit();
  af.name = name;
  return(hash_search(afhash, &af));
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static void free_actfunc(void *af)
{ 
  xfree(af);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void nn_shutdown_actfuncs(void)
{
  if(afhash)
    hash_do_func(afhash, free_actfunc);
  hash_destroy(afhash);
  afhash = NULL;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static double aftanhf(double in)
{
  return(tanh(in));
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static double aftanhd(double in, double out)
{
  return((1 + out) * (1 - out));
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static double aftanhdd(double in, double out, double deriv)
{
  return(-2 * deriv * out);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static double afsinf(double in)
{
  return(sin(in));
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static double afsind(double in, double out)
{
  return(cos(in));
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static double afsindd(double in, double out, double deriv)
{
  return(-out);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static double afcosf(double in)
{
  return(cos(in));
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static double afcosd(double in, double out)
{
  return(-sin(in));
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static double afcosdd(double in, double out, double deriv)
{
  return(-out);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static double aflogisticf(double in)
{
  if(in > 1000.0)
    return(1.0 - 1e-8);
  else if(in < -1000.0)
    return(0.0 + 1e-8);
  return 1 / (exp(-in) + 1);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static double aflogisticd(double in, double out)
{
  return(out * (1 - out));
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static double aflogisticdd(double in, double out, double deriv)
{
  return(deriv * (1 - 2 * out));
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static double afgaussf(double in)
{
  return(exp(-in*in));
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static double afgaussd(double in, double out)
{
  return(-2.0 * out * in);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static double afgaussdd(double in, double out, double deriv)
{
  return(-2.0 * (out + deriv * in));
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static double afexpf(double in)
{
  return(exp(in));
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static double afexpd(double in, double out)
{
  return(out);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static double afexpdd(double in, double out, double deriv)
{
  return(deriv);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static double afexpnxf(double in)
{
  return(exp(-in));
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static double afexpnxd(double in, double out)
{
  return(-out);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static double afexpnxdd(double in, double out, double deriv)
{
  return(out);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static double aflinearf(double in)
{
  return(in);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static double aflineard(double in, double out)
{
  return(1);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static double aflineardd(double in, double out, double deriv)
{
  return(0);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static int afcmp(const void *a, const void *b, void *obj)
{
  const NN_ACTFUNC *x = a, *y = b;
  
  return(strcmp(x->name, y->name));
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static unsigned long afnumify(const void *a, void *obj)
{
  const NN_ACTFUNC *x = a;

  return(hash_string_numify(x->name));
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static void afinit(void)
{
  if(afhash == NULL) {
    afhash = hash_create(64, afnumify, afcmp, NULL);
    nn_register_actfunc("logistic", aflogisticf, aflogisticd, aflogisticdd);
    nn_register_actfunc("sigmoid", aflogisticf, aflogisticd, aflogisticdd);
    nn_register_actfunc("tanh", aftanhf, aftanhd, aftanhdd);
    nn_register_actfunc("gauss", afgaussf, afgaussd, afgaussdd);
    nn_register_actfunc("gaussian", afgaussf, afgaussd, afgaussdd);
    nn_register_actfunc("exp", afexpf, afexpd, afexpdd);
    nn_register_actfunc("exp(-x)", afexpnxf, afexpnxd, afexpnxdd);
    nn_register_actfunc("lin", aflinearf, aflineard, aflineardd);
    nn_register_actfunc("none", aflinearf, aflineard, aflineardd);
    nn_register_actfunc("linear", aflinearf, aflineard, aflineardd);
    nn_register_actfunc("sin", afsinf, afsind, afsindd);
    nn_register_actfunc("sine", afsinf, afsind, afsindd);
    nn_register_actfunc("cos", afcosf, afcosd, afcosdd);
    nn_register_actfunc("cosine", afcosf, afcosd, afcosdd);
  }
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


