##
## $Id: Makefile 55 2013-07-17 15:01:29Z andrew $
##


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
			testExpressions.o \
			\
			../utils/testutils.o \
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

