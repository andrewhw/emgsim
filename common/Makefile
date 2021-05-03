##	------------------------------------------------------------
##	Makefile for common directory
##	------------------------------------------------------------
##	$Id: Makefile 107 2013-06-22 20:22:05Z andrew $

SRCHOME		=	.

INCLUDEDIR	=	$(SRCHOME)/include
LIBDIR		=	$(SRCHOME)/lib

RANLIB		=	ranlib
LINT		=	lint
MAKEFILE	= 	makefile
SHELL		= 	/bin/sh
DEFINES		=	$(RDEFINES) 

##			-DUSE_SYSLOG

CFLAGS		=	-Wall \
			$(CWARNFLAGS) $(DEFINES) -I$(INCLUDEDIR)


LIBNAME	=	$(LIBDIR)/libcommon.a

.SUFFIXES: .ln .o .c

.c.ln :
	$(LINT) -c -I$(INCLUDEDIR) $<

.c.o :
	$(CC) $(CFLAGS) -c $< -o $@

SUBDIRS		=  \
		alloc \
		basedir \
		commandline \
		commandpipe \
		dbg \
		error \
		file \
		gnuplot \
		intrretry \
		interpolate \
		io \
		log \
		os \
		math \
		path \
		string \
		timing \
		time


OBJS		= \
		alloc/arrayAllocator.o \
		alloc/isort.o \
		alloc/lsList.o \
		alloc/memlist.o \
		alloc/panic.o \
		alloc/tclCkalloc.o \
		\
		basedir/basedir.o \
		\
		commandline/commandline.o \
		\
		commandpipe/commandpipe.o \
		\
		dbg/debug.o \
		\
		error/Derror.o \
		error/strerror.o \
		\
		file/attvalue.o \
		file/tokenizer.o \
		file/cleanfile.o \
		file/copyfile.o \
		file/configuration.o \
		file/csvtools.o \
		file/plotBuffer.o \
		file/tempfile.o \
		\
		intrretry/i_close.o \
		intrretry/i_open.o \
		intrretry/i_rdwr.o \
		intrretry/i_stat.o \
		\
		interpolate/spline.o \
		\
		io/io_utils.o \
		\
		log/log.o \
		\
		os/getOsVersion.o \
		\
		path/convertPath.o \
		path/dirlist.o \
		path/fopenpath.o \
		path/openpath.o \
		path/pathtools.o \
		\
		gnuplot/simpleplots.o \
		gnuplot/tools.o \
		gnuplot/histogram.o \
		\
		math/acceleration.o \
		math/adjust.o \
		math/calcPeakToPeak.o \
		math/conv.o \
		math/fft.o \
		math/filtfilt.o \
		math/functions.o \
		math/chords.o \
		math/factorial.o \
		math/random.o \
		\
		string/niceDouble.o \
		string/niceFilename.o \
		string/strTimeDelta.o \
		string/strTimeString.o \
		string/strUnique.o \
		string/strbasename.o \
		string/strconcat.o \
		string/strcase.o \
		string/strisblank.o \
		string/strndup.o \
		string/strqconcat.o \
		string/strsplit.o \
		string/strtruncblank.o \
		string/strunctrl.o \
		string/formatParagraph.o \
		string/slnprintf.o \
		\
		timing/timer.o \
		\
		time/julian.o

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

