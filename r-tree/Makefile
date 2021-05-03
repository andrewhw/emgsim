##
## Makefile for R-Tree library
##
## $Id: Makefile 16 2021-05-03 17:41:43Z andrew $

INCLUDEDIR	=	./include
LIBDIR		=	./lib

CC		=	gcc
RANLIB		=	ranlib
LINT		=	lint
MAKEFILE	= 	makefile
SHELL		= 	/bin/sh
DEFINES		=	$(RDEFINES) 
CFLAGS		=	-g -pedantic -Wall \
			$(CWARNFLAGS) $(DEFINES) \
                        -I$(INCLUDEDIR) -I../common/include


LIBNAME	=	$(LIBDIR)/librtree.a
TESTEXE	=	testrtree

.SUFFIXES: .ln .o .c

.c.o :
	$(CC) $(CFLAGS) -c $< -o $@

SUBDIRS		=  \
		src

OBJS		= \
		src/card.o \
		src/index.o \
		src/node.o \
		src/rectangle.o \
		src/split_q.o

TESTOBJS        = \
		test/test.o

$(LIBNAME) : $(OBJS)
	- if [ ! -d $(LIBDIR) ] ; then mkdir $(LIBDIR) ; fi
	- rm -f $(LIBNAME)
	ar cr $(LIBNAME) $(OBJS)
	$(RANLIB) $(LIBNAME)

$(TESTEXE) test : $(LIBNAME) $(TESTOBJS)
	$(CC) $(CFLAGS) -o $(TESTEXE) $(TESTOBJS) $(LIBNAME) -lm

clean allclean: 
	- rm -f $(LIBNAME) *.o *core *.ln [Mm]akefile.bak
	@ for name in $(SUBDIRS) test; \
	do \
		echo "make clean in $$name" ; \
		(cd $$name ; rm -f *.o *core *.ln) ; \
	done
	rm -f $(TESTEXE)

all: $(LIBNAME) $(TESTEXE)

