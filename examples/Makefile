#########################################################################
#
# Copyright (c) 1995-2000 by G. W. Flake.
#
#########################################################################

include ../etc/Configure

SRCS  = $(wildcard *.c)
PROGS = $(SRCS:%.c=%)

SPORE = aa.train.out aa.test.out

default: all
all: progs
progs: $(PROGS)
libs:
depend:
docs:

$(PROGS): % : %.o ../lib/libnode.a
	$(CC) -o $@ $@.o $(LDFLAGS) $(LIBS)

distclean realclean clean:
	rm -f $(PROGS) *.o *~ $(SPORE)

#########################################################################
