##	$Id: Makefile 52 2012-05-25 14:22:17Z andrew $

SUBDIRS	=  \
	TVector \
	TLinkedList \
	tlTable \
	tlMatrix \
	tlTrie \
	tlExpression

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

