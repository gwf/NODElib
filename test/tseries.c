
/* Copyright (c) 1997 by G. W. Flake. */

/* A test for the ver_deltas in the ARRAY type. */

#include <nodelib.h>
#include <stdio.h>
#include <string.h>

#define SZ  100
#define XSZ 6
#define YSZ 3

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

int main(int argc, char **argv)
{
  FILE *fp;
  SERIES *ser;
  double *x, *y;
  unsigned i, j, sz;
  unsigned vdx[XSZ - 1] = { 1, 2, 4, 8, 16 };
  unsigned vdy[YSZ - 1] = { 3, 6 };

  ser = series_create();
  for(i = 0; i < SZ; i++)
    series_append_val(ser, i);
  ser->x_width = XSZ;
  ser->y_width = YSZ;
  ser->x_delta = 2;
  ser->y_delta = 1;
  ser->step = 1;
  ser->offset = 2;

  series_reinitiate(ser);
  fp = fopen("tseries-xy.out", "w");
  sz = series_get_num_pat(ser);
  for(i = 0; i < sz; i++) {
    x = series_get_x_pat(ser, i);
    for(j = 0; j < XSZ; j++)
      fprintf(fp, j < (XSZ - 1) ? "%d\t" : "%d\t\t", (int)x[j]);
    y = series_get_y_pat(ser, i);
    for(j = 0; j < YSZ; j++)
      fprintf(fp, j < (YSZ - 1) ? "%d\t" : "%d\n", (int)y[j]);
  }
  fclose(fp);
		
  ser->var_x_deltas = vdx;
  ser->var_y_deltas = NULL;
  series_reinitiate(ser);
  fp = fopen("tseries-Xy.out", "w");
  sz = series_get_num_pat(ser);
  for(i = 0; i < sz; i++) {
    x = series_get_x_pat(ser, i);
    for(j = 0; j < XSZ; j++)
      fprintf(fp, j < (XSZ - 1) ? "%d\t" : "%d\t\t", (int)x[j]);
    y = series_get_y_pat(ser, i);
    for(j = 0; j < YSZ; j++)
      fprintf(fp, j < (YSZ - 1) ? "%d\t" : "%d\n", (int)y[j]);
  }
  fclose(fp);
		
  ser->var_x_deltas = NULL;
  ser->var_y_deltas = vdy;
  series_reinitiate(ser);
  fp = fopen("tseries-xY.out", "w");
  sz = series_get_num_pat(ser);
  for(i = 0; i < sz; i++) {
    x = series_get_x_pat(ser, i);
    for(j = 0; j < XSZ; j++)
      fprintf(fp, j < (XSZ - 1) ? "%d\t" : "%d\t\t", (int)x[j]);
    y = series_get_y_pat(ser, i);
    for(j = 0; j < YSZ; j++)
      fprintf(fp, j < (YSZ - 1) ? "%d\t" : "%d\n", (int)y[j]);
  }
  fclose(fp);
		
  ser->var_x_deltas = vdx;
  ser->var_y_deltas = vdy;
  series_reinitiate(ser);
  fp = fopen("tseries-XY.out", "w");
  sz = series_get_num_pat(ser);
  for(i = 0; i < sz; i++) {
    x = series_get_x_pat(ser, i);
    for(j = 0; j < XSZ; j++)
      fprintf(fp, j < (XSZ - 1) ? "%d\t" : "%d\t\t", (int)x[j]);
    y = series_get_y_pat(ser, i);
    for(j = 0; j < YSZ; j++)
      fprintf(fp, j < (YSZ - 1) ? "%d\t" : "%d\n", (int)y[j]);
  }
  fclose(fp);
		
  exit(0);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

