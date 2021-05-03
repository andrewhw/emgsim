##
## $Id: Makefile 14 2012-05-25 17:45:23Z andrew $
##


CC			=	gcc
MAKE			=	make
SHELL			=	/bin/sh

EXENAME			=	testcase

RDEFINES		=	-DDEBUG \
				-DTCL_MEM_DEBUG

DEFINES			=	$(RDEFINES)

#				-DEXTENDED_DIRECTORY_NAMES


INCLUDEFLAGS		=	-I. -I../../include \
				-I../utils \
				-I../../../common/include

CFLAGS			=	-g $(DEFINES) $(INCLUDEFLAGS) -pedantic -Wall

LDFLAGS			=	-L../../../common/lib \
				-L../../lib

LDLIBS			=	-lrtree -lcommon -lm

OBJS			= \
			\
			../utils/testutils.o \
			\
			testSample.o \
			testNested.o \
			testOverlap.o \
			\
			main.o

all : $(EXENAME)


.SUFFIXES: .c .sh

.sh.c	:
	sh $*.sh


##
##	Targets begin here
##

$(EXENAME) : $(OBJS) lib-common lib-rtree
	$(CC) $(LDFLAGS) $(CFLAGS) \
		-o $(EXENAME) \
		$(OBJS) $(LDLIBS)

lib-rtree :
	( \
		cd ../.. ; \
		make \
			RDEFINES="$(RDEFINES)" \
	)

lib-common :
	( \
		cd ../../../common ; \
		make \
			RDEFINES="$(RDEFINES)" \
	)

clean : 
	- rm -f $(OBJS) $(EXENAME)
	- rm -f *.o core
	- rm -f main.c

allclean : clean
	- (cd ../../../common ; make clean )
	- (cd ../.. ; make clean )

tags ctags : dummy
	- ctags *.c ../utils/*.c ../../../common/*/*.c ../../src/*.c

main.c : dummy

dummy : 
