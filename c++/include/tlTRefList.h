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
 * $Id: tlTRefList.h 57 2013-12-13 21:33:01Z andrew $
 */


#ifndef		__TEMPLATE_REF_LIST_HEADER__
#define		__TEMPLATE_REF_LIST_HEADER__

#include "tclCkalloc.h"


#define		T_REFLIST_BLOCKSIZE		8
/**
CLASS
		tlTRefList

	Similar to the tlTVector class, but intelligently
	handles/holds/references tlRef based objects
*/
template <class T> class tlTRefList
{
protected:
	int n_;
	int nAllocated_;
	T **reflist_;

public:
	////////////////////////////////////////
	// Constructor
	tlTRefList();

	////////////////////////////////////////
	// Destructor
	~tlTRefList();

	////////////////////////////////////////
	// return the length of the vector
	int getLength() const;

	////////////////////////////////////////
	// set the length of the vector
	void setLength(int length);

	////////////////////////////////////////
	// return value <b>index</b>
	T* getAt(int index) const;

	////////////////////////////////////////
	// Set a value in the vector
	void setAt(int index, T* value);

	////////////////////////////////////////
	// Returns a reference to the vector
	// element.  Used to get or set values.
	T* operator[] (int index);

	////////////////////////////////////////
	// Add a value to the vector, increasing the length
	// and incrementing its reference
	void append(T* value);

	////////////////////////////////////////
	// Remove an element from a specific index,
	// decrementing its reference and moving
	// up all remaining elements in the list
	void removeAt(int i);

	////////////////////////////////////////
	// Insert an element, moving all other elements
	// up the list
	void insertAt(int i, T* value);

	////////////////////////////////////////
	// Clear the old contents of the vector
	void clear();
};

template <class T>
tlTRefList<T>::tlTRefList()
{
	reflist_ = NULL;
	n_ = 0;
	nAllocated_ = 0;
}

template <class T>
tlTRefList<T>::~tlTRefList()
{
	clear();
}

template <class T>
int tlTRefList<T>::getLength() const
{
	return n_;
}

template <class T>
void tlTRefList<T>::clear()
{
	int i;

	if (reflist_ != NULL) {
		for (i = 0; i < n_; i++) {
		    if (reflist_[i] != NULL)
				reflist_[i]->unref();
		}
		ckfree(reflist_);
		reflist_ = NULL;
		nAllocated_ = 0;
		n_ = 0;
	}
}

template <class T>
void tlTRefList<T>::setLength(int length)
{
	int i;

	/** clear out any references that may exist above the new length */
	for (i = length; i < n_; i++) {
		if (reflist_[i] != NULL) {
		    reflist_[i]->unref();
		    reflist_[i] = NULL;
		}
	}

	/** allocate more space if need be */
	if (length > nAllocated_) {
		T **old; 
		int nBlocks;

		old = reflist_;
		nBlocks = (length / T_REFLIST_BLOCKSIZE) + 1;
		nAllocated_ = nBlocks * T_REFLIST_BLOCKSIZE;
		reflist_ = (T**) ckalloc( nAllocated_ * sizeof(T*) );
		if (old != NULL) {
		    for (i = 0; i < n_; i++) {
				reflist_[i] = old[i];
		    }
		    while (i < nAllocated_) {
				reflist_[i] = NULL;
				i++;
		    }
		    delete [] old;
		} else {
		    for (i = 0; i < nAllocated_; i++) {
				reflist_[i] = NULL;
		    }
		}
	}
	n_ = length;
}

template <class T>
T*
tlTRefList<T>::getAt(int i) const
{
	return reflist_[i];
}

template <class T>
void tlTRefList<T>::setAt(int i, T* value)
{
	setLength(i+1);
	reflist_[i] = value;
	if (reflist_[i] != NULL)
		reflist_[i]->ref();
}

template <class T>
void tlTRefList<T>::append(T* value)
{
	setLength(n_ + 1);
	reflist_[n_-1] = value;
	if (reflist_[n_-1] != NULL)
		reflist_[n_-1]->ref();
}

template <class T>
void tlTRefList<T>::removeAt(int index)
{
	int i;

	if (reflist_[index] != NULL)
		reflist_[index]->unref();
	for (i = index + 1; i < n_; i++) {
		reflist_[i - 1] = reflist_[i];
	}
	setLength(n_ - 1);
}

template <class T>
void tlTRefList<T>::insertAt(int index, T* value)
{
	int oldLen;
	int i;

	oldLen = n_;
	setLength(n_ + 1);
	for (i = oldLen; i > index; i--) {
		reflist_[i] = reflist_[i - 1];
	}
	reflist_[index] = value;
	if (reflist_[index] != NULL)
		reflist_[index]->ref();
}

template <class T>
T* tlTRefList<T>::operator[] (int index)
{
	if (index >= n_) {
		setLength(index + 1);
	}
	return reflist_[index];
}

#endif /* __TEMPLATE_REF_LIST_HEADER__ */

