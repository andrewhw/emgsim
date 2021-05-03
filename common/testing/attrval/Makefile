##
## $Id: Makefile 109 2013-07-17 15:01:18Z andrew $
##


MAKE			=	make
SHELL			=	/bin/sh

EXENAME			=	testcase

RDEFINES		=	-g -DDEBUG

DEFINES			=	$(RDEFINES)

INCLUDEFLAGS	=	-I. -I../../include -I../utils

CFLAGS			=	-g $(DEFINES) $(INCLUDEFLAGS) -pedantic -Wall

LDFLAGS			=	-L../../lib

LDLIBS			=	-lcommon -lm

OBJS			= \
			../utils/testutils.o \
			\
			testSimple.o \
			\
			main.o

all	: $(EXENAME)


.SUFFIXES: .c .sh

.c.o	:
	$(CC) $(CFLAGS) -c $*.c -o $*.o

.sh.c	:
	sh $*.sh


##
##	Targets begin here
##

$(EXENAME) : $(OBJS) lib-common 
	$(CC) $(LDFLAGS) $(CFLAGS) -o $(EXENAME) $(OBJS) $(LDLIBS)


lib-common :
	( \
		cd ../.. ; \
		make RDEFINES="$(RDEFINES)" \
	)

clean : 
	- rm -f $(OBJS) $(EXENAME)
	- rm -f *.o */*.o core
	- rm -f main.c

allclean : clean
	- (cd ../.. ; make clean )

tags ctags : dummy
	- ctags *.c ../../*/*.c

main.c : dummy

dummy :

