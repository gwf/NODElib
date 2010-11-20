
/* Copyright (c) 1997 by G. W. Flake. */

/* A test for the array_destroy_index routine */

#include <nodelib.h>
#include <stdio.h>
#include <string.h>

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

int main(int argc, char **argv)
{
  ARRAY *a;
  SCAN *scan;
  char *str;
  int i;

  scan = scan_create(1, stdin);
  scan->delims = "+-=/";
  scan->whites = " \t\n";
  scan->comments = "";
  
  a = array_create(5, char);

  printf("> ");
  while((str = scan_get(scan)) != NULL) {
    if(*str == '=') {
      scan->delims = ""; scan->whites = "\n";
      str = scan_get(scan);
      scan->delims = "+-=/"; scan->whites = " \t\n";
      array_clear(a);
      array_append_ptr(a, str, strlen(str));
    }
    else if(*str == '+') {
      scan->delims = ""; scan->whites = "\n";
      str = scan_get(scan);
      scan->delims = "+-=/"; scan->whites = " \t\n";
      printf("append(%s)\n", str);
      array_append_ptr(a, str, strlen(str));      
    }
    else if(*str == '-') {
      scan->delims = ""; scan->whites = "\n";
      str = scan_get(scan);
      scan->delims = "+-=/"; scan->whites = " \t\n";
      printf("prepend(%s)\n", str);
      array_prepend_ptr(a, str, strlen(str));      
    }
    else if(*str == '/') {
      str = scan_get(scan);
      array_destroy_index(a, atoi(str));
    }
    else if(*str == '^') {
      str = scan_get(scan);
      i = atoi(str);
      str = scan_get(scan);
      scan->delims = "+-=/"; scan->whites = " \t\n";
      array_insert_index(a, *str, char, i);
    }
    str = array_to_pointer(a);
    printf(" \"%s\"\n> ", str);
    xfree(str);
  }
  exit(0);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

