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
 * $Id: tlDocs.h 57 2013-12-13 21:33:01Z andrew $
 */


#ifndef		__TOOL_MASTER_HEADER__
#define		__TOOL_MASTER_HEADER__


/**
LIBRARY
		Tools

OVERVIEW
	This is a simple C++ tool set for building other applications.

	Classes of note include:
	<table>

	<tr><td>tlSrString
		<td>a self-managing string class similar in use and purpose
		    to the Java string class

	<tr><td>tlTable
		<td>a table structure for reading CVS type data

	<tr><td>tlBinTable
		<td>a child class of tlTable which can "bin" the data
		    in a couple of different ways, grouping like data
		    together

	<tr><td>tlErrorManager
		<td>a class used for reporting errors.  The default implementation
		    simply collects a set of string together and allows them
		    to be reported at one time; child classes could potentially
		    have a tie-in with some kind of GUI error manager to report
		    errors as they occur.

	<tr><td>tlHashTable
		<td>a generic associative map

	<tr><td>tlRef
		<td>a simple reference-counting mechanism; most of the classes
		    in this library are reference-counted.

	<tr><td>tlTuple
		<td>a list of typed values

	<tr><td>tlSParser
		<td>tools for parsing files

	<tr><td>tlSTable
		<td>tools for manipulating table values

	</table>

NOTES:
	This library depends on the C "common" library, which (unfortunately)
	cannot be documented here due to the limitations of cocoon.
*/

#endif /* __TOOL_MASTER_HEADER__ */

