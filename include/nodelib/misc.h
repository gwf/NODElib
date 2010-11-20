
/* Copyright (c) 1995 by G. W. Flake.
 *
 * NAME
 *   misc.h - miscellaneous but useful routines
 * SYNOPSIS
 *   This module includes mutidimensional array allocation and
 *   deallocation routines, a command-line parsing function, and
 *   random number generators with uniform and Gaussian
 *   distributions.
 * AUTHOR
 *   Gary William Flake (\url{\bf{gary.flake@usa.net}}{mailto:gary.flake@usa.net}).
 */

#ifndef __MISC_H__
#define __MISC_H__

#include <stdio.h>
#include <stdarg.h>

#include "nodelib/etc/version.h"
#include "nodelib/etc/options.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
                          
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* Allocates and returns a \em{dim} dimensional array with the specified
   element size, \em{elemsz}. The caller should supply \em{dim}
   additional arguments which correspond to the dimension sizes.  As an
   example:
   \begin{verbatim}
   \bf
      char **foo = allocate_array (2, sizeof(char), 4, 6);
   \rm
   \end{verbatim}
   will enable you to reference foo[i][j] with i and j taking maximum
   values of 3 and 5. */

void *allocate_array(size_t dim, size_t elemsz, ...);

/* Frees all memory allocated in an \bf{allocate_array()} call.  */

void deallocate_array(void *ptr);


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* This is simply a list of basic C types augmented with a couple extra
   entries.  Some of NODElib's modules want a type name (e.g., the OPTION
   type below) so instead of defining several enums for each separate
   module we keep this one enum list for the whole library.  Not every
   value of the SL_TYPE will make sense for every usage, so beware.  */

typedef enum SL_TYPE {
  SL_NULL,
  SL_U_CHAR,    SL_S_CHAR,
  SL_U_SHORT,   SL_S_SHORT,
  SL_U_INT,     SL_S_INT,
  SL_U_LONG,    SL_S_LONG,
  SL_FLOAT,     SL_DOUBLE,
  SL_STRING,    SL_SWITCH,
  SL_SET,       SL_OTHER
} SL_TYPE;

#define OPT_CHAR      SL_S_CHAR
#define OPT_S_CHAR    SL_S_CHAR
#define OPT_U_CHAR    SL_U_CHAR
#define OPT_SHORT     SL_S_SHORT
#define OPT_S_SHORT   SL_S_SHORT
#define OPT_U_SHORT   SL_U_SHORT
#define OPT_INT       SL_S_INT
#define OPT_S_INT     SL_S_INT
#define OPT_U_INT     SL_U_INT
#define OPT_LONG      SL_S_LONG
#define OPT_S_LONG    SL_S_LONG
#define OPT_U_LONG    SL_U_LONG
#define OPT_FLOAT     SL_FLOAT
#define OPT_DOUBLE    SL_DOUBLE
#define OPT_STRING    SL_STRING
#define OPT_SWITCH    SL_SWITCH
#define OPT_SET       SL_SET
#define OPT_OTHER     SL_OTHER
#define OPT_NULL      SL_NULL

/* An option consists of a \em{name,} a \em{type} enum, a \em{ptr} to
   either the location of the storage or to a special user supplied
   function that partially parses the line for that one type, and a
   \em{help} string that briefly describes the usage of the
   option. All SL_TYPEs are valid values for the \em{type} field;
   however, for backward compatibility there are defines which equate
   all of the \bf{SL_*} enums to an equivalent \bf{OPT_*} enum.

   Under normal use, one should define a static array of OPTIONs that
   define all of the possible command line options.  This array must
   have a final entry of type \bf{SL_NULL} so that the end of the
   array can be detected.  After defining the OPTION array, you
   should make a single call to the \bf{get_options()} function.
   For each command line option, \bf{get_options()} will compare the
   option to each \em{name} field in the array of OPTIONs passed to
   it. If there is a match, then \bf{get_options()} will do the
   appropriate thing based on the \em{type} of the matched entry.  If
   \em{type} is \bf{SL_SWITCH}, then \em{ptr} is interpreted as a
   pointer to an integer and its value is logically negated.  If
   \em{type} is \bf{SL_STRING} or any of the other scalar types, then
   \em{ptr} is interpreted as a pointer to the appropriate type and
   its value will be set according to the next command line word.
   Option entries with a \em{type} of \bf{SL_OTHER} are special cases
   and are handled as described in the documentation for
   \bf{get_options().} */

typedef struct OPTION {
  char *name;
  SL_TYPE type;
  void *ptr;
  char *help;
} OPTION;

typedef struct OPTION_SET_MEMBER {
  char *name;
  void *value;
} OPTION_SET_MEMBER;

typedef struct OPTION_SET {
  void *dest;
  OPTION_SET_MEMBER *members;
} OPTION_SET;


/* A function to automagically parse command line arguments.  The
   first two arguments passed should be the identically named
   parameters in the \bf{main()} functions of your program.  The
   \em{options} parameter is the address of the first element in an
   OPTION array that you must define before the call (see the
   discussion above for the OPTION documentation.)  The \em{help}
   parameter should be a brief description of the program (but can be
   NULL).  This string, along with a table that summarizes all of the
   program options is printed if the \bf{-help} switch is given to the
   program or in the event that \bf{get_options()} is unable to parse
   the command line options.  The \em{obj} parameter allows the
   programer to pass additional information to a function that is used
   for an \bf{OPT_OTHER} option (see below for more information on
   this.)  Finally, the \em{rest} parameter determines how the
   \bf{get_options()} function behaves when it encounters a command
   line argument that it does not recognize.  If \em{rest} is zero,
   then the function will consider any unrecognized command line
   option to be an error, and will print usage information on stderr
   before exiting the program.  If \em{rest} is nonzero, then the
   function will return with the number of correctly parsed words in
   the command line when an unknown option is encountered.  At that
   point, it is up to the caller to correctly continue parsing the
   command line.

   Regarding user defined functions for \bf{OPT_OTHER} types, your
   function should look similar the following example:
   \begin{verbatim}
   \bf
     int user_parse(char **argv, int argc, OPTION *options,
                    int *cargc, int opti, void *obj)
     {
       int i, num_args_used;
       
       /\* No more args left so return an error. *\/
       if(argc == *cargc)
         return(1);
       
       /\* Process any args that need to be processed. *\/
       for(i = *cargc + 1; i < argc; i++) {
         /\* Process argv[i] *\/
       }
       
       /\* Set to the next thing to be parsed. *\/;
       *carg = *carg + num_args_used;
       
       /\* Return result to indicate no errors. *\/
       return(0);
     }
   \rm
   \end{verbatim} 
   The parameters \em{argv} and \em{argc} are identical to what was
   passsed to the \bf{main()} function.  The \em{options} parameter
   and \em{obj} parameter are given the same values as those passed to
   \bf{get_options().}  The value of \em{argv[*cargv]} is the exact
   command line word that triggered the user function to be called,
   and \em{options[opti]} is the specific entry that was a match to
   the current command line word.  You can do anything you want based
   on the remaining arguments, but you must set the value pointed to
   by \em{cargc} to be equal to the total number of options processed
   so that the calling routine knows where to continue processing.
   Zero should be the return value if nothing goes wrong; non-zero,
   otherwise. */

int get_options(int argc, char **argv, OPTION *options, /*\*/
		char *help, void *obj, int rest);



/* A function that will pretty-print a usage note, a help string and
   an option summary to stderr. */

void display_options(char *progname, OPTION *options, char *help);

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* Output all of the version information from NODElib's version.h file. */

void nodelib_version(void);


/* Uniformally computes a random number between \em{low} and
   \em{high} . */

double random_range(double low, double high);


/* Computes a Gaussian random number with zero mean and unit variance. */

double random_gauss(void);


/* Returns the the longest flush-right substring of \em{str} that does
   not contain a '/' character. */

char *simple_basename(char *str);


/* Fills the first \em{n} elements of \em{indices} with the numbers
   from 0 to (\em{n} - 1) in random order. */

void shuffle_indices(int *indices, int n);


/* Fills the first \em{n} elements of \em{indices} with the numbers
   from 0 to (\em{n} - 1) in random order. */

void shuffle_unsigned_indices(unsigned *indices, unsigned n);

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifdef __cplusplus
}
#endif /* __cplusplus */

/* h2man:skipbeg */

/* Get a reasonable pseudo random number generator. */

#ifdef __sun__

#include <limits.h>

#ifdef SUNOS
long lrand48();
void srand48(long);
#endif
#define random lrand48
#define RANDOM_MAX (LONG_MAX)
#define srandom srand48

#else /* ! __sun__ */

#ifdef WIN32                                            /* modified WEB */

#define random rand
#define RANDOM_MAX (RAND_MAX)           
#define srandom srand

/* Get M_PI in case somebody (don't you hate Microsoft?) didn't define it */
/* modified WEB */
 
#ifndef M_PI
#define M_PI 3.141592653589793238
#endif

#else /* ! WIN32 */

#include <limits.h>
#define RANDOM_MAX (INT_MAX)

#endif /* WIN32 */
#endif /* __sun__ */

/* h2man:skipend */


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#endif /* __MISC_H__ */
