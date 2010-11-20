
/* Compile time options for NODElib */


#ifndef __NODELIB_OPTIONS_H__
#define __NODELIB_OPTIONS_H__

/* Set to use the NODElib extension library. */

#define NL_EXTENSIONS 1


/* Attempt to inline tiny functions. */
/* #define INLINE */
#define INLINE inline


/* Use these defines to work around the namespace pollution in libmp
   and CMU Common Lisp. */

#define xmalloc _nodelib_xmalloc
#define xfree   _nodelib_xfree

#undef size_t

/* Work around the broken include files on SunOS. */

#ifdef SUNOS
#define memmove(p1, p2, p3)   bcopy(p2, p1, p3)

#include <sys/stdtypes.h>
#include <stdio.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int fflush(FILE *);
int fprintf(FILE *, char *, ...);
int fputc(char, FILE *);
int fread(void *, int, int, FILE *);
int fscanf(FILE *, char *, ...);
int fseek(FILE *, long, int);
int fwrite(void *, int, int, FILE *);
int printf(char *, ...);
int sscanf(char *, char *, ...);
void bcopy(void *, void *, int);
void fclose(FILE *);
void fputs(char *, FILE *);
void rewind(FILE *);
void perror(char *s);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif


#endif /* __NODELIB_OPTIONS_H__ */
