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
 * $Id: tlTVector.h 57 2013-12-13 21:33:01Z andrew $
 */


#ifndef		__TOOL_TEMPLATE_VECTOR_HEADER__
#define		__TOOL_TEMPLATE_VECTOR_HEADER__

#include "tclCkalloc.h"


#define		TL_T_VECTOR_BLOCKSIZE		8
/**
CLASS
		tlTVector

	A template vector class (similar to the STL vector) which can be
	used when the STL class cannot -- notably in MFC projects
*/
template <class T> class tlTVector
{
protected:
	int n_;
	int nAllocated_;
	T *value_;

public:
	////////////////////////////////////////
	// Constructor
	tlTVector();

	////////////////////////////////////////
	// Destructor
	~tlTVector();

	////////////////////////////////////////
	// return the length of the vector
	int getLength() const;

	////////////////////////////////////////
	// set the length of the vector
	void setLength(int length);

	////////////////////////////////////////
	// return value <b>index</b>
	T& getAt(int index) const;

	////////////////////////////////////////
	// Set a value in the vector
	void setAt(int index, T& value);

	////////////////////////////////////////
	// Create a plain array of these types,
	// allocated using ckalloc(),
	// of length getLength()
	T *allocateArray() const;

	////////////////////////////////////////
	// Returns a reference to the vector
	// element.  Used to get or set values.
	T& operator[] (int index);

	////////////////////////////////////////
	// Add a value to the vector, increasing the length
	void append(T& value);

	////////////////////////////////////////
	// Remove an element from a specific index
	void removeAt(int i);

	////////////////////////////////////////
	// Insert an element, moving all other elements
	// up the list
	void insertAt(int i, T& value);

	////////////////////////////////////////
	// Clear the old contents of the vector
	void clear();
};

template <class T>
tlTVector<T>::tlTVector()
{
	value_ = NULL;
	n_ = 0;
	nAllocated_ = 0;
}

template <class T>
tlTVector<T>::~tlTVector()
{
	clear();
}

template <class T>
int tlTVector<T>::getLength() const
{
	return n_;
}

template <class T>
void tlTVector<T>::clear()
{
	if (value_ != NULL) {
		delete [] value_;
		value_ = NULL;
		nAllocated_ = 0;
		n_ = 0;
	}
}

template <class T>
void tlTVector<T>::setLength(int length)
{

	if (length > nAllocated_) {
		T *old; 
		int nBlocks, i;

		old = value_;
		nBlocks = (length / TL_T_VECTOR_BLOCKSIZE) + 1;
		nAllocated_ = nBlocks * TL_T_VECTOR_BLOCKSIZE;
		value_ = new T [ nAllocated_ ];
		if (old != NULL) {
		    for (i = 0; i < n_; i++) {
				value_[i] = old[i];
		    }
		    delete [] old;
		}
	}
	n_ = length;
}

template <class T>
T&
tlTVector<T>::getAt(int i) const
{
	return value_[i];
}

template <class T>
void tlTVector<T>::setAt(int i, T& value)
{
	setLength(i+1);
	value_[i] = value;
}

template <class T>
void tlTVector<T>::append(T& value)
{
	setLength(n_ + 1);
	value_[n_-1] = value;
}

template <class T>
void tlTVector<T>::removeAt(int index)
{
	int i;

	for (i = index + 1; i < n_; i++) {
		value_[i - 1] = value_[i];
	}
	setLength(n_ - 1);
}

template <class T>
void tlTVector<T>::insertAt(int index, T& value)
{
	int oldLen;
	int i;

	oldLen = n_;
	setLength(n_ + 1);
	for (i = oldLen; i > index; i--) {
		value_[i] = value_[i - 1];
	}
	value_[index] = value;
}

template <class T>
T& tlTVector<T>::operator[] (int index)
{
	if (index >= n_) {
		setLength(index + 1);
	}
	return value_[index];
}

template <class T>
T* tlTVector<T>::allocateArray() const
{
	T* resultArray;

	resultArray = (T *) ckalloc(sizeof(T) * n_);
	memcpy(resultArray, value_, sizeof(T) * n_);
	return resultArray;
}

#endif /* __TOOL_TEMPLATE_VECTOR_HEADER__ */

