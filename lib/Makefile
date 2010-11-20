#########################################################################
#
# Copyright (c) 1995-2000 by G. W. Flake.
#
#########################################################################

include ../etc/Configure

SRCS  = $(wildcard ../src/*.c)
OBJS  = $(SRCS:%.c=%.o)

default: all
all: libs
libs: libnode.a libnle.a
progs:
depend:
clean:
	cd nle; make clean
docs:

libnode.a: $(OBJS)
	$(AR) cr $@ $(OBJS)
	$(RANLIB) $@

libnle.a:
	rm -f libnle.a
	uname=`uname`; \
	if [ -d $$uname -a -e $$uname/libnle.a ]; then \
	  ln -s $$uname/libnle.a; \
	else \
	  echo ""; \
	  echo "Binary libnle.a not found.  Building from scratch...."; \
	  echo ""; \
	  (cd nle; make); \
	  if [ \! -d $$uname ]; then \
	    mkdir $$uname; \
	  fi; \
	  cp nle/libnle.a $$uname; \
	  ln -s $$uname/libnle.a; \
	fi;

distclean realclean:
	rm -f libnode.a libnle.a *.o *~
	cd nle; make realclean

#########################################################################
