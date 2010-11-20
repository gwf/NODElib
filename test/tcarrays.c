
/* Copyright (c) 1996 by G. W. Flake. */

/* A test for the allocate_array() routine... */

#include <nodelib.h>
#include <stdio.h>
#include <string.h>

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

int main(int argc, char **argv)
{
  char *c, *cc = "Hello world! How are you! I am fine!";
  short **s, *ss;
  double ***d, *dd;
  unsigned ****u, *uu;
  unsigned i, j, k, l, okay, aokay = 1;

  c = allocate_array(1, sizeof(char), strlen(cc) + 1);
  strcpy(c, cc);
  deallocate_array(c);
  fprintf(stderr, "%s: passed 1d char\n", argv[0]);

  okay = 1;
  s = allocate_array(2, sizeof(short), 5, 7);
  for(i = 0; i < 5; i++)
    for(j = 0; j < 7; j++)
      s[i][j] = i * 10 + j;
  ss = &s[0][0];
  for(i = 0; i < 5; i++) {
    for(j = 0; j < 7; j++) {
      if(s[i][j] != i * 10 + j) {
	okay = 0;
	fprintf(stderr, "%s: failed 2d short pass 1\n", argv[0]);
	break;
      }
      if(s[i][j] != *ss) {
	okay = 0;
	fprintf(stderr, "%s: failed 2d short pass 2\n", argv[0]);
	break;
      }
      ss++;
    }
    if(!okay) break;
  }
  if(!okay) aokay = 0;
  else fprintf(stderr, "%s: passed 2d short\n", argv[0]);
  deallocate_array(s);

  okay = 1;
  d = allocate_array(3, sizeof(double), 5, 7, 3);
  for(i = 0; i < 5; i++)
    for(j = 0; j < 7; j++)
      for(k = 0; k < 3; k++)
	d[i][j][k] = i * 100 + j * 10 + k;
  dd = &d[0][0][0];
  for(i = 0; i < 5; i++) {
    for(j = 0; j < 7; j++) {
      for(k = 0; k < 3; k++) {
	if(d[i][j][k] != i * 100 + j * 10 + k) {
	  okay = 0;
	  fprintf(stderr, "%s: failed 3d double pass 1\n", argv[0]);
	  break;
	}
	if(d[i][j][k] != *dd) {
	  okay = 0;
	  fprintf(stderr, "%s: failed 2d double pass 2\n", argv[0]);
	  break;
	}
	dd++;
      }
      if(!okay) break;
    }
    if(!okay) break;
  }
  if(!okay) aokay = 0;
  else fprintf(stderr, "%s: passed 3d double\n", argv[0]);
  deallocate_array(d);
  
  okay = 1;
  u = allocate_array(4, sizeof(unsigned), 5, 7, 3, 11);
  for(i = 0; i < 5; i++)
    for(j = 0; j < 7; j++)
      for(k = 0; k < 3; k++)
	for(l = 0; l < 11; l++)
	  u[i][j][k][l] = i * 1000 + j * 100 + k * 10 + l;
  uu = &u[0][0][0][0];
  for(i = 0; i < 5; i++) {
    for(j = 0; j < 7; j++) {
      for(k = 0; k < 3; k++) {
	for(l = 0; l < 11; l++) {
	  if(u[i][j][k][l] != i * 1000 + j * 100 + k * 10 + l) {
	    okay = 0;
	    fprintf(stderr, "%s: failed 4d unsigned pass 1\n", argv[0]);
	    break;
	  }
	  if(u[i][j][k][l] != *uu) {
	    okay = 0;
	    fprintf(stderr, "%s: failed 4d unsigned pass 2\n", argv[0]);
	    break;
	  }
	  uu++;
	}
	if(!okay) break;
      }
      if(!okay) break;
    }
    if(!okay) break;
  }
  if(!okay) aokay = 0;
  else fprintf(stderr, "%s: passed 4d unsigned\n", argv[0]);
  deallocate_array(u);

  exit(!aokay);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
