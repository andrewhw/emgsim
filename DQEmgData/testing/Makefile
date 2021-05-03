##	$Id: Makefile 134 2012-05-25 17:14:36Z andrew $

SUBDIRS	=  \
	compatibility \
	bounds

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
	( cd .. ; make clean )

