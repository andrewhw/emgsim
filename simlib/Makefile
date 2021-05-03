##	------------------------------------------------------------
##	Makefile for common directory
##	------------------------------------------------------------
##	$Id: Makefile.Unix 1156 2004-06-25 17:50:35Z andrew $

INCLUDEFLAGS	=	-I./include \
			-I../common/include \
			-I../emg-tools/include \
			-I../DQEmgData/include \
			-I../r-tree/include \
			-I../glpk/include
LIBDIR		=	./lib

CC		=	gcc
CXX		=	g++
RANLIB		=	ranlib
LINT		=	lint
MAKEFILE	= 	makefile
SHELL		= 	/bin/sh
DEFINES		=	$(RDEFINES)

#			-DUSE_JITTER_DB

CFLAGS		=	-g -pedantic -Wall \
			$(CWARNFLAGS) $(DEFINES) $(INCLUDEFLAGS)


LIBNAME	=	$(LIBDIR)/libsimulator.a

.SUFFIXES: .ln .o .c .cpp

.c.ln :
	$(LINT) -c -I$(INCLUDEDIR) $<

.c.o :
	$(CC) $(CFLAGS) -c $< -o $@

.cpp.o :
	$(CXX) $(CFLAGS) -c $< -o $@

SUBDIRS		=  \
		src

OBJS		= \
		src/emgutil.o \
		src/fileutil.o \
		src/firing.o \
		src/globalHandler.o \
		src/globals.o \
		src/logwrite.o \
		src/make16bit.o \
		src/makeMUP.o \
		src/muscle.o \
		src/muscleNeuropathy.o \
		src/noiseFunction.o \
		src/statistics.o \
		src/userinput.o \
		src/JitterDB.o \
		src/MUP.o \
		src/MuscleData.o \
		src/NoiseGenerator.o \
		src/NeedleInfo.o \
		src/3Circle.o \
		\
		src/Simulator.o \
		src/SimulationResult.o \


$(LIBNAME) : $(OBJS)
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


##
## $Log$
## Revision 1.22  2004/06/25 17:50:35  andrew
## o Added Mahdieh's missing file to makefile
##
## Revision 1.21  2003/12/15 21:23:48  andrew
## o Removed LP basis for mean distribution generation
##
## Revision 1.20  2003/12/12 15:12:13  andrew
## o Moved LP problem into its own set of files
##
## Revision 1.19  2003/10/10 16:03:26  andrew
## o Working on integrating the LP for mean MU diameter calculation
##
## Revision 1.18  2003/05/09 18:00:01  andrew
## o Added turn-counting statistics
##
## Revision 1.17  2003/03/20 18:10:14  andrew
## o Nomenclature update -- MUAP -> MUP, MFAP -> MFP throughout
##
## Revision 1.16  2003/02/04 19:12:45  andrew
## o Beginning to add in Neuropathy
## o Broke out MU chooser into own function.
##
## Revision 1.15  2003/01/14 00:17:30  andrew
## o Update for filtering bug Sheng found
##
## Revision 1.14  2002/12/18 02:52:15  andrew
## o Rolled back to get multi-needle stuff working.
##
## Revision 1.12  2002/09/25 06:52:13  robert
## added sMUP_util.o SMUP.o objects, robert
##
## Revision 1.11  2002/09/20 03:53:44  robert
## added sinWave.o object, R.Tollola
##
## Revision 1.10  2002/07/20 21:24:08  andrew
## o First crack at working cannula calculation.
##
## Revision 1.9  2002/07/10 03:04:18  andrew
## o Began adding R-Tree code to simulator.
##
## Revision 1.8  2002/05/27 01:49:11  andrew
## o Re-wrote the interpolation stuff for MUP.
## o Alignment now (finally) seems to be working ok.
##
## Revision 1.7  2002/05/22 02:06:09  andrew
## o Expanding simulation result to incorporate more information
## o Added in list of active MUP
## o Beginning to add in MuscleData information
##
## Revision 1.6  2002/04/30 19:09:55  andrew
## o Began adding in a Simulation Result object, which is returned from
##   a simulator "run"
##
## Revision 1.5  2002/03/14 15:49:02  andrew
## o Adding in DQEMG data file generation.
##
## Revision 1.4  2002/03/06 02:50:33  andrew
## o Fixed up noise implementation so that it is controlled by the
##   user, and has sensible default values
##
## Revision 1.3  2002/03/06 00:43:58  andrew
## o Broke noise generation out into its own file
## o Fixed up missing zero in constant name
##
## Revision 1.2  2002/02/19 02:31:21  andrew
## o Working on cleaning up interface from main
## o Beginning to rectify globals
##
## Revision 1.1  2002/02/06 01:44:06  andrew
## o Initial checkin
##
## Revision 1.1  2002/01/27 22:58:22  andrew
## o Initial Check-in
## o This source forms the bulk of the simulator proper, and will
##   be called from the UI wrapper to perform the simulator tasks.
##   This part of the code can remain platform-independent, and will
##   not have any attachment to the UI except through a callback
##   based implementation.
##
##
