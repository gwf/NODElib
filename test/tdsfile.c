
/* Copyright (c) 1996 by G. W. Flake. */

/* A test for the allocate_array() routine... */

#include <nodelib.h>
#include <stdio.h>
#include <limits.h>

#define TESTFILE "/tmp/tdsfiletest.dat"

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

int main(int argc, char **argv)
{
  unsigned i, j, sz, xsz, ysz;
  double *d;
  DSM_FILE *dsmf;
  DATASET *data;
  FILE *fp;

  /* Write first test batch. */

  if((fp = fopen(TESTFILE, "w")) == NULL) {
    fprintf(stderr, "%s: cannot open test file.\n", argv[0]);
    exit(1);
  }
  for(i = 0; i <= UCHAR_MAX; i++)
    fprintf(fp, "%c", i);
  fclose(fp);

  dsmf = dsm_file(TESTFILE);
  dsmf->x_width = 2;
  dsmf->y_width = 3;
  dsmf->x_read_width = 2;
  dsmf->y_read_width = 3;
  dsmf->offset = -2;
  dsmf->step = 3;
  dsmf->type = SL_U_CHAR;
  dsm_file_initiate(dsmf);
  
  data = dataset_create(&dsm_file_method, dsmf);
  sz = dataset_size(data);
  xsz = dataset_x_size(data);
  ysz = dataset_y_size(data);
  for(i = 0; i < sz; i++) {
    d = dataset_x(data, i);
    for(j = 0; j < xsz; j++)
      fprintf(stdout, "%d\t", (int)d[j]);
    fprintf(stdout, "\t");
    d = dataset_y(data, i);
    for(j = 0; j < ysz; j++)
      fprintf(stdout, "%d\t", (int)d[j]);
    fprintf(stdout, "\n");
  }

  dsm_destroy_file(dataset_destroy(data));
  exit(0);  
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
