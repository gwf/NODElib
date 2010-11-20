#########################################################################
#
# Copyright (c) 1995-2000 by G. W. Flake.
#
#########################################################################


RANLIB  = ranlib
AR      = ar
LDFLAGS = -L.
MAKE    = make
CC      = gcc
CFLAGS  = -O4 -funroll-loops -fomit-frame-pointer 

SRCS  = $(wildcard *.c)
OBJS  = $(SRCS:%.c=%.o)

default: libnle.a

libnle.a: $(OBJS)
	$(AR) cr $@ $(OBJS)
	$(RANLIB) $@

clean:
	rm -f *.o *~

realclean:
	rm -f *.o  *~ libnle.a

tar:
	$(MAKE) clean
	cd ..; tar cvvfz nle.tgz nle

#########################################################################
