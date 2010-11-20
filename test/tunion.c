
/* Copyright (c) 1997 by G. W. Flake. */

/* A test for the union DATASET type */

#include <nodelib.h>
#include <stdio.h>

#define SIZE  7
#define COUNT 5

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

int main(int argc, char **argv)
{
  SERIES *ser;
  DSM_UNION *dsmunion;
  DATASET*ds;
  double *x, *y;
  int i, j, sz, num;

  num = 0;
  dsmunion = dsm_union();
  for(i = 0; i < COUNT; i++) {
    ser = series_create();
    for(j = 0; j < SIZE; j++) {
      series_append_val(ser, num);
      series_append_val(ser, -num);
      num++;
    }
    ser->x_width = ser->y_width = ser->x_delta =
      ser->y_delta = ser->offset = 1;
    ser->step = 2;
    
    dsm_union_add(dsmunion, dataset_create(&dsm_series_method, ser));
  }
  
  ds = dataset_create(&dsm_union_method, dsmunion);

  printf("size = %d\n", dataset_size(ds));
  printf("x_size = %d\n", dataset_x_size(ds));
  printf("y_size = %d\n", dataset_y_size(ds));
  
  sz = dataset_size(ds);
  for(i = 0; i < sz; i++) {
    x = dataset_x(ds, i);
    y = dataset_y(ds, i);
    printf("%d - %f %f\n", i, x[0], y[0]);
  }
  exit(0);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


