
/* Copyright (c) 1995 by G. W. Flake. */

#include <math.h>
#include <memory.h>
#include <string.h>
#include <stdarg.h>

#include "nodelib/xalloc.h"
#include "nodelib/misc.h"
#include "nodelib/ulog.h"
#include "nodelib/array.h"

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void nodelib_version(void)
{
  ulog(ULOG_INFO, "NODElib version information\n");
  ulog(ULOG_INFO, "---------------------------\n");
  ulog(ULOG_INFO, "     USER : %s\n", NODELIB_USER);
  ulog(ULOG_INFO, "     HOST : %s\n", NODELIB_HOST);
  ulog(ULOG_INFO, "     DATE : %s\n", NODELIB_DATE);
  ulog(ULOG_INFO, "    UNAME : %s\n", NODELIB_UNAME);
  ulog(ULOG_INFO, "  VERSION : %s\n", NODELIB_VERSION);
  ulog(ULOG_INFO, " COMPILER : %s\n", NODELIB_COMPILER);
  ulog(ULOG_INFO, "COPYRIGHT : %s\n", NODELIB_COPYRIGHT);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static void *get_space(size_t dim, size_t elemsz, unsigned *sizes,
		       char **pointers, char **data)
{
  void *result, **hold;
  unsigned i;
  
  if(dim == 1) {
    result = *data;
    *data += sizes[0] * elemsz;
    return(result);
  }
  else {
    result = *pointers;
    hold = result;
    *pointers += sizes[0] * sizeof(void *);
    for(i = 0; i < sizes[0]; i++)
      hold[i] = get_space(dim - 1, elemsz, sizes + 1, pointers, data);
    return(result);
  }
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static void *va_allocate_array(size_t dim, size_t elemsz, va_list args)
{
  extern const size_t xalloc_magic;
  size_t metotal, petotal, align, offset, total;
  size_t mtotal = 1, ptotal = 0;
  unsigned i, *sizes;
  char *pointers, *data;
  void *result;

  if(dim == 1)
    return(xmalloc(elemsz * (va_arg(args, int))));
  else {
    sizes = xmalloc(sizeof(unsigned) * dim);
    for(i = 0; i < dim; i++) {
      sizes[i] = va_arg(args, int);
      mtotal *= sizes[i];
      if(i < dim - 1)
	ptotal += mtotal;
    }
    metotal = mtotal * elemsz;
    petotal = ptotal * sizeof(void *);
    
    /* We need to consider the alignment of the type when computing
     * total bytes needed.  We assume xalloc_magic specifies the
     * strictest machine type alignment.
     */
    if(petotal != xalloc_magic * (petotal / xalloc_magic))
      align = ((petotal / xalloc_magic) + 1) * xalloc_magic - petotal;
    else
      align = 0;

    total = petotal + align + metotal;
    offset = petotal + align;
    pointers = xmalloc(total);
    data = pointers + offset;

    result = get_space(dim, elemsz, sizes, &pointers, &data);
    xfree(sizes);
    return(result);
  }
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void *allocate_array(size_t dim, size_t elemsz, ...)
{
  va_list args;
  void *result;
  
  va_start(args, elemsz);
  result = va_allocate_array(dim, elemsz, args);
  va_end(args);
  return(result);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void deallocate_array(void *ptr)
{
  xfree(ptr);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#define MAXLINELEN 70

void pretty_print_option(char *str, int maxlen)
{
  int len, i;
  char *ptr;

  len = strlen(str);
  if(len + maxlen < MAXLINELEN) {
    fprintf(stderr, str);
    return;
  }
  str = strdup(str);
  ptr = (len < MAXLINELEN) ? str + len : str + MAXLINELEN;
  while(ptr > str) {
    if(*ptr == ' ' && ((ptr - str) + maxlen) < MAXLINELEN)
      break;
    ptr--;
  }
  if(*ptr == ' ') {
    *ptr = 0;
    fprintf(stderr, str);
    *ptr = ' ';
    fputc('\n', stderr);
    for(i = 0; i < maxlen + 5; i++)
      fputc(' ', stderr);
    pretty_print_option(ptr, maxlen);
  }
  else 
    fprintf(stderr, str);
  free(str);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void display_options(char *progname, OPTION *options, char *help)
{
  int i, j, maxlen, len;

  fprintf(stderr, "Usage: %s [ options ]\n", progname);
  if(help) fprintf(stderr, "\n%s\n", help);
  fprintf(stderr, "Options with defaults in parentheses are:\n\n");
  
  i = maxlen = 0;
  while(options[i].name != NULL) {
    if(maxlen < (len = strlen(options[i].name)))
      maxlen = len;
    i++;
  }
  
  i = 0;
  while(options[i].name != NULL) {
    fprintf(stderr, "  %s   ", options[i].name);
    len = strlen(options[i].name);
    for(j = 0; j < (maxlen - len + 1); j++)
      fputc(' ', stderr);
    if (options[i].type == OPT_SET) {
      OPTION_SET_MEMBER *mems = ((OPTION_SET *)options[i].ptr)->members;
      void **dest = ((OPTION_SET *)options[i].ptr)->dest;
      ARRAY *buff;
      char *p;
      unsigned cnt;
      
      for (cnt = 0; mems[cnt].name != NULL; cnt++) ;
      buff = array_create(1024, char);
      array_append_ptr(buff, options[i].help, strlen(options[i].help));
      p = " -- valid options are: ";
      array_append_ptr(buff, p, strlen(p));
      for (j = 0;  j < cnt; j++) {
	array_append_ptr(buff, mems[j].name, strlen(mems[j].name));
	if (cnt > 2 && j < cnt - 1) {
	  p = ", ";
	  array_append_ptr(buff, p, strlen(p));
	}
	if (cnt == 2 && j == 0) {
	  p = " and ";
	  array_append_ptr(buff, p, strlen(p));
	}
	else if (j == cnt - 2) {
	  p = "and ";
	  array_append_ptr(buff, p, strlen(p));
	}
      }
      p = " (";
      array_append_ptr(buff, p, strlen(p));
      p = "unknown";
      for (j = 0;  j < cnt; j++)
	if (mems[j].value == *dest) {
	  p = mems[j].name;
	  break;
	}
      array_append_ptr(buff, p, strlen(p));
      p = ")";
      array_append_ptr(buff, p, strlen(p));

      p = array_to_pointer(buff);
      pretty_print_option(p, maxlen);
      xfree(p);
      array_destroy(buff);
    }
    else {
      pretty_print_option(options[i].help, maxlen);
      switch(options[i].type) {
        case OPT_INT:
          fprintf(stderr, " (%d)", *(int *)options[i].ptr);
          break;
        case OPT_DOUBLE:
	  fprintf(stderr, " (%g)", *(double *)options[i].ptr);
	  break;
        case OPT_STRING:
	  if(*(char **)options[i].ptr)
	    fprintf(stderr, " (\"%s\")", *(char **)options[i].ptr);
	  else
	    fprintf(stderr, " (\"(NULL)\")");
        break;
        case OPT_SWITCH:
	  fprintf(stderr, (*(int *)options[i].ptr) ? " (ON)" : " (OFF)");
	  break;
        default:
	  break;
      }
    }
    fprintf(stderr, "\n");
    i++;
  }
  fprintf(stderr, "\n");
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void bad_option(char **argv, int badopt)
{
  fprintf(stderr, "%s: unknown or incorrectly used option \"%s\".\n",
          argv[0], argv[badopt]);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

int convert_set(OPTION_SET *set, char *arg)
{
  void **dest = set->dest;
  OPTION_SET_MEMBER *mems = set->members;
  unsigned i;

  for (i = 0; mems[i].name != NULL; i++)
    if (strcmp(arg, mems[i].name) == 0) {
      *dest = mems[i].value;
      return 0;
    }
  return 1;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#define PROCESS_OPTION_TYPE(TYPE, CONVERTER)                   \
          do {                                                 \
            if(i + 1 >= argc) {                                \
              bad_option(argv, i);                             \
              display_options(argv[0], options, help);         \
	      exit(1);                                         \
            }                                                  \
            *(TYPE *)options[j].ptr =                          \
                                (TYPE)CONVERTER(argv[i + 1]);  \
            i += 2;                                            \
          } while(0)


int get_options(int argc, char **argv, OPTION *options, char *help,
		 void *obj, int rest)
{
  int i, j, found;

  /* For each argument in the command line.
   */
  i = 1;
  while(i < argc) {
    found = 0, j = 0;
    /*
     * Search for the current word in the option list.
     */
    while(!found) {
      /*
       * If the option was not found.
       */
      if(strcmp(argv[i], "-help") == 0) {
        display_options(argv[0], options, help);
	exit(0);
      }
      else if(options[j].name == NULL) {
	if(rest) return(i);
        bad_option(argv, i);
        display_options(argv[0], options, help);
	exit(1);
      }
      /*
       * If the word is found in the option list.
       */ 
      else if(strcmp(argv[i], options[j].name) == 0) {
        /*
         * Check the type, make sure we have another arg
         * coming if needed, eat up the next arg, set
         * the value of the parameter, and display help
         * if needed.
         */
        found = 1;
        switch(options[j].type) {
          case SL_S_CHAR: PROCESS_OPTION_TYPE(char, *); break;
          case SL_U_CHAR: PROCESS_OPTION_TYPE(char, *); break;
          case SL_S_SHORT: PROCESS_OPTION_TYPE(short, atoi); break;
          case SL_U_SHORT: PROCESS_OPTION_TYPE(short, atoi); break;
          case SL_S_INT: PROCESS_OPTION_TYPE(int, atoi); break;
          case SL_U_INT: PROCESS_OPTION_TYPE(unsigned, atoi); break;
          case SL_S_LONG: PROCESS_OPTION_TYPE(long, atoi); break;
          case SL_U_LONG: PROCESS_OPTION_TYPE(unsigned long, atoi); break;
          case SL_DOUBLE: PROCESS_OPTION_TYPE(double, atof); break;
          case SL_FLOAT: PROCESS_OPTION_TYPE(float, atof); break;
          case SL_STRING:
            if(i + 1 >= argc) {
              bad_option(argv, i);
              display_options(argv[0], options, help);
	      exit(1);
            }
            *(char **)options[j].ptr = argv[i + 1];
            i += 2;
            break;
	  case SL_SWITCH:
            *(int *)options[j].ptr = !*(int *)options[j].ptr;
            i += 1;
            break;
          case SL_SET:
            if(i + 1 >= argc) {
              bad_option(argv, i);
              display_options(argv[0], options, help);
	      exit(1);
            }
            if(convert_set(options[j].ptr, argv[i + 1])) {
              bad_option(argv, i);
              display_options(argv[0], options, help);
	      exit(1);
            }
            i += 2;
	    break;
          case SL_OTHER:
            if(((int (*)())options[j].ptr)(argv, argc, options, &i, j,
					   obj)) {
              bad_option(argv, i);
              display_options(argv[0], options, help);
	      exit(1);
            }
            break;
	  default:
            break;
	}
      }
      j++;
    }
  }
  return(argc);
}

#undef PROCESS_OPTION_TYPE

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

double random_range(double low, double high)
{
  double tmp;

  tmp = fabs((double)(random()) / RANDOM_MAX);
  tmp = tmp * (high - low) + low;
  return(tmp);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

double random_gauss(void)
{
  static int holding = 0;  /* Are we holding an old value? */
  static double hold;      /* The last value that we save. */
  double factor, r, v1, v2;

  if(holding) {
    holding = 0;
    return(hold);
  }
  else {
    do {
      v1 = random_range(-1.0, 1.0);
      v2 = random_range(-1.0, 1.0);
      r = v1 * v1 + v2 * v2;
    } while(r >= 1.0);
    factor = sqrt(-2.0 * log(r) / r);
    hold = v1 * factor;
    holding = 1;
    return(v2 * factor);
  }
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

char *simple_basename(char *str)
{
  int i, sz = strlen(str);
  
  for(i = sz; i > 0; i--)
    if(str[i] == '/') return(str + i + 1);
  return(str);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void shuffle_indices(int *indices, int n)
{
  int i, j, swap;

  for (i = 0; i < n; i++) indices[i] = i;
  for (i = 0; i < n; i++) {
    j = i + random() % (n - i);
    swap = indices[i];
    indices[i] = indices[j];
    indices[j] = swap;
  }
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void shuffle_unsigned_indices(unsigned *indices, unsigned n)
{
  unsigned i, j, swap;

  for (i = 0; i < n; i++) indices[i] = i;
  for (i = 0; i < n; i++) {
    j = i + random() % (n - i);
    swap = indices[i];
    indices[i] = indices[j];
    indices[j] = swap;
  }
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

