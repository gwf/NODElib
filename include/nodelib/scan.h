
/* Copyright (c) 1995 by G. W. Flake.
 *
 * NAME
 *   scan.h - a simple reentrant text scanner
 * SYNOPSIS
 *   These routines only distinguish delimiter characters from
 *   comment characters and white space characters.  In other words,
 *   these scanners are powerful enough to scan languages such as the
 *   Bourne-shell or lisp, but could not handle C language comments.
 *   However, we can scan character arrays and files identically.
 * DESCRIPTION
 *   The \bf{scan} package provides a simple method for scanning
 *   text from character strings and and file pointers.  The package
 *   distinquishes between four types of characters: white space,
 *   delimiters, comment characters, and everything else (the other type).
 *
 *   In a nutshell, the \bf{scan_get()} function will skip all white
 *   space and return the first sequence of consecutive characters that
 *   contain only "other" characters.  If a comment characters is
 *   encountered, then all characters are skipped until a newline is
 *   found. If a delimiter is found then only the delimiter is returned.
 *   Thus, \bf{scan} would be a decent scanner for either a lisp-like
 *   language or a bourne-shell-like language.
 * BUGS
 *   Note that for the return values of the functions \bf{scan_get()}
 *   and \bf{scan_peek()} the caller does not "own" the memory since
 *   the return values point to internal buffers.  Subsequent calls
 *   to these function will wipe out the old values.  If you need the
 *   results to have a longer life-span then you should copy them to
 *   memory that you allocated yourself.
 * AUTHOR
 *   Gary William Flake (\url{\bf{gary.flake@usa.net}}{mailto:gary.flake@usa.net}).
 * SEE ALSO
 *   \bf{regex}(2), \bf{fgets}(3), \bf{getc}(3).
 */

#ifndef __SCAN_H__
#define __SCAN_H__

#include <stdio.h>
#include "nodelib/array.h"
#include "nodelib/etc/version.h"
#include "nodelib/etc/options.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* The user can set the first three fields to meaningful value.
   Every other field should remain hidden. */

typedef struct SCAN {
  char  *delims, *whites, *comments;
  char  *buffer, *ptr;
  ARRAY *token;
  FILE  *fp;
} SCAN;

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* Creates and returns a pointer to a SCAN structure.  If \em{isfile}
   is non-zero then \em{src} should be a valid FILE pointer to read
   text from.  If \em{isfile} is zero then \em{src} should be a
   NULL terminated character string that you wish to scan. */

SCAN *scan_create(int isfile, void *src);

/* This function is takes an already created SCAN structure pointer
   and reinitializes it to a new source.  This is usefull if you
   are scanning multiple strings.  The \em{delims}, \em{whites},
   and \em{comments} fields remain unchanged. */

SCAN *scan_recreate(SCAN *s, int isfile, void *src);

/* Returns a pointer to a string which contains the next "word" or
   delimiter.  If an EOF or end of string is encountered, then a
   NULL pointer is returned. */

char *scan_get(SCAN *s);

/* The function is the same as \bf{scan_get()} except that the internal
   pointers of \em{s} are maintained such that a subsequent call to
   \bf{scan_get()} or \bf{scan_peek()} will return the same "word". */

char *scan_peek(SCAN *s);

/* This function flushes the internal buffers until a newline is
   encountered, which is useful if your scanner detects a parse
   error. */

void  scan_flush(SCAN *s);

/* This last function will free up all associated memory of \em{s}.
   and return it to the system. */

void  scan_destroy(SCAN *s);

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __SCAN_H__ */
