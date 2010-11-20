
/* Copyright (c) 1995 by G. W. Flake.
 *
 * NAME
 *   ulog.h - standardized I/O routines with many features
 * SYNOPSIS
 *   Because of this module, there is not a single plain printf()
 *   in all of NODElib.  All screen I/O is passed through the ULOG
 *   package, which means that you can arbitrarily set how verbose
 *   the messages are and where they should go (i.e., log files,
 *   error message window, etc.).
 * DESCRIPTION
 *   The user log system (\bf{ulog}) provides a method for
 *   controlling all tty type output in a user program.  The philosophy
 *   behind this package is that modular routines should never directly
 *   call \bf{printf}(3), but should instead direct output to
 *   \bf{ulog}.  When appropriately used, it becomes possible to
 *   selectively redirect output messages based on type.  This feature is
 *   tremendously useful when existing code is used as a back-end to a GUI,
 *   for example. Thus, instead of rewriting \bf{printf()} statements
 *   so that output is sent to a window, one only needs to select a new
 *   output hook.
 *   
 *   There are seven different types of messages distinguished by
 *   \bf{ulog}. Normally you will only use the \bf{ulog()} function
 *   which is used in a manner similar to \bf{printf()}, except that
 *   a message type must be specified.  The message types are described
 *   below.  Since each message type can be sent to a different 
 *   destination, the default destination is also listed, along with
 *   any other peculiarities of the message type.
 *
 *   \begin{itemize}
 *   \item \bf{ULOG_DEBUG}
 *           Messages that contain information normally only of use
 *           while debugging.  Default destination is to stderr.  All
 *           message are preceded by the string "(FILE, LINE): "
 *           to indicate where the call was made.  Any instance of
 *           "%m" in the format string is replaced with the string
 *           which corresponds to \em{errno}.
 *
 *   \item \bf{ULOG_INFO}
 *           This message type is provided just in case none of the
 *           other types fit your needs.  It behaves just like ULOG_PRINT
 *           but defaults to standard error instead.  Use this for
 *           debugging and error messages when it doesn't make sense
 *           for a (FILE, LINE) prefix to be printed.
 *
 *   \item \bf{ULOG_PRINT}
 *           Messages that you would normally consider
 *           the standard output of your program.  Default destination
 *           is to stdout.  Only the standard \bf{printf()} type
 *           formats are supported.
 *
 *   \item \bf{ULOG_WARN}
 *           Warning messages denote a fully recoverable condition.
 *           Warning conditions usually denote a problem with the user's
 *           input, or a problem with the environment.  For example,
 *           failure to open a file with a supplied name would be a
 *           typical warning condition. Default destination is to stderr.
 *           Any instance of "%m" in the format string is replaced with
 *           the string which corresponds to \em{errno}.
 *
 *   \item \bf{ULOG_ERROR}
 *           Error messages denote serious problems that may be
 *           recoverable.  Error conditions are usually indicative
 *           of a programmer's error.  For example, an attempt to pop
 *           an item off of an empty stack would be an error. Default
 *           destination is to stderr.  All message are preceded by
 *           the string "(FILE, LINE): " to indicate where the call
 *           was made.  Any instance of "%m" in the format string is
 *           replaced with the string which corresponds to \bf{errno}.
 *
 *   \item \bf{ULOG_FATAL}
 *           Fatal errors are similar to normal errors except that they
 *           are considered unrecoverable.  An example would be a NULL
 *           being returned from \bf{malloc()} or a failed assertion.
 *           The result of a ULOG_FATAL message is identical to a
 *           ULOG_ERROR message except that each shutdown hook is called
 *           prior to \bf{exit(1)}. See below for more information
 *           on shutdown hooks.
 *
 *   \item \bf{ULOG_ABORT}
 *           An abort message is identical to a fatal message except that
 *           the shutdown hooks are not called and \bf{abort()} is
 *           called.  This allows you to dump core at a time of your
 *           choosing.
 *   \end{itemize}
 *
 *   Since each use of ULOG_DEBUG, ULOG_ERROR, ULOG_FATAL, and ULOG_ABORT
 *   prints out the prefix "(FILE, LINE): " any instance of "%t" in the
 *   format string will print a newline followed spaces, followed by 
 *   " : " such that the the ":" characters of the two lines will match up.
 *   For example,
 *   \begin{verbatim}
 *     ulog(ULOG_ERROR, "func: failed to write \\\\"%s\\\\".%t%m.",
 *          fname);
 *   \end{verbatim}
 *   will print:
 *   \begin{verbatim}
 *     (foo.c, 128): func: failed to write "afile".
 *                 : No space left on device.
 *   \end{verbatim}
 *   Notice also that newlines are appended to any line without one.
 * BUGS
 *   Right now \bf{ulog} has a fixed size output buffer determined by
 *   ULOG_BUFF_SZ.  There is no easy way around this since \bf{ulog} uses
 *   \bf{vsprintf()} to do the bulk of the string processing.
 * AUTHOR
 *   Gary William Flake (\url{\bf{gary.flake@usa.net}}{mailto:gary.flake@usa.net}).
 * SEE ALSO
 *   \bf{syslog}(3), \bf{printf}(3), \bf{errno}(2),
 *   \bf{perror}(3)
 */

#ifndef __ULOG_H__
#define __ULOG_H__


#include <stdio.h>
#include <stdarg.h>
#include "nodelib/etc/version.h"
#include "nodelib/etc/options.h"

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* Different type of messages. */

#define ULOG_DEBUG     0
#define ULOG_INFO      1
#define ULOG_PRINT     2
#define ULOG_WARN      3
#define ULOG_ERROR     4
#define ULOG_FATAL     5
#define ULOG_ABORT     6
#define ULOG_MSG_TYPES 7

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if 0

/* THIS IS A BOGUS FUNCTION DEFINITION WHICH IS USED ONLY TO FOOL H2MAN. The
   real ulog() can only be defined as a macro.  However, since it it's name
   evaluates to a function pointer I believe that it is safe to propagate the 
   myth that it is a normal function. */


/* The \bf{ulog()} function is not really function, but a special type of
   macro that evaluates to a function.  This distinction is probably not
   needed for most users, however we give the warning just in case.  Given a
   message \em{type} as described in the beginning of this section
   \bf{ulog()} prints the appropriate message as determined by the format
   string \em{fmt} and the rest of the arguments. */

void ulog(int type, char *fmt, ...);

#endif				/* 0 */


/* If \em{fp} is non-NULL then all messages of type \em{type} will be written 
   to the file pointed to by \em{fp}. */

void ulog_redirect_fp(int type, FILE * fp);

/* If \em{func} is non-NULL then all messages of type \em{type} will be
   passed to the function pointed to by \em{func}. */

void ulog_redirect_hook(int type, void (*func) (char *msg));

/* This function adds \em{func} to the central list of shutdown hooks, which
   are called in the event of a ULOG_FATAL message being issued. You can use
   this to gracefully exit the program (write out buffers, free memory on DOS 
   machines, etc.). */

void ulog_add_shutdown_hook(void (*func) (void));

/* This function will remove \em{func} from the central list of shutdown
   hooks. */

void ulog_remove_shutdown_hook(void (*func) (void));

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* h2man:ignore */
#define ulog (__ulog_store_generic(__LINE__, __FILE__, -1))

#define udebug (__ulog_store_alias(__LINE__, __FILE__, ULOG_DEBUG))
#define uinfo  (__ulog_store_alias(__LINE__, __FILE__, ULOG_INFO))
#define uprint (__ulog_store_alias(__LINE__, __FILE__, ULOG_PRINT))
#define uwarn  (__ulog_store_alias(__LINE__, __FILE__, ULOG_WARN))
#define uerror (__ulog_store_alias(__LINE__, __FILE__, ULOG_ERROR))
#define ufatal (__ulog_store_alias(__LINE__, __FILE__, ULOG_FATAL))
#define uabort (__ulog_store_alias(__LINE__, __FILE__, ULOG_ABORT))


/* h2man:ignore */
void (*__ulog_store_generic(unsigned, char *, int)) (int, char *, ...);
void (*__ulog_store_alias(unsigned, char *, int)) (char *, ...);

#ifdef PTHREADS
void ulog_mutex_init();
#endif 


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* h2man:include  There are two global variables that you can use to tune the 
   behavior of \bf{ulog}:

   \begin{itemize}

   \item \bf{int} \em{ulog_threshold} ; The order of the message types listed 
   in the beginning of this section is in increasing order of severity.  Only 
   messages which have a severity greater than \em{ulog_threshold} will be
   printed.  By default it is set to 0 or ULOG_PRINT.

   \item \bf{int} \em{ulog_never_dump} ; If set to non-zero, then ULOG_ABORT
   messages will behave as normal but call \bf{exit(2)} instead of
   \bf{abort()}.

   \item \bf{int} \em{ulog_no_prefix} ; If set to non-zero, then messages of
   type ULOG_DEBUG, ULOG_ERROR, ULOG_FATAL, and ULOG_ABORT will not print the 
   (FILE, LINE) prefix string before a message.

   \end{itemize} */

/* h2man:skipbeg */
#ifdef OWNER
#define ISOWNER(x) x
#define NOTOWNER(x)
#else
#define ISOWNER(x)
#define NOTOWNER(x) x
#endif

NOTOWNER(extern)
     int ulog_threshold ISOWNER( = ULOG_PRINT);
NOTOWNER(extern)
     int ulog_never_dump ISOWNER( = 0);
NOTOWNER(extern)
     int ulog_no_prefix ISOWNER( = 0);

#undef OWNER
#undef ISOWNER
#undef NOTOWNER
/* h2man:skipend */

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#endif	/* __ULOG_H__ */
