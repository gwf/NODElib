#########################################################################
#
# Copyright (c) 1995-2000 by G. W. Flake.
#
#########################################################################

.SUFFIXES: .h .html
.h.html:
	$(H2HTML) -name `basename $<` < $< > $@

include ../etc/Configure

VPATH = ../include/nodelib

HDRS     = $(wildcard ../include/nodelib/*.h)
HTML     = $(HDRS:../include/nodelib/%.h=%.html)
ALLHTML  = $(HTML) index.html

default: all
all: docs
libs:
progs:
clean distclean:
depend:

$(HTML): ../etc/h2html.pl .headerschanged

docs: .headerschanged $(ALLHTML) .links

.headerschanged: $(HDRS)
	touch .headerschanged

.links: .headerschanged
	touch .links
	$(PERL) ../etc/htmlcrossref.pl $(HTML)
	$(PERL) ../etc/htmlindex.pl $(HTML)

index.html: nodelib.html
	rm -f index.html; cp nodelib.html index.html
#	sh ../etc/mkindex.sh > index.html

faq.html:
	ln -s ../etc/faq.htm faq.html

realclean:
	rm -f *.html .headerschanged .links *~

#########################################################################
