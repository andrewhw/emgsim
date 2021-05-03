##
## $Id: Makefile 139 2013-07-17 15:08:38Z andrew $
##


MAKE			=	make
SHELL			=	/bin/sh

EXENAME			=	testcase

RDEFINES		=	
DEFINES			=	$(RDEFINES)

#				-DEXTENDED_DIRECTORY_NAMES


INCLUDEFLAGS		=	-I../../include -I.

CXXFLAGS		=	-g $(DEFINES) $(INCLUDEFLAGS) -pedantic -Wall

LDFLAGS			=	-L../../lib

LDLIBS			=	-lDQEmgData -lm
#-lefence

OBJS			= \
			testNumChannels.o \
			\
			main.o

all : $(EXENAME)


.SUFFIXES: .o .cpp .sh

.cpp.o	:
	$(CXX) $(CXXFLAGS) -c $*.cpp -o $*.o

.sh.cpp	:
	sh $*.sh


##
##	Targets begin here
##

$(EXENAME) : $(OBJS) lib-DQEmgData
	$(CXX) $(LDFLAGS) $(CXXFLAGS) \
		-o $(EXENAME) \
		$(OBJS) $(LDLIBS)

lib-DQEmgData :
	( cd ../.. ; make )

clean : 
	- rm -f $(OBJS) $(EXENAME)
	- rm -f *.o */*.o core
	- rm -f main.cpp

allclean : clean
	( cd ../.. ; make clean )

tags ctags : dummy
	- ctags *.cpp ../../src/*.cpp

main.cpp : dummy

dummy : 

