#########################################################################
#
# Copyright (c) 1995-2000 by G. W. Flake.
#
#########################################################################

.SUFFIXES: .h .3
.h.3:
	$(H2MAN) -name `basename $<` < $< > $@

include ../etc/Configure

VPATH = ../include/nodelib

HDRS  = $(wildcard ../include/nodelib/*.h)
MANS  = $(HDRS:../include/nodelib/%.h=%.3)

default: all
all: docs
libs:
progs:
depend:

$(MANS): ../etc/h2man.pl

docs: $(MANS)

clean distclean:
	rm -f index.db  *~

realclean:
	rm -f *.3 index.db

#########################################################################
