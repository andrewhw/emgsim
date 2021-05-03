/**
 ** Mainline routine for updated simulator, begun Summer 2001.
 **
 ** $Id: testBitstring.c 61 2010-01-23 21:46:56Z andrew $
 **/
#include <stdio.h>
#include <string.h>

#include "bitstring.h"
#include "massert.h"


int
testAString(int nBits)
{
	int nBytes;
	int i, v;
	BITSTRING s;

	nBytes = BITSTRING_LEN(nBits);
	printf("<TEST> Testing bitstring of %d bits (%d bytes) in length . . .\n",
					nBits, nBytes);

	printf("<TEST> Allocating and initializing . . .\n");
	s = ALLOC_BITSTRING(nBits);
	memset(s, 0, nBytes);

	printf("<TEST> Read 0 Test . . . ");
	for (i = 0; i < nBits; i++) {
		VALIDATE_MEMORY;
		v = GET_BIT(s, i);
		VALIDATE_MEMORY;
		if (v == 0) {
			printf("<PASS>\n");
		} else {
			printf("<FAIL> error 0\n");
		}
	}

	printf("<TEST> Setting '1' . . . ");
	for (i = 0; i < nBits; i++) {
		VALIDATE_MEMORY;
		SET_BIT(s, i, 1);
		VALIDATE_MEMORY;
	}

	printf("<TEST> Reading '1' . . . ");
	for (i = 0; i < nBits; i++) {
		VALIDATE_MEMORY;
		v = GET_BIT(s, i);
		VALIDATE_MEMORY;
		if (v == 1) {
			printf("<PASS>\n");
		} else {
			printf("<FAIL> error 1\n");
		}
	}


	printf("<TEST> Setting '0' . . . ");
	for (i = 0; i < nBits; i++) {
		VALIDATE_MEMORY;
		SET_BIT(s, i, 0);
		VALIDATE_MEMORY;
		v = GET_BIT(s, i);
		if (v == 0) {
			printf("<PASS>\n");
		} else {
			printf("<FAIL> error 0-1\n");
		}
	}

	printf("<TEST> Reading '0' . . . ");
	for (i = 0; i < nBits; i++) {
		VALIDATE_MEMORY;
		v = GET_BIT(s, i);
		VALIDATE_MEMORY;
		if (v == 0) {
			printf("<PASS>\n");
		} else {
			printf("<FAIL> error 0-2\n");
		}
	}



	/** ---------------------------------------------- */


	printf("<TEST> Setting i %% 2 . . . ");
	for (i = 0; i < nBits; i++) {
		VALIDATE_MEMORY;
		SET_BIT(s, i, i % 2);
		VALIDATE_MEMORY;
	}

	printf("<TEST> Reading i %% 2 . . . ");
	for (i = 0; i < nBits; i++) {
		VALIDATE_MEMORY;
		v = GET_BIT(s, i);
		VALIDATE_MEMORY;
		if (v == i % 2) {
			printf("<PASS>\n");
		} else {
			printf("<FAIL> error 2-even\n");
		}
	}



	printf("<TEST> Setting (i+1) %% 2 . . . ");
	for (i = 0; i < nBits; i++) {
		VALIDATE_MEMORY;
		SET_BIT(s, i, ((i+1) % 2));
		VALIDATE_MEMORY;
	}

	printf("<TEST> Reading (i+1) %% 2 . . . ");
	for (i = 0; i < nBits; i++) {
		VALIDATE_MEMORY;
		v = GET_BIT(s, i);
		VALIDATE_MEMORY;
		if (v == ((i+1) % 2)) {
			printf("<PASS>\n");
		} else {
			printf("<FAIL> error 2-odd\n");
		}
	}


	VALIDATE_MEMORY;
	FREE_BITSTRING(s);
	VALIDATE_MEMORY;

	return 1;
}

static int nBits[] = { 1, 2, 7, 8, 9, 128, 255, 256, 1024, 65536, (-1) };

int
testBitstring(int argc, char **argv)
{
	int status = 1;
	int i;

	for (i = 0; nBits[i] > 0; i++) {
		if ( ! testAString(nBits[i]) ) {
			printf("<FAIL> Test with %d bits failed\n", nBits[i]);
			status = 0;
		}
	}

	return status;
}

