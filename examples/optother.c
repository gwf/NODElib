
#include <nodelib.h>
#include <stdio.h>


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

int baz(char **argv, int argc, OPTION *options, int *cargc, int opt, void *obj)
{
  int i;

  if(argc == *cargc) return(1);
  for(i = *cargc + 1; i < argc; i++) {
    if(argv[i][0] == '-') {
      *cargc = i;
      return(0);
    }
    printf("baz[%d] = %s\n", i - *cargc, argv[i]);
  }
  *cargc = argc;
  return(0);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

int main(int argc, char **argv)
{
  int foo = 1, buz;
  char *bar = "My name is bar!";
  OPTION opts[] = {
    { "-foo",      OPT_INT,    &foo,  "foo - this is a very long "
                                      "description. My god! When will "
                                      "it end? Ohh, the pain!"          },
    { "-bar",      OPT_STRING, &bar,  "bar"                             },
    { "-buzinginfy",
                   OPT_SWITCH, &buz,  "Long name, short description."   },
    { "-buzinginfyinify",
                   OPT_SWITCH, &buz,  "Long name, long description that "
                                      "goes on, and on, and on, and on."},
    { "-baz",      OPT_OTHER,  baz,   "baz"                             },
    { NULL,        OPT_NULL,   NULL,  NULL                              }
  };

  get_options(argc, argv, opts, NULL, NULL, 0);
  
  printf("foo = %d\n", foo);
  printf("bar = \"%s\"\n", bar);
  
  return 0;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

