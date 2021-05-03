##	------------------------------------------------------------
##	$Id: Makefile 66 2021-05-03 17:41:35Z andrew $

INCLUDEFLAGS	=	-I./include \
			-I../common/include
LIBDIR		=	./lib

RANLIB		=	ranlib
LINT		=	lint
MAKEFILE	= 	makefile
SHELL		= 	/bin/sh
DEFINES		=	$(RDEFINES) 


CXXFLAGS	=	-Wall \
			$(CWARNFLAGS) $(DEFINES) $(INCLUDEFLAGS)


LIBNAME	=	$(LIBDIR)/libc++tools.a

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
		src/tlRef.o \
		src/tlRefManager.o \
		\
		src/tlStringAllocationTool.o \
		\
		src/tlDataChooser.o\
		src/tlHashTable.o\
		src/tlTrie.o\
		src/tlBin.o\
		src/tlTuple.o\
		src/tlMatrix.o\
		src/tlSrMatrix.o \
		src/tlGslMatrix.o\
		src/tlSrGslMatrix.o \
		src/tlVector.o\
		src/tlColumn.o\
		\
		src/tlBinTable.o\
		src/tlTable.o\
		\
		src/tlSTable.o\
		src/tlSParser.o \
		src/tlExpression.o \
		\
		src/tlErrorManager.o \
		src/tlSrValue.o\
		src/tlSrString.o \
		\
		src/tlUnicodeConvert.o

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


