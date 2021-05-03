##
## $Id: Makefile 875 2007-07-18 16:46:57Z andrew $
##


#CC			=	gcc47
#CXX			=	g++47

MAKE			=	make
SHELL			=	/bin/sh

EXENAME			=	simtext

RDEFINES		=	-g -DDEBUG \
				-DDQD_DEBUG_DUMP \
				-DUSE_NUMERICAL_RECIPES_RANDOM \
				-DTCL_MEM_DEBUG -DMEM_DEPRECATION_OK

DEFINES			=	$(RDEFINES)

#				-DEXTENDED_DIRECTORY_NAMES


INCLUDEFLAGS		=	-I. -Icommon/include \
				-Iemg-tools/include \
				-Isimlib/include \
				-IDQEmgData/include \
				-Ir-tree/include

CFLAGS			=	-g $(DEFINES) $(INCLUDEFLAGS) -Wall

#				-Wall -Wshadow -Wconversion
#				-Wmissing-prototypes

LDFLAGS			=	-Lsimlib/lib \
				-Lcommon/lib \
				-Lemg-tools/lib \
				-LDQEmgData/lib \
				-Lr-tree/lib

LDLIBS			=	-lsimulator -lemg \
				-lDQEmgData \
				-lrtree \
				-lcommon \
				-lm 

OBJS			= \
			main.o \
			options.o

all	: $(EXENAME)


.SUFFIXES: .o .c .cpp

.c.o	:
	$(CC) $(CFLAGS) -c $*.c -o $*.o

.cpp.o	:
	$(CXX) $(CFLAGS) -c $*.cpp -o $*.o


##
##	Targets begin here
##

$(EXENAME) : $(OBJS) lib-common lib-emg lib-simulator lib-DQEmgData lib-r-tree
	$(CXX) $(LDFLAGS) $(CFLAGS) -o $(EXENAME) $(OBJS) $(LDLIBS)

lib-common :
	( \
		cd common ; \
		make \
			CC="$(CC)" CXX="$(CXX)" \
			RDEFINES="$(RDEFINES)" \
	)

lib-emg :
	( \
		cd emg-tools ; \
		make \
			CC="$(CC)" CXX="$(CXX)" \
			RDEFINES="$(RDEFINES)" \
	)

lib-simulator :
	( \
		cd simlib ; \
		make \
			CC="$(CC)" CXX="$(CXX)" \
			RDEFINES="$(RDEFINES)" \
	)

lib-DQEmgData :
	( \
		cd DQEmgData ; \
		make \
			CC="$(CC)" CXX="$(CXX)" \
			RDEFINES="$(RDEFINES)" \
	)

lib-r-tree :
	( \
		cd r-tree ; \
		make \
			CC="$(CC)" CXX="$(CXX)" \
			RDEFINES="$(RDEFINES)" \
	)

clean : plotclean
	- rm -f $(OBJS) $(EXENAME)
	- rm -f *.o */*.o core core.*

plotclean :
	- rm -f plots/*.gif plots/*.eps plots/*.png

allclean : clean
	- (cd common ; make clean )
	- (cd emg-tools ; make clean )
	- (cd simlib ; make clean )
	- (cd DQEmgData ; make clean )
	- (cd r-tree ; make clean )

tags ctags : dummy
	- ctags *.cpp common/*/*.c */src/*.cpp

plots : dummy
	- ( cd plots ; createplot ; createmasterplot )


archive zip: dummy
	- ( \
		name=`date +"simulator-src-%B%d,%Y"`; \
		svn export svn://svn.qemg.org/simtext/trunk $$name; \
		( \
			cd $$name ; \
			rm *.ps; \
			rm *.bat;\
			rm -rf run*;\
			rm -rf plot*;\
			rm -rf group*;\
			rm -rf combine*;\
		); \
		zip -r $$name.zip $$name; \
	  )

## this target is here for 'always do' rules to depend on

dummy :


