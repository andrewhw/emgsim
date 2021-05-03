##	$Id: Makefile 103 2012-05-25 14:22:21Z andrew $

SUBDIRS	=  \
	arrayAlloc \
	attrval \
	bitstring \
	commandpipe \
	histogram \
	mathtools \
	random \
	tokenizer

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
