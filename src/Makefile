#########################################################################
#
# Copyright (c) 1995-2000 by G. W. Flake.
#
#########################################################################

include ../etc/Configure

VPATH = ../include

SRCS  = $(wildcard *.c)
OBJS  = $(SRCS:%.c=%.o)

default: all
all: libs depend
libs: depend
progs:
docs:

clean:
	rm -f *.o *~

distclean realclean:
	rm -f *.o .depend *~

depend: .depend
.depend:
	$(DEPEND) $(CPPFLAGS) $(SRCS) | \
	  sed 's/nodelib\(\/nodelib\)*/nodelib/g' | \
	  sed 's/\.\.\/include\/nodelib\/etc\/version.h//g' | \
	  sed '/^  \\$$/d' > .depend;

ifeq (.depend,$(wildcard .depend))
include ./.depend
endif

#########################################################################
