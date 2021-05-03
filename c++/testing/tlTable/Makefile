##
## $Id: Makefile 55 2013-07-17 15:01:29Z andrew $
##


MAKE			=	make
SHELL			=	/bin/sh

EXENAME			=	testcase

RDEFINES		=	-DDEBUG \
					-DUSE_NUMERICAL_RECIPES_RANDOM \
					-DTCL_MEM_DEBUG -DMEM_DEPRECATION_OK \
					-DGSL_LIBRARY_INSTALLED \
					-I/usr/local/include

DEFINES			=	$(RDEFINES)

#				-DEXTENDED_DIRECTORY_NAMES


INCLUDEFLAGS		=	-I. -I../utils -I../../include \
					-I../../../common/include

CXXFLAGS		=	-g $(DEFINES) $(INCLUDEFLAGS) -pedantic -Wall

LDFLAGS			=	-L../../lib -L../../../common/lib \
					-L/usr/local/lib -L/sw/lib

LDLIBS			=	-lc++tools -lcommon -lgsl -lgslcblas -lm

OBJS			= \
			tools.o \
			../utils/testutils.o \
			\
			testDiscretizeEqual.o \
			testDiscretizeEqual_smallXOR.o \
			testDiscretizeEqual_uniformDistrib.o \
			testDiscretizeEqual_expDistrib.o \
			testDiscretizeEqual_intXOR.o \
			\
			testDiscretizeMMEGroupUnique_uniformDistrib.o \
			testDiscretizeMMEGroupUnique_expDistrib.o \
			testDiscretizeMMEGroupUnique_1_to_16.o \
			testDiscretizeMMEGroupUnique_Float.o \
			testDiscretizeMMEGroupUnique_XOR.o \
			testDiscretizeMMEGroupUnique_floatRepeats.o \
			testDiscretizeMMEGroupUnique_intRepeats.o \
			testDiscretizeMMEGroupUnique_intXOR.o \
			testDiscretizeMMEGroupUnique_floatXORDatafile.o \
			\
			testDiscretizeMMEIgnoreUnique_uniformDistrib.o \
			testDiscretizeMMEIgnoreUnique_expDistrib.o \
			testDiscretizeMMEIgnoreUnique_1_to_16.o \
			testDiscretizeMMEIgnoreUnique_Float.o \
			testDiscretizeMMEIgnoreUnique_floatRepeats.o \
			testDiscretizeMMEIgnoreUnique_intRepeats.o \
			\
			testDiscretizeStrings.o \
			\
			main.o


.SUFFIXES: .cpp .sh

.cpp.o	:
	$(CXX) $(CXXFLAGS) -c $*.cpp -o $*.o

.sh.cpp	:
	sh $*.sh


##
##	Targets begin here
##

$(EXENAME) : $(OBJS) lib-c++ lib-common
	$(CXX) $(LDFLAGS) $(CXXFLAGS) \
		-o $(EXENAME) \
		$(OBJS) $(LDLIBS)

lib-common :
	( \
		cd ../../../common ; \
		make \
			RDEFINES="$(RDEFINES)" \
	)

lib-c++ :
	( \
		cd ../.. ; \
		make \
			RDEFINES="$(RDEFINES)" \
	)

clean : 
	- rm -f $(OBJS) $(EXENAME)
	- rm -f *.o */*.o core
	- rm -f main.cpp

allclean : clean
	- (cd ../.. ; make clean )
	- (cd ../../../common ; make clean )

tags ctags : dummy
	- ctags *.cpp ../../../common/*/*.c ../../*/*.cpp

main.cpp : dummy

dummy : 

