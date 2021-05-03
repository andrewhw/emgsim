##
## $Id: Makefile 109 2013-07-17 15:01:18Z andrew $
##


MAKE			=	make
SHELL			=	/bin/sh

EXENAME			=	testcase

RDEFINES		=	-g -DDEBUG \
				-DUSE_NUMERICAL_RECIPES_RANDOM \
				-DMEM_DEPRECATION_OK -DTCL_MEM_DEBUG

DEFINES			=	$(RDEFINES)

INCLUDEFLAGS	=	-I. -I../../include -I../utils

CFLAGS			=	-g $(DEFINES) $(INCLUDEFLAGS) -pedantic -Wall

LDFLAGS			=	-L../../lib

LDLIBS			=	-lcommon -lm

OBJS			= \
			../utils/testutils.o \
			\
			test_2d_double.o \
			test_2d_int.o \
			test_2d_short.o \
			test_2d_char.o \
			\
			test_3d_double.o \
			test_3d_int.o \
			test_3d_short.o \
			test_3d_char.o \
			\
			main.o

GENERATED_SOURCE = \
			test_2d_double.c \
			test_2d_int.c \
			test_2d_short.c \
			test_2d_char.c \
			\
			test_3d_double.c \
			test_3d_int.c \
			test_3d_short.c \
			test_3d_char.c


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


test_2d_double.c : 2d_TYPE.c
	sed -e 's:TYPE:double:g' 2d_TYPE.c > test_2d_double.c

test_2d_int.c : 2d_TYPE.c
	sed -e 's:TYPE:int:g' 2d_TYPE.c > test_2d_int.c

test_2d_short.c : 2d_TYPE.c
	sed -e 's:TYPE:short:g' 2d_TYPE.c > test_2d_short.c

test_2d_char.c : 2d_TYPE.c
	sed -e 's:TYPE:char:g' 2d_TYPE.c > test_2d_char.c


test_3d_double.c : 3d_TYPE.c
	sed -e 's:TYPE:double:g' 3d_TYPE.c > test_3d_double.c

test_3d_int.c : 3d_TYPE.c
	sed -e 's:TYPE:int:g' 3d_TYPE.c > test_3d_int.c

test_3d_short.c : 3d_TYPE.c
	sed -e 's:TYPE:short:g' 3d_TYPE.c > test_3d_short.c

test_3d_char.c : 3d_TYPE.c
	sed -e 's:TYPE:char:g' 3d_TYPE.c > test_3d_char.c


lib-common :
	( \
		cd ../.. ; \
		make RDEFINES="$(RDEFINES)" \
	)

clean : 
	- rm -f $(OBJS) $(EXENAME)
	- rm -f *.o */*.o core
	- rm -f main.c
	- rm -f $(GENERATED_SOURCE)

allclean : clean
	- (cd ../.. ; make clean )

tags ctags : dummy
	- ctags *.c ../../*/*.c

main.c : dummy

dummy :

