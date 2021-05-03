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
 * $Id: tlHashTable.cpp 57 2013-12-13 21:33:01Z andrew $
 */


#ifndef MAKEDEPEND
# include <stdlib.h>
# include <string.h>
# include <stdarg.h>
# include <ctype.h>
# include <math.h>
#endif

#include "tclCkalloc.h"
#include "listalloc.h"
#include "stringtools.h"

#include "tlHashTable.h"
#include "tlColumn.h"

#define		BLOCK_FACTOR		8
#define		DEFAULT_PRIME		1999
#define		MAX_PRIME		5003
#define		MIN_PRIME		13


static int sGetAPrime(int seed)
{
	int isPrime;
	double root;
	int testPrime, factor;

	if (seed <= MIN_PRIME)
		return MIN_PRIME;

	if ((seed % 2) == 0)
		seed++;

	for (testPrime = seed; testPrime < MAX_PRIME; testPrime += 2)
	{
		if ((testPrime % 2) == 0)
			continue;

		isPrime = 1;
		root = sqrt( (float)testPrime );

		/**
		 * Minimum possible divisor is 3 (we handle even numbers above
		 * so start with 3 and move up to the square root to find a
		 * new prime)
		 */
		for ( factor = 3; factor <= root; factor += 2)
		{
			if ((testPrime % factor) == 0)
			{
				isPrime = 0;
				break;
			}
		}
		if (isPrime)
		{
			return testPrime;
		}
	}
	return MAX_PRIME;
}

tlHashTable::tlHashTable()
{
	numHashBins_ = DEFAULT_PRIME;
	hashBins_ = NULL;
}

tlHashTable::tlHashTable(int primeSeed)
{
	numHashBins_ = sGetAPrime(primeSeed);
	hashBins_ = NULL;
}

tlHashTable::~tlHashTable()
{
	int i;

	if (hashBins_ != NULL)
	{
		for (i = 0; i < numHashBins_; i++)
		{
			if (hashBins_[i].value_ != NULL)
			{
				ckfree(hashBins_[i].value_);
			}
		}
		ckfree(hashBins_);
	}
}

int
tlHashTable::calculateHash(
		void *voidKey, short keySize,
		void * /*userData */
    ) const
{
	const char *charKey;
	int result = 0;
	int i;

	charKey = (const char *) voidKey;
	for (i = 0; i < keySize; i++)
	{
		result = (result + abs(charKey[i])) % numHashBins_;
	}
	return result;
}


int
tlHashTable::compareKey(
		void *voidKey1,
		void *voidKey2,
		short keySize,
		void * /*userData */
    ) const
{
	return memcmp(voidKey1, voidKey2, keySize);
}

int
tlHashTable::lookup(
		void *key,
		short keySize,
		void **value,
		int *valueSize,
		void *userData
    ) const
{
	int hashIndex;
	int i;

	if (hashBins_ == NULL)
		return 0;

	/** get the hashIndex value */
	hashIndex = calculateHash(key, keySize, userData);


	/** if the entry was not valid before, then mark it as valid */
	if ( ! hashBins_[hashIndex].isValid_ )
	{
		return 0;
	}

	/** if it was valid, search for the key in this bin */
	for (i = 0; i < hashBins_[hashIndex].numEntries_; i++)
	{
		if (compareKey(hashBins_[hashIndex].value_[i].key_, key,
					keySize, userData) == 0)
		{
			/** we have found the key, so load up the value & size */
			(*value) = hashBins_[hashIndex].value_[i].value_;
			(*valueSize) = hashBins_[hashIndex].value_[i].valueSize_;

			return 1;
		}
	}

	return 0;
}

int
tlHashTable::insert(
		void *key, short keySize,
		void *value, int valueSize,
		void *userData
    )
{
	int hashIndex;
	int status;
	int i;

	if (hashBins_ == NULL)
	{
		hashBins_ = (struct HashTableBin *)
					ckalloc( numHashBins_ * sizeof(struct HashTableBin));
		MSG_ASSERT(hashBins_ != NULL, "Out of memory");
		memset(hashBins_, 0,
				numHashBins_ * sizeof(struct HashTableBin));
	}

	/** get the hashIndex value */
	hashIndex = calculateHash(key, keySize, userData);


	/** if the entry was not valid before, then mark it as valid */
	if ( ! hashBins_[hashIndex].isValid_ )
	{
		hashBins_[hashIndex].isValid_ = 1;

	} else
	{
		/**
		 * if bin is valid, check that we do not have
		 * this key already in the bin
		 */
		for (i = 0; i < hashBins_[hashIndex].numEntries_; i++)
		{
			if (compareKey(hashBins_[hashIndex].value_[i].key_, key,
						keySize, userData) == 0)
				return 0;
		}
	}

	/**
	 * If we get here we need to add in the new value to the list
	 * of hashed values for this bin
	 */
	status = listCheckSize(
			hashBins_[hashIndex].numEntries_ + 1,
			(void **) &hashBins_[hashIndex].value_,
			&hashBins_[hashIndex].blocks_,
			BLOCK_FACTOR,
			sizeof(HashTableValue));
	MSG_ASSERT(status, "Out of memory");

	/** record the values into the new structure */
	{
		struct HashTableValue *record;
		record = &hashBins_[hashIndex].value_[
					hashBins_[hashIndex].numEntries_
				];

		record->keySize_ = keySize;
		record->key_ = key;
		record->value_ = value;
		record->valueSize_ = valueSize;
	}
	hashBins_[hashIndex].numEntries_++;
	
	return 1;
}

int
tlHashTable::HashTableBin::dumpHashBin(FILE *fp, int indent) const
{
	int i;

	fprintf(fp, "%*sHashBin contains %d entries:\n",
					indent, "", numEntries_);
	for (i = 0; i < numEntries_; i++)
	{

		if (value_[i].key_ == NULL)
		{
			fprintf(fp, "%*s%3d : key  <nul>\n",
				indent, "",
				i);
		} else if (value_[i].keySize_ == sizeof(long))
		{
			fprintf(fp, "%*s%3d : key  <0x%lx>\n",
				indent, "",
				i,
				(long) value_[i].key_);
		} else
		{
			char *ustr;

			ustr = strunctrl((const char *) value_[i].key_,
							value_[i].keySize_);
			fprintf(fp, "%*s%3d : key  <0x%08lx %ld> [%s]\n",
				indent, "",
				i,
				(unsigned long) value_[i].key_,
				(long) value_[i].key_,
				ustr);
		}


		if (value_[i].value_ == NULL)
		{
			fprintf(fp, "%*s%3d : data <nul>\n", indent, "", i);
		} else if (value_[i].valueSize_ == sizeof(long))
		{
			fprintf(fp, "%*s%3d : data <0x%lx>\n",
				indent, "",
				i,
				(long) value_[i].value_);
		} else
		{
			fprintf(fp, "%*s%3d : data <0x%08lx %ld> [%s]\n",
				indent, "",
				i,
				(unsigned long) value_[i].value_,
				(long) value_[i].value_,
				strunctrl((const char *) value_[i].value_,
							value_[i].valueSize_));
		}

	}

	return ( ! ferror(fp));
}

int
tlHashTable::dumpHashTable(FILE *fp, int indent, int printInvalid) const
{
	int i;

	fprintf(fp, "%*sHash Table contains %d bins:\n",
					indent, "", numHashBins_);
	for (i = 0; i < numHashBins_; i++)
	{
		if (hashBins_[i].isValid_)
		{
			fprintf(fp, "%*s%4d : valid\n", indent, "", i);
			if ( ! hashBins_[i].dumpHashBin(fp, indent + 4) )
				return 0;
		} else if (printInvalid)
		{
			fprintf(fp, "%*s%4d : invalid\n", indent, "", i);
		}
	}

	return (!ferror(fp));
}

