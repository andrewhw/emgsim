##	------------------------------------------------------------
##	Makefile for emg-tools directory
##	------------------------------------------------------------
##	$Id: Makefile 49 2013-07-17 15:08:53Z andrew $

INCLUDEFLAGS	=	-I./include -I../common/include
LIBDIR		=	./lib

RANLIB		=	ranlib
LINT		=	lint
MAKEFILE	= 	makefile
SHELL		= 	/bin/sh
DEFINES		=	$(RDEFINES) 


CFLAGS		=	-g -pedantic -Wall \
			$(INCLUDEFLAGS) $(DEFINES)

CXXFLAGS	=	$(CFLAGS)


LIBNAME	=	$(LIBDIR)/libemg.a

.SUFFIXES: .o .c .cpp

.c.o :
	$(CC) $(CFLAGS) -c $< -o $@

.cpp.o	:
	$(CXX) $(CXXFLAGS) -c $< -o $@

SUBDIRS		=  \
		src

OBJS		= \
		src/buffertools.o \
		src/dco_utils.o \
		src/emg.o \
		src/emgdat.o \
		src/prm.o

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

installclean : clean

pure : 
	@echo "Nothing to do for pure"

rebuild: allclean alldepend all allinstall

all:
	(cd .. ; make );

allclean:
	(cd .. ; make clean);

alldepend:
	(cd .. ; make depend);

allpure:
	(cd .. ; make pure);

allinstall:
	(cd .. ; make install);

netinstall:
	(cd .. ; make netinstall);

ci:
	(cd .. ; make ci);

