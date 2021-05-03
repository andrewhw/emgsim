## $Id: Makefile 178 2021-05-03 17:41:34Z andrew $

INCLUDEFLAGS	=	-I./include
LIBDIR		=	./lib

RANLIB		=	ranlib
LINT		=	lint
MAKEFILE	= 	makefile
SHELL		= 	/bin/sh
DEFINES		=	$(RDEFINES) 

CFLAGS		=	-g -pedantic -Wall \
			$(CWARNFLAGS) $(DEFINES) $(INCLUDEFLAGS)
CXXFLAGS	= $(CFLAGS)


LIBNAME	=	$(LIBDIR)/libDQEmgData.a

.SUFFIXES: .ln .o .c .cpp

.c.ln :
	$(LINT) -c -I$(INCLUDEDIR) $<

.c.o :
	$(CC) $(CFLAGS) -c $< -o $@

.cpp.o :
	$(CXX) $(CXXFLAGS) -c $< -o $@

SUBDIRS		=  \
		src

OBJS		= \
		src/DQEmgData.o \
		src/dqemgjulian.o

$(LIBNAME) : $(OBJS)
	- if [ ! -d $(LIBDIR) ] ; then mkdir $(LIBDIR) ; fi
	- rm -f $(LIBNAME)
	ar cr $(LIBNAME) $(OBJS)
	$(RANLIB) $(LIBNAME)

clean : 
	- rm -f $(LIBNAME) *.o *core *.ln [Mm]akefile.bak
	@ for name in $(SUBDIRS); \
	do \
		echo "make clean in $$name" ; \
		(cd $$name ; rm -f *.o *core *.ln) ; \
	done

