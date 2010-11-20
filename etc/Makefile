#########################################################################
#
# Copyright (c) 1995-2000 by G. W. Flake.
#
#########################################################################

include Configure

default: all
all: libs docs
libs: version.h
depend:
docs:
progs:

version.h: version.sh version.in VERSION
	sh version.sh < version.in > version.h

README: $(wildcard ../include/*.h)
	( cd ../html; make index.html )
	lynx -nolist -dump ../html/index.html > README
#	sh mkreadme.sh > README

FAQ: html2txt.pl faq.htm
	perl html2txt.pl faq.htm > FAQ

MANIFEST: FORCE
	cd .. ; $(PERL) etc/manifest.pl > etc/MANIFEST

clean distclean:
	rm -f man2html *.tgz *.zip

realclean:
	rm -f man2html *.tgz *.zip MANIFEST README version.h

FORCE:

#########################################################################

