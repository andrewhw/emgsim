##
## $Id: Makefile 13 2012-05-25 17:15:32Z andrew $
##


CC			=	gcc

MAKE			=	make
SHELL			=	/bin/sh

EXE_1			=	gammavol
EXE_2			=	spherevol
EXELIST			=	$(EXE_1) $(EXE_2)

DEFINES			=	

INCLUDEFLAGS		=	-I../include

CFLAGS			=	-g $(DEFINES) $(INCLUDEFLAGS) \
                                -pedantic -Wall

LDFLAGS			=	
LDLIBS			=	-lm 



OBJS_1			=	\
				gammavol.o

OBJS_2			=	\
				spherevol.o


all	: $(EXELIST)


.SUFFIXES: .o .c

.c.o	:
	$(CC) $(CFLAGS) -c $*.c -o $*.o

##
##	Targets begin here
##

$(EXE_1) : $(OBJS_1)
	$(CC) $(LDFLAGS) $(CFLAGS) -o $(EXE_1) $(OBJS_1) $(LDLIBS)

$(EXE_2) : $(OBJS_2)
	$(CC) $(LDFLAGS) $(CFLAGS) -o $(EXE_2) $(OBJS_2) $(LDLIBS)

clean allclean :
	- rm -f $(OBJS_1) $(OBJS_2) $(EXELIST $(MAKEDEPENDRULES))
	- rm -f *.o */*.o core

tags ctags : dummy
	- ctags *.c ../src/*.c


## this target is here for 'always do' rules to depend on

dummy :

