/**
 * Copyright (c) 2013
 * All rights reserved.
 *
 * This code is part of the reseach work of
 * Andrew Hamilton-Wright (andrewhw@ieee.org).
 *
 * ----------------------------------------------------------------
 *
 * Redistribution and use in source and binary forms, with or with-
 * out modification, are permitted provided that recognition of the
 * author as the original contributor is provided in any source or
 * documentation relating to this code.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WAR-
 * RANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.
 *
 * IN NO EVENT SHALL THE AUTHOR OR ANY ASSOCIATED INSTITUTION BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PRO-
 * CUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * $Id: tlSTable.h 57 2013-12-13 21:33:01Z andrew $
 */


#ifndef		__TABLE_LOAD_TOOL_HEADER__
#define		__TABLE_LOAD_TOOL_HEADER__

#include "os_defs.h"

#include "tlRef.h"
#include "tlSrString.h"

#include "plottools.h"
#include "listalloc.h"

struct attValList;
class tlErrorManager;
class tlTable;
class tlTuple;

/**
CLASS
		tlSTable

	Static tools for loading tlTables
*/
class OS_EXPORT tlSTable
{
protected:
	tlSTable();

protected:
	~tlSTable();

public:

	////////////////////////////////////////
	// Read in data from open text file pointer, which 
	// is assumed to begin with
	// a list of string labels for columns
	static int loadTable(
		    tlTable *data,
		    FILE *ifp,
		    const char *delimiters,
		    tlErrorManager *err,
		    const char *filename = NULL,
		    FILE *verboseFP = NULL
		);

	////////////////////////////////////////
	// Read in data from the given text file, which 
	// is assumed to begin with
	// a list of string labels for columns
	static int loadTable(
		    tlTable *data,
		    const char *filename,
		    const char *delimiters,
		    tlErrorManager *err,
		    FILE *verboseFP = NULL
		);

	////////////////////////////////////////
	// Read in header from text file
	// (assumed to be a list of string
	// labels for columns)
	static int parseDataHeader(
		    tlTable *data,
		    struct tokenizer *t,
		    const char *filename,
		    const char *delimiters,
		    tlErrorManager *err
		);

	////////////////////////////////////////
	// Verify that the columns found in the
	// header are not new to us, and flag
	// what order they are in in the new file
	static int verifyDataHeader(
		    tlTable *data,
		    struct tokenizer *t,
		    const char *filename,
		    const char *delimiters,
		    tlErrorManager *err
		);

	////////////////////////////////////////
	// Read in the data as an array of values
	static int parseAndReorgSingleDataLine(
		    tlTuple *newlyReadLine,
		    tlTable *data,
		    struct tokenizer *t,
		    const char *filename,
		    const char *delimiters,
		    tlTuple *workingTuple,
		    tlErrorManager *err
		);

	////////////////////////////////////////
	// Read in the data as an array of values
	static int parseDataLines(
		    tlTable *data,
		    struct tokenizer *t,
		    const char *filename,
		    const char *delimiters,
		    tlErrorManager *err,
		    FILE *verboseFP = NULL
		);

	////////////////////////////////////////
	// Read in a single line of data
	static int parseSingleDataLine(
		    struct tokenizer *t,
		    tlTuple *tuple,
		    const char *filename,
		    const char *delimiters,
		    tlErrorManager *err
		);
};

#endif /* __TABLE_LOAD_TOOL_HEADER__ */

