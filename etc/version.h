
#ifndef __NODELIB_VERSION_H__
#define __NODELIB_VERSION_H__

#define NODELIB_USER      "flakeg"
#define NODELIB_HOST      "flakeg-mac.corp.yahoo.com"
#define NODELIB_UNAME     "Darwin flakeg-mac.corp.yahoo.com 7.5.0 Darwin Kernel Version 7.5.0: Thu Aug  5 19:26:16 PDT 2004; root:xnu xnu-517.7.21.obj~3 RELEASE_PPC  Power Macintosh powerpc"
#define NODELIB_DATE      "Thu Nov 18 14:22:50 PST 2004"
#define NODELIB_COMPILER  "gcc version 3.3 20030304 (Apple Computer, Inc. build 1495)"
#define NODELIB_VERSION   "1.31.1"
#define NODELIB_COPYRIGHT "Copyright (c) 1995-2004 by G. W. Flake."

static const char *nodelib_info[] = {
  NODELIB_USER,
  NODELIB_HOST,
  NODELIB_UNAME,
  NODELIB_COMPILER
  NODELIB_DATE,
  NODELIB_VERSION,
  NODELIB_COPYRIGHT
};


/* Silly little hack so that gcc doesn't complain about nodelib_info
   being unused. */

static void *nodelib_info_kluge[] = { (void *) & nodelib_info_kluge, 
                                      (void *) & nodelib_info };

#endif /* __NODELIB_VERSION_H__ */

