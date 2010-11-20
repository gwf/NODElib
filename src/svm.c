
/* Copyright (c) 1998 by G. W. Flake. */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <limits.h>

#include "nodelib/xalloc.h"
#include "nodelib/ulog.h"
#include "nodelib/misc.h"
#include "nodelib/svm.h"

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

double svm_output(SVM *svm, double *x)
{
  unsigned i;
  double sum = 0;

  if (!svm->regression) {
    for (i = 0; i < svm->sz; i++)
      sum += svm->kernel(svm->x[i], x, svm->aux, svm->xdim) *
	svm->alpha[i] * svm->y[i];
  }
  else {
    for (i = 0; i < svm->sz; i++)
      sum += svm->kernel(svm->x[i], x, svm->aux, svm->xdim) * svm->alpha[i];
  }
  return sum - svm->b;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void svm_write_fp(SVM *svm, FILE *fp)
{
  unsigned i, j;

  fprintf(fp, "%u\n", svm->regression);
  fprintf(fp, "%u\n", svm->sz);
  fprintf(fp, "%u\n", svm->xdim);
  fprintf(fp, "% f\n", svm->b);
  fprintf(fp, "% f\n", svm->aux);
  if (svm->kernel == svm_kernel_gauss) fprintf(fp, "1\n");
  else if (svm->kernel == svm_kernel_poly) fprintf(fp, "2\n");
  else if (svm->kernel == svm_kernel_tanh) fprintf(fp, "3\n");
  else if (svm->kernel == svm_kernel_coulomb) fprintf(fp, "4\n");
  else fprintf(fp, "4\n");
  for (i = 0; i < svm->sz; i++) {
    fprintf(fp, "% .20e % f ", svm->alpha[i], svm->y[i]);
    for (j = 0; j < svm->xdim; j++) 
      fprintf(fp, "% f ", svm->x[i][j]);
    fprintf(fp, "\n");
  }
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

SVM *svm_read_fp(FILE *fp)
{
  SVM *svm;
  unsigned i, j, ktype;

  svm = xmalloc(sizeof(SVM));
  svm->alpha = svm->y = NULL;
  svm->x = NULL;

  if (fscanf(fp, "%u", &(svm->regression)) != 1) goto bad_file;
  if (fscanf(fp, "%u", &(svm->sz)) != 1) goto bad_file;
  if (fscanf(fp, "%u", &(svm->xdim)) != 1) goto bad_file;
  if (fscanf(fp, "%lf", &(svm->b)) != 1) goto bad_file;
  if (fscanf(fp, "%lf", &(svm->aux)) != 1) goto bad_file;
  if (fscanf(fp, "%u", &ktype) != 1) goto bad_file;

  if (ktype == 1) svm->kernel = svm_kernel_gauss;
  else if (ktype == 2) svm->kernel = svm_kernel_poly;
  else if (ktype == 3) svm->kernel = svm_kernel_tanh;
  else if (ktype == 4) svm->kernel = svm_kernel_coulomb;
  else svm->kernel = svm_kernel_linear;
  
  svm->alpha = xmalloc(sizeof(double) * svm ->sz);
  svm->y = xmalloc(sizeof(double) * svm ->sz);
  svm->x = allocate_array(2, sizeof(double), svm->sz, svm->xdim);

  for (i = 0; i < svm->sz; i++) {
    if (fscanf(fp, "%lf %lf ", &(svm->alpha[i]), &(svm->y[i])) != 2)
      goto bad_file;
    for (j = 0; j < svm->xdim; j++) 
      if (fscanf(fp, "%lf ", &(svm->x[i][j])) != 1)
	goto bad_file;
  }
  return svm;

 bad_file:
  if (svm) svm_destroy(svm);
  fclose(fp);
  ulog(ULOG_WARN, "svm_read_fp: error reading file");
  return NULL;  
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

SVM *svm_read(char *fname)
{
  FILE *fp;
  SVM *svm;

  fp = fopen(fname, "r");
  if(fp == NULL) {
    ulog(ULOG_WARN, "svm_read: unable to open '%s': %m.", fname);
    return(NULL);
  }
  svm = svm_read_fp(fp);
  fclose(fp);
  return svm;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

int svm_write(SVM *svm, char *fname)
{
  FILE *fp;

  fp = fopen(fname, "w");
  if(fp == NULL) {
    ulog(ULOG_WARN, "svm_write: unable to open '%s': %m.", fname);
    return 1;
  }
  svm_write_fp(svm, fp);
  fclose(fp);
  return 0;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void svm_destroy(SVM *svm)
{
  if (svm->alpha) xfree(svm->alpha);
  if (svm->y) xfree(svm->y);
  if (svm->x) deallocate_array(svm->x);
  xfree(svm);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
