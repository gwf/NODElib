
/* Copyright (c) 1998 by G. W. Flake. */

/* A test for the SVM routines */

#include <nodelib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

typedef struct DSSHORT {
  short *data;
  double *xbuf1, *xbuf2, *ybuf1, *ybuf2;
  unsigned sz, xsz, ysz, whichx, whichy;
} DSSHORT;

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

INLINE unsigned dsshort_size(void *instance)
{
  DSSHORT *dsshort = instance;
  return dsshort->sz;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

INLINE unsigned dsshort_x_size(void *instance)
{
  DSSHORT *dsshort = instance;
  return dsshort->xsz;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

INLINE unsigned dsshort_y_size(void *instance) 
{
  DSSHORT *dsshort = instance;
  return dsshort->ysz;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

INLINE double *dsshort_x(void *instance, unsigned index)
{
  DSSHORT *dsshort = instance;
  unsigned i;
  double *dst;
  short *src;
  
  dst = (dsshort->whichx ? dsshort->xbuf1 : dsshort->xbuf2);
  src = dsshort->data + index * (dsshort->xsz + dsshort->ysz);
  dsshort->whichx = (dsshort->whichx ? 0 : 1);
  for (i = 0; i < dsshort->xsz; i++)
    dst[i] = src[i] / 1000.0;
  return dst;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

INLINE double *dsshort_y(void *instance, unsigned index)
{
  DSSHORT *dsshort = instance;
  unsigned i;
  double *dst;
  short *src;
  
  dst = (dsshort->whichy ? dsshort->ybuf1 : dsshort->ybuf2);
  src = dsshort->data + index * (dsshort->xsz + dsshort->ysz) + dsshort->xsz;
  dsshort->whichy = (dsshort->whichy ? 0 : 1);
  for (i = 0; i < dsshort->ysz; i++)
    dst[i] = src[i] / 1000.0;
  return dst;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

DATASET_METHOD dsshort_method = {
  dsshort_size,
  dsshort_x_size,
  dsshort_y_size,
  dsshort_x,
  dsshort_y
};

DATASET *create_short_dataset(char *fname, int xdim, int ydim)
{
  DSSHORT *dsshort;
  DATASET *ds;
  FILE *fp;
  struct stat fpstat;
  unsigned bytes;

  if(stat(fname, &fpstat)) {
    perror("could not stat file: ");
    exit(1);
  }
  bytes = fpstat.st_size;
  if (bytes % ((xdim + ydim) * sizeof(short)) != 0) {
    fprintf(stderr, "file size not a multiple of"
	    "((xdim + ydim) * sizeof(short))\n");
    exit(1);
  }

  dsshort = xmalloc(sizeof(DSSHORT));
  dsshort->data = xmalloc(bytes);
  dsshort->sz = bytes / ((xdim + ydim) * sizeof(short));
  dsshort->xsz = xdim;
  dsshort->ysz = ydim;
  dsshort->whichx = dsshort->whichy = 0;
  dsshort->xbuf1 = xmalloc(sizeof(double) * xdim);
  dsshort->xbuf2 = xmalloc(sizeof(double) * xdim);
  dsshort->ybuf1 = xmalloc(sizeof(double) * ydim);
  dsshort->ybuf2 = xmalloc(sizeof(double) * ydim);

  if ((fp = fopen(fname, "r")) == NULL) {
    fprintf(stderr, "could not open '%s' for reading\n", fname);
    exit(1);
  }
  if ((fread(dsshort->data, sizeof(char), bytes, fp)) != bytes) {
    fprintf(stderr, "problems reading data from '%s'\n", fname);
    exit(1);
  }
  fclose(fp);

  ds = dataset_create(&dsshort_method, dsshort);
  return ds;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

typedef struct DSDOUBLE {
  double *data;
  unsigned sz, xsz, ysz;
} DSDOUBLE;

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

INLINE unsigned dsdouble_size(void *instance)
{
  DSDOUBLE *dsdouble = instance;
  return dsdouble->sz;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

INLINE unsigned dsdouble_x_size(void *instance)
{
  DSDOUBLE *dsdouble = instance;
  return dsdouble->xsz;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

INLINE unsigned dsdouble_y_size(void *instance) 
{
  DSDOUBLE *dsdouble = instance;
  return dsdouble->ysz;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

INLINE double *dsdouble_x(void *instance, unsigned index)
{
  DSDOUBLE *dsdouble = instance;
  return dsdouble->data + index * (dsdouble->xsz + dsdouble->ysz);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

INLINE double *dsdouble_y(void *instance, unsigned index)
{
  DSDOUBLE *dsdouble = instance;
  return dsdouble->data + index * (dsdouble->xsz + dsdouble->ysz) + dsdouble->xsz;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

DATASET_METHOD dsdouble_method = {
  dsdouble_size,
  dsdouble_x_size,
  dsdouble_y_size,
  dsdouble_x,
  dsdouble_y
};

DATASET *create_double_dataset(char *fname, int xdim, int ydim)
{
  DSDOUBLE *dsdouble;
  DATASET *ds;
  FILE *fp;
  struct stat fpstat;
  unsigned bytes;

  if(stat(fname, &fpstat)) {
    perror("could not stat file: ");
    exit(1);
  }
  bytes = fpstat.st_size;
  if (bytes % ((xdim + ydim) * sizeof(double)) != 0) {
    fprintf(stderr, "file size not a multiple of"
	    "((xdim + ydim) * sizeof(double))\n");
    exit(1);
  }

  dsdouble = xmalloc(sizeof(DSDOUBLE));
  dsdouble->data = xmalloc(bytes);
  dsdouble->sz = bytes / ((xdim + ydim) * sizeof(double));
  dsdouble->xsz = xdim;
  dsdouble->ysz = ydim;

  if ((fp = fopen(fname, "r")) == NULL) {
    fprintf(stderr, "could not open '%s' for reading\n", fname);
    exit(1);
  }
  if ((fread(dsdouble->data, sizeof(char), bytes, fp)) != bytes) {
    fprintf(stderr, "problems reading data from '%s'\n", fname);
    exit(1);
  }
  fclose(fp);

  ds = dataset_create(&dsdouble_method, dsdouble);
  return ds;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

int myhook(SMORCH *smorch)
{
  fprintf(stderr, "%u\t%u\t%u\t% .12f\t%u\t%u\n", smorch->epoch,
	  smorch->num_changed, smorch->nonbound->count, smorch->objective,
	  smorch->cache_hit, smorch->cache_miss);
  return 0;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

int myfinalhook(SMORCH *smorch)
{
  fprintf(stderr, "% .12f\t% .12f\n",
	  smorch_objective(smorch), smorch_constraint(smorch));    
  return 0;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

int main(int argc, char **argv)
{
  int seed = 1, xdim = 0, ydim = 1, csz = 0, yindex = 0, ssz = 0;
  int clever = 0, regress = 0, dump = 0, best = 0, wfirst = 0, lazy = 0;
  int offset = 0, xdelta = 0, tube = 0;
  double trate = 2.0, tfinal = 10.0;
  double C = 100, aux = 0.5, tol = 1e-3, eps = 1e-8, regeps = 0.1;
  char *fname = NULL, *kname = "gauss", *dtype = "ascii";
  
  OPTION opts[] = {
    { "-dtype",  OPT_STRING,   &dtype,
      "data type. Value should be one of: "
      "ascii, double, short (short_val / 1000.0 = dbl_val), or map "
      "(memory-mapped file of doubles)" },
    { "-kernel", OPT_STRING,   &kname,
      "SVM kernel. Value should be one of: "
      "gauss, coulomb, poly, tanh, or linear" },
    { "-xdim",   OPT_INT,      &xdim,
      "dimensionality of input points" },
    { "-ydim",   OPT_INT,      &ydim,
      "dimensionality of target points" },
    { "-ssz",    OPT_INT,      &ssz,
      "subset size" },
    { "-fname",  OPT_STRING,   &fname,
      "data file name" },
    { "-seed",   OPT_INT,      &seed,
      "random number seed for shuffled indices" },
    { "-C",      OPT_DOUBLE,   &C,
      "maximum size for Lagrange multipliers" },
    { "-aux",    OPT_DOUBLE,   &aux,
       "auxiliary parameter: variance for Gaussian kernels, "
      "power for polynomials, and threshold for sigmoids" },
    { "-tol",    OPT_DOUBLE,   &tol,
      "tolerance for classification errors" },
    { "-eps",    OPT_DOUBLE,   &eps,
      "floating point epsilon" },
    { "-csz",    OPT_INT,      &csz,
      "kernel output cache size" },
    { "-yindex", OPT_INT,      &yindex,
      "which y[] to classify" },
    { "-clever", OPT_SWITCH,   &clever,
      "use 'ultra clever' incremental outputs" },
    { "-best",   OPT_SWITCH,   &best,
      "use best step if relatively easy to compute" },
    { "-wfirst", OPT_SWITCH,   &wfirst,
      "Always attempt to optimize worst KKT exemplar first" },
    { "-lazy",   OPT_SWITCH,   &lazy,
      "only do a hard search over all multipliers when necessary" },
    { "-tube",   OPT_SWITCH,   &tube,
      "use tube shrinking heuristic?" },
    { "-trate",  OPT_DOUBLE,   &trate,
      "tube shrinking factor" },
    { "-tfinal", OPT_DOUBLE,   &tfinal,
      "final tube shrinkage" },
    { "-regress",OPT_SWITCH,   &regress,
      "assume this is a regression problem (not classification)" },
    { "-regeps", OPT_DOUBLE,   &regeps,
      "epsilon for regression problems" },
    { "-xdelta", OPT_INT,      &xdelta,
      "space between x's (only for time delayed time series)" },
    { "-offset", OPT_INT,      &offset,
      "space between x's and y's (only for time delayed time series)" },
    { "-dump",   OPT_SWITCH,   &dump,
      "dump SVM output to file?" },
    { NULL,      OPT_NULL,     NULL,    NULL }
  };
  
  SERIES *ser;
  DSM_FILE *dsmfile;
  DATASET *data;
  SVM *svm;
  SMORCH smorch = SMORCH_DEFAULT;
  FILE *fp;
  unsigned i, sz, j;
  double *x, *y;
  
  get_options(argc, argv, opts, NULL, NULL, 0);
  
  if(fname == NULL || xdim <= 0) {
    display_options(argv[0], opts, NULL);
    exit(1);
  }
  
  srandom(seed);

  if (!strcmp(dtype, "ascii")) {
    ser = series_read_ascii(fname);
    ser->x_width = xdim;
    ser->y_width = ydim;
    if (xdelta > 0 && offset > 0) {
      ser->x_delta = xdelta;
      ser->offset = offset;
      ser->step = 1;
    }
    else {
      ser->x_delta = ser->y_delta = ser->offset = 1;
      ser->step = ser->x_width + ser->y_width;
    }
    data = dataset_create(&dsm_series_method, ser);
  }
  else if (!strcmp(dtype, "map")) {
    dsmfile = dsm_file(fname);
    dsmfile->x_width = xdim;
    dsmfile->y_width = ydim;
    dsmfile->x_read_width = xdim * sizeof(double);
    dsmfile->y_read_width = ydim * sizeof(double);
    dsmfile->offset = dsmfile->skip = 0;
    dsmfile->step = dsmfile->x_read_width + dsmfile->y_read_width;
    dsmfile->type = SL_DOUBLE;
    dsm_file_initiate(dsmfile);
    data = dataset_create(&dsm_file_method, dsmfile);
  }
  else if (!strcmp(dtype, "double")) {
    data = create_double_dataset(fname, xdim, ydim);
  }
  else if (!strcmp(dtype, "short")) {
    data = create_short_dataset(fname, xdim, ydim);      
  }
  else {
    display_options(argv[0], opts, NULL);
    exit(1);
  }
  
  smorch.data = data;
  
   /* Set up the proper kernel to use. */
  if(!strcmp(kname, "gauss"))
    smorch.kernel = svm_kernel_gauss;
  else if(!strcmp(kname, "poly"))
    smorch.kernel = svm_kernel_poly;
  else if(!strcmp(kname, "tanh"))
    smorch.kernel = svm_kernel_tanh;
  else if(!strcmp(kname, "linear"))
    smorch.kernel = svm_kernel_linear;
  else if(!strcmp(kname, "coulomb"))
    smorch.kernel = svm_kernel_coulomb;
  else {
    display_options(argv[0], opts, NULL);
    exit(1);
  }
  
  smorch.cache_size = csz;
  smorch.yindex = yindex;
  smorch.aux = aux;
  smorch.C = C;
  smorch.tol = tol;
  smorch.eps = eps;
  smorch.hook = myhook;
  smorch.finalhook = myfinalhook;
  smorch.subset_size = ssz;
  smorch.ultra_clever = clever;
  smorch.best_step = best;
  smorch.worst_first = wfirst;
  smorch.lazy_loop = lazy;
  smorch.regression = regress;
  smorch.regeps = regeps;
  smorch.tube = tube;
  smorch.tube_rate = trate;
  smorch.tube_final = tfinal;
  
  svm = smorch_train(&smorch);
  svm_write(svm, "tsvm.svm");

  if (dump) {
    fp = fopen("tsvm.tst", "w");
    sz = dataset_size(data);
    for (i = 0; i < sz; i++) {
      x = dataset_x(data, i);
      y = dataset_y(data, i);
      for (j = 0; j < xdim; j++)
	fprintf(fp, "% .4f ", x[j]);
      fprintf(fp, "% .4f ", y[yindex]);
      fprintf(fp, "% .4f\n", svm_output(svm, x));
    }
    fclose(fp);
  }
  
  svm_destroy(svm);
  
  exit(0);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
