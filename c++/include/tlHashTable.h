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
 * CUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, HASH, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *..
 * $Id: tlHashTable.h 57 2013-12-13 21:33:01Z andrew $
 */


#ifndef		__TOOL_HASH_TABLE_HEADER__
#define		__TOOL_HASH_TABLE_HEADER__

#ifndef MAKDEPEND
# include <stdio.h>
# include <stdarg.h>
#endif

#include "os_defs.h"

#include "tlRef.h"

class tlColumn;

/**
CLASS
		tlHashTable

	A simple hash-table class which manages pointers
	to data within user space.  Hashing is performed
	using a simple `accumulate bytes modulus table size'
	algorithm.
*/
class OS_EXPORT tlHashTable : public tlRef
{
public:

	////////////////////////////////////////
	// A value stored in a bin.  There will
	// be more than one value per bin if the
	// user has stored more than one unique
	// key which hashes to the same value.
	// <p>
	// Members are:
	// <table>
	// <tr><td>keySize_		<td>Size of stored key
	// <tr><td>key_		<td>Pointer to (user managed) key storage.
	//								If keys were allocated using malloc()
	//								and stored, this value is available for
	//								free()
	// <tr><td>valueSize_		<td>Size of user data
	// <tr><td>value_		<td>Pointer to (user managed) data storage.
	//								If data was allocated using malloc()
	//								and stored, this value is available for
	//								free(), as with the keys
	// </table>
	struct HashTableValue {
		short keySize_;
		void *key_;
		int valueSize_;
		void *value_;
	};

	////////////////////////////////////////
	// Internal hash bin.  These can be inspected
	// through the getMaxHashBin() and getHashBin() functions.
	// <p>
	// Members are:
	// <table>
	// <tr><td>isValid_		<td>
	//						Set to 1 if data is valid; if this
	//						value is 0, the contents of the
	//						other values are undefined
	// <tr><td>numEntries_		<td>
	//						number of entries in the value list
	// <tr><td>value_		<td>
	//						list of values stored in the hash table
	// </table>
	struct HashTableBin {
		short isValid_;
		int numEntries_;
		int blocks_;
		HashTableValue *value_;

		////////////////////////////////////////
		// Dump this bin
		int dumpHashBin(FILE *fp, int indent) const;
	};


	int numHashBins_;
	struct HashTableBin *hashBins_;

public:
	////////////////////////////////////////
	// Construct a hash table with a default
	// number of bins (currently 1999).
	tlHashTable();

	////////////////////////////////////////
	// Construct a hash table with <b>primeSize</b>
	// number of bins.  If <b>primeSize</b> is
	// not prime, the hash table will internally
	// `round up' to the next nearest prime number,
	// up to an internal maximum, curently 5003.
	tlHashTable(int primeSize);

protected:
	////////////////////////////////////////
	// Destructor -- called from unref()
	~tlHashTable();

public:

	////////////////////////////////////////
	// Put a value into the hash table.
	// <p>
	// Note that the argument <b>value</b>
	// is a `pointer to void', which
	// should be passed the address of an
	// external object referenced from
	// within the hash table.
	// <p>
	// The hash table only makes a copy of the
	// address passed in, not to the actual data.
	// This allows data of arbitrary size to be
	// referenced by the hash table, but requires
	// that the user manage the data the hash table
	// points to.  If the user wishes the hash table
	// to manage memory itself, it is recommended
	// that storage is allocated and then passed in.
	// A complete list of all internal values is
	// available with the getMaxHashBin() and getHashBin()
	// interface below.
	// <p>
	// The following example supposes that the values stored
	// are doubles and the keys are integers.  Storage of
	// the doubles is assumed to be managed by the user.
	// Access of these values is demonstrated in lookup(), below.
	// <pre>
	// int key, status;
	// double *valueP = (reference to externally managed double);
	// key = 12345; /** some key value */;
	//
	// status = insert(key, sizeof(int), valueP, sizeof(double));
	// if (status == 0) {
	//     ... failed attempt to insert duplicate key ...
	// }
	// </pre>
	// At this point the hash table has stored a copy of the address
	// in <b>value</b>;
	// <p>
	// See also lookup().
	int insert(
				void *key, short keySize,
				void *value, int valueSize,
				void *userData = NULL
		    );

	////////////////////////////////////////
	// Look a value up within the hash table.
	// <p>
	// Note that the argument <b>value</b>
	// is a pointer to pointer to void, which
	// is intended to be used to populate a
	// pointer passed from the parent context
	// which will `point at' the internal value.
	// <p>
	// The argument <b>valueSize</b> should be
	// passed the address of an integer argument
	// which can then be used to verify that the
	// size of the value is as expected, if desired.
	// <p>
	// This example supposes that the values stored
	// are doubles and the keys are integers,
	// as in the insert() example above.
	// <pre>
	// int key, status;
	// int valueSize;
	// double *valueP, value;
	//
	// status = lookup(key, sizeof(int), &valueP, &valueSize);
	// if (status == 0) {
	//     ... failure ...
	// }
	// assert(valueSize == sizeof(double));
	// value = (*valueP);
	// </pre>
	// <p>
	// See also insert().
	int lookup(
					void *key, short keySize,
				void **value, int *valueSize,
				void *userData = NULL
		    ) const;

	////////////////////////////////////////
	// get the maximum hash bin.  Iteration up
	// to this point will allow the user to
	// inspect all stored values.  See getHashBin() below.
	int getMaxHashBin() const;

	////////////////////////////////////////
	// Return a hash bin from the internal list.
	// Note that if the flag <b>isValid</b> is
	// not set, then the values in the other
	// members are undefined.
	// <p>
	// This interface is primarily intended to be
	// used to free resources stored only in the
	// hash table by means of a loop of the order:
	// <pre>
	// int i, j;
	//
	// for (i = 0; i < hashTable->getMaxHashBin(); i++) {
	//     if (hashTable->getHashBin(i)->isValid_) {
	//         for (j = 0; j < hashTable->getHashBin(i)->numEntries_; j++) {
	//
	//             /** free keys if result of malloc() was stored */
	//             free( hashTable->getHashBin(i)->value_[j].key_ );
	//
	//             /** free values if result of malloc() was stored */
	//             free( hashTable->getHashBin(i)->value_[j].value_ );
	//         }
	//     }
	// }
	// </pre>
	HashTableBin *getHashBin(int index);

	////////////////////////////////////////
	// Calculate a hash value based on a key.  The default
	// algorithm adds all bytes passed in, taking
	// the modulus of this sum relative to <b>numHashBins_</b>,
	// (the length of the internal table).  The result is
	// therefore a mapping across <b>numHashBins_</b> values;
	// <b>numHashBins_</b> is fixed to be a prime number
	// by the constructor.
	// <p>
	// Overriding this function in a child class allows user
	// code to supply a (potentially more efficient) hashing
	// algorithm.
	virtual int calculateHash(
		    void *voidKey,
		    short keySize,
		    void *userData
		) const;

	////////////////////////////////////////
	// Access to override the key comparison
	// done to identify keys in buckets.
	// <p>
	// If you have changed the type of the
	// keys from simple bitstrings, you will
	// need to override this and the calculateHash()
	// function.
	virtual int compareKey(
		    void *voidKey1,
		    void *voidKey2,
		    short keySize,
		    void *userData
		) const;

	////////////////////////////////////////
	// Dump the whole table
	int dumpHashTable(FILE *fp, int indent, int printInvalid = 0) const;

};

inline int tlHashTable::getMaxHashBin() const
{
	if (hashBins_ == NULL)
		return 0;

	return numHashBins_;
}

inline tlHashTable::HashTableBin *tlHashTable::getHashBin(int index)
{
	return &hashBins_[index];
}

#endif /* __TOOL_HASH_TABLE_HEADER__ */

