##	$Id: Makefile 13 2012-05-25 17:15:32Z andrew $

SUBDIRS	=  \
	general

all : 
	@ for name in $(SUBDIRS); \
	do \
		echo "make in $$name" ; \
		( cd $$name ; make ) ; \
	done

clean : 
	@ for name in $(SUBDIRS); \
	do \
		echo "make clean in $$name" ; \
		( cd $$name ; make clean ) ; \
	done

allclean : clean
	( cd ../../common ; make clean )
	( cd .. ; make clean )
