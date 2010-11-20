#########################################################################
#
# Copyright (c) 1995-2000 by G. W. Flake.
#
#########################################################################

include etc/VERSION
include etc/Configure

ROOT    = $(shell pwd)
BASE    = $(notdir $(ROOT))
DIRS    = etc src lib man html examples test

default:
	@$(SED) -n 's/\(..*\)/  \1/g;1,/^ *--$$/p' etc/INSTALL

all libs docs progs clean distclean realclean: FORCE
	@for dir in $(DIRS); \
	do \
	  echo ""; echo "##########################################"; \
	  echo "### Making $@ in directory $$dir."; echo "###"; \
	  cd $(PWD)/$$dir; echo cd $(PWD)/$$dir; \
	  if $(MAKE) $@; then echo ""; else break; fi; \
	done

tar:
	cd etc; $(MAKE) README
	$(PERL) etc/manifest.pl tar > etc/MANIFEST
	cd ..; \
	( ( find $(BASE) \! -name '*.[oa3]' -a \
	               \! -name '*.html'  -a \
	               \! -name '.*'  -a \
	               \! -name '#*' -a \
	               \! -name '*~' -a \
	               \! -perm 755 -a \
	               \! -type d -print; \
	    find $(BASE)/lib -name libnle.a -print; \
	    find $(BASE) -type l -print; \
	   ) | egrep -v '(lib/libnle.a|lib/nle/libnle.a|/tmp)'; \
           echo $(BASE)/configure ) | \
	$(TAR) -T - -czf $(BASE)-$(VERSION).tgz

# bigtar: docs
# 	cd etc; $(MAKE) README FAQ
# 	cd ..; \
# 	( find $(BASE) \! -name '*.[oa]' -a \
# 	               \! -name '#*' -a \
# 	               \! -name '*~' -a \
# 	               \! -perm 755 -a \
# 	               \! -type d -print; \
#           echo $(BASE)/configure ) | \
# 	$(TAR) -T - -czf $(BASE)-$(VERSION)-big.tgz

zip: docs
	cd etc; $(MAKE) README FAQ
	#
	rm include/nodelib/etc
	mkdir include/nodelib/etc
	cp etc/version.h include/nodelib/etc
	cp etc/options.h include/nodelib/etc
	#
	cd ..; \
	( find $(BASE) \! -name '*.[oa]' -a \
	               \! -name '.*'  -a \
	               \! -name '#*' -a \
	               \! -name '*~' -a \
	               \! -perm +1 -a \
	               \! -type l -print; \
          echo $(BASE)/html/index.html \
        ) | \
	zip $(BASE)-$(VERSION).zip -@
	rm include/nodelib/etc/*.h
	rmdir include/nodelib/etc
	cd include/nodelib; ln -s etc ../../etc

dist:
	cd etc; $(MAKE) version.h
	$(PERL) etc/manifest.pl tar > etc/MANIFEST
	$(MAKE) tar
#	$(PERL) etc/manifest.pl > etc/MANIFEST
#	$(MAKE) bigtar
#	$(MAKE) zip

FORCE:

#########################################################################
