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
 * $Id: tlTDoubleLinkedList.h 57 2013-12-13 21:33:01Z andrew $
 */


#ifndef		__TOOL_TEMPLATE_DOUBLE_LINKED_LIST_HEADER__
#define		__TOOL_TEMPLATE_DOUBLE_LINKED_LIST_HEADER__

#include "massert.h"

# define		DBG_CHECK_BAD_DELETE(n) 		\
		MSG_ASSERT((n)->list_ != NULL, "Attempt to operate on deleted node!\n")


/**
CLASS
		tlTDoubleLinkedList

	A template linked list class for bidirectionally linked lists.
*/
template <class T> class tlTDoubleLinkedList
{
public:
	class tlTNodeDoubleLinkedList {
	public:
		T data_;
		class tlTNodeDoubleLinkedList * next_;
		class tlTNodeDoubleLinkedList * prev_;
		class tlTDoubleLinkedList *list_;

	public:
		tlTNodeDoubleLinkedList(
				T& value,
				class tlTDoubleLinkedList *list
				);
		~tlTNodeDoubleLinkedList();

		////////////////////////////////////////
		// return the managed data element for this node
		T& data();

		////////////////////////////////////////
		// delete this node from the list it is in
		void deleteNode();

	};

protected:
	class tlTNodeDoubleLinkedList *head_;
	class tlTNodeDoubleLinkedList *tail_;

public:
	int length_;

public:
	////////////////////////////////////////
	// Constructor
	tlTDoubleLinkedList();

	////////////////////////////////////////
	// Destructor
	~tlTDoubleLinkedList();

	////////////////////////////////////////
	// return the head of the list
	class tlTNodeDoubleLinkedList *getHead() const {
		return head_;
	};

	////////////////////////////////////////
	// return the tail of the list
	class tlTNodeDoubleLinkedList *getTail() const {
		return tail_;
	};

	////////////////////////////////////////
	// return the number of elements added
	// to the list
	int getLength() const;

	////////////////////////////////////////
	// Add a value to the beginning of the list, increasing the length
	void prepend(T value);

	////////////////////////////////////////
	// Add a value to the end of the list, increasing the length
	void append(T value);

	////////////////////////////////////////
	// Clear the old contents of the list
	void clear();

public:
	////////////////////////////////////////
	// INTERNAL FUNCTION
	//
	// set the head of the list -- used
	// only by iterator
	void setHead_(class tlTNodeDoubleLinkedList *newValue) {
		MSG_ASSERT(head_->list_ == newValue->list_,
					"attempt to set head with invalid node\n");
		head_ = newValue;
	};

	////////////////////////////////////////
	// INTERNAL FUNCTION
	//
	// return the tail of the list -- used
	// only by iterator
	void setTail_(class tlTNodeDoubleLinkedList *newValue) {
		MSG_ASSERT(tail_->list_ == newValue->list_,
					"attempt to set head with invalid node\n");
		tail_ = newValue;
	};

};


template <class T>
tlTDoubleLinkedList<T>::tlTDoubleLinkedList()
{
	head_ = NULL;
	tail_ = NULL;
}

template <class T>
tlTDoubleLinkedList<T>::~tlTDoubleLinkedList()
{
	clear();
}

template <class T>
void tlTDoubleLinkedList<T>::clear()
{
	class tlTNodeDoubleLinkedList *oldHead;

	while (head_ != NULL) {
		oldHead = head_;
		head_ = head_->next_;
		delete oldHead;
	}
	tail_ = NULL;
}

template <class T>
void tlTDoubleLinkedList<T>::prepend(T value)
{
//	tlTDoubleLinkedList<T>::tlTNodeDoubleLinkedList *newValue
//				= new tlTDoubleLinkedList<T>::tlTNodeDoubleLinkedList(
//								value,
//								this
//						    );

	tlTNodeDoubleLinkedList *newValue
				= new tlTNodeDoubleLinkedList(
								value,
								this
						    );

	if (head_ == NULL) {
		head_ = tail_ = newValue;
	} else {
		newValue->next_ = head_;
		head_->prev_ = newValue;
		head_ = head_->prev_;
	}
	length_++;
}

template <class T>
void tlTDoubleLinkedList<T>::append(T value)
{
	tlTNodeDoubleLinkedList *newValue
				= new tlTNodeDoubleLinkedList(
								value,
								this
						    );

	if (head_ == NULL) {
		head_ = tail_ = newValue;
	} else {
		newValue->prev_ = tail_;
		tail_->next_ = newValue;
		tail_ = tail_->next_;
	}
	length_++;
}

template <class T>
int
tlTDoubleLinkedList<T>::getLength() const
{
	return length_;
}

template <class T>
tlTDoubleLinkedList<T>::tlTNodeDoubleLinkedList::tlTNodeDoubleLinkedList(
		T& value,
		class tlTDoubleLinkedList *list
	)
{
	data_ = value;
	next_ = prev_ = NULL;
	list_ = list;
}

template <class T>
tlTDoubleLinkedList<T>::tlTNodeDoubleLinkedList::~tlTNodeDoubleLinkedList()
{
	list_ = NULL;
}

template <class T>
T&
tlTDoubleLinkedList<T>::tlTNodeDoubleLinkedList::data()
{
	DBG_CHECK_BAD_DELETE(this);

	return data_;
}

template <class T>
void
tlTDoubleLinkedList<T>::tlTNodeDoubleLinkedList::deleteNode()
{
	DBG_CHECK_BAD_DELETE(this);

	if (prev_ == NULL) {
		list_->head_ = next_;
	} else {
		prev_->next_ = next_;
	}

	if (next_ == NULL) {
		list_->tail_ = prev_;
	} else {
		next_->prev_ = prev_;
	}

	list_->length_--;

	delete this;
}


/**
CLASS
		tlTIteratorDoubleLinkedList

	Friend class for iterating through, and managing data in,
	a tlTDoubleLinkedList class.
*/
template <class T>
class tlTIteratorDoubleLinkedList
{
protected:
	class tlTDoubleLinkedList<T> *list_;
	typename tlTDoubleLinkedList<T>::tlTNodeDoubleLinkedList *curNode_;
	int isAtEnd_;
	int isAtBegin_;

public:
	////////////////////////////////////////
	// Constructor
	tlTIteratorDoubleLinkedList(tlTDoubleLinkedList<T> *list);

	////////////////////////////////////////
	// Destructor
	~tlTIteratorDoubleLinkedList();

	////////////////////////////////////////
	// return the list itself
	tlTDoubleLinkedList<T> *getList();

	////////////////////////////////////////
	// return the current list node
	typename tlTDoubleLinkedList<T>::tlTNodeDoubleLinkedList *node();

	////////////////////////////////////////
	// Returns true if the iterator has moved off
	// the beginning of the list
	// or the list is empty
	int isAtBegin();

	////////////////////////////////////////
	// Returns true if the iterator has moved off
	// the end of the list
	// or the list is empty
	int isAtEnd();

	////////////////////////////////////////
	// Move ahead in the list.
	// Returns 0 if already at tail 
	int next();

	////////////////////////////////////////
	// Move towards the beginning in the list.
	// Returns 0 if already at head 
	int prev();

	////////////////////////////////////////
	// set to head of list
	void gotoHead();

	////////////////////////////////////////
	// set to tail of list
	void gotoTail();

	////////////////////////////////////////
	// If the list is non-empty, add the new
	// value in a new node prior to the current
	// iterator location, and return 1;
	// if the list is empty, create a new
	// head node and return 0.
	//
	// In either case, the iterator will then
	// point at the newly added node
	//
	int insertBeforeCurrent(T newValue);

	////////////////////////////////////////
	// If the list is non-empty, add the new
	// value in a new node after the current
	// iterator location and return 1;
	// if the list is empty, create a new
	// head node and return 0.
	//
	// In either case, the iterator will then
	// point at the newly added node
	//
	int insertAfterCurrent(T newValue);

	////////////////////////////////////////
	// add to the beginning of the list,
	// updating the iterator to point at
	// the newly added value
	int prepend(T newValue);

	////////////////////////////////////////
	// add at the end of the list,
	// updating the iterator to point at
	// the newly added value
	int append(T newValue);

	////////////////////////////////////////
	// If the list is non-empty, delete
	// the current node and return 1.
	//
	// If the list is empty, simply return 0.
	//
	// The iterator will end up pointing at
	// the previous node from the one deleted
	// unless the head is deleted.
	//
	int deleteCurrent();

	////////////////////////////////////////
	// return the prev node in the list, then
	// decrement the list pointer
	T &operator --(void);

	////////////////////////////////////////
	// return the next node in the list after
	// decrement the list pointer
	T &operator ++(int);
};

template <class T>
tlTIteratorDoubleLinkedList<T>::tlTIteratorDoubleLinkedList(
		tlTDoubleLinkedList<T> *list
	)
{
	list_ = list;
	curNode_ = list_->getHead();
	isAtEnd_ = isAtBegin_ = 0;
}

template <class T>
tlTIteratorDoubleLinkedList<T>::~tlTIteratorDoubleLinkedList()
{
}

template <class T>
tlTDoubleLinkedList<T> *
tlTIteratorDoubleLinkedList<T>::getList()
{
	return list_;
}

template <class T>
typename tlTDoubleLinkedList<T>::tlTNodeDoubleLinkedList *
tlTIteratorDoubleLinkedList<T>::node()
{
	DBG_CHECK_BAD_DELETE(curNode_);

	return curNode_;
}

template <class T>
int
tlTIteratorDoubleLinkedList<T>::isAtBegin()
{
	DBG_CHECK_BAD_DELETE(curNode_);

	if ((list_ == NULL)
					|| (list_->getHead() == NULL)) {
		return 1;
	}

	return isAtBegin_;
}

template <class T>
int
tlTIteratorDoubleLinkedList<T>::isAtEnd()
{
	DBG_CHECK_BAD_DELETE(curNode_);

	if ((list_ == NULL)
					|| (list_->getHead() == NULL)) {
		return 1;
	}

	return isAtEnd_;
}


template <class T>
int
tlTIteratorDoubleLinkedList<T>::next()
{
	DBG_CHECK_BAD_DELETE(curNode_);

	if (curNode_->next_ == NULL) {
		isAtEnd_ = 1;
		return 0;
	}

	isAtEnd_ = 0;
	curNode_ = curNode_->next_;
	return 1;
}

template <class T>
int
tlTIteratorDoubleLinkedList<T>::prev()
{
	DBG_CHECK_BAD_DELETE(curNode_);

	if (curNode_->prev_ == NULL) {
		isAtBegin_ = 1;
		return 0;
	}

	isAtBegin_ = 0;
	curNode_ = curNode_->prev_;
	return 1;
}

template <class T>
void
tlTIteratorDoubleLinkedList<T>::gotoHead()
{
	curNode_ = list_->getHead();

	isAtBegin_ = 1;
	if (curNode_ == list_->getTail()) {
		isAtEnd_ = 1;
	} else {
		isAtEnd_ = 0;
	}

}

template <class T>
void
tlTIteratorDoubleLinkedList<T>::gotoTail()
{
	curNode_ = list_->getTail();

	isAtEnd_ = 1;
	if (curNode_ == list_->getHead()) {
		isAtBegin_ = 1;
	} else {
		isAtBegin_ = 0;
	}
}

template <class T>
int
tlTIteratorDoubleLinkedList<T>::insertBeforeCurrent(T newValue)
{
	typename tlTDoubleLinkedList<T>::tlTNodeDoubleLinkedList *newNode;

	if ((list_->getHead() == NULL) || (curNode_->prev_ == NULL)) {
		list_->prepend(newValue);
		curNode_ = list_->getHead();
		return 0;
	}

	DBG_CHECK_BAD_DELETE(curNode_);

	newNode = new typename
					tlTDoubleLinkedList<T>::tlTNodeDoubleLinkedList(
								newValue,
								list_
						    );

	newNode->next_ = curNode_;
	curNode_->prev_->next_ = newNode;

	newNode->prev_ = curNode_->prev_;
	curNode_->prev_ = newNode;

	curNode_ = newNode;

	list_->length_++;

	return 1;
}

template <class T>
int
tlTIteratorDoubleLinkedList<T>::insertAfterCurrent(T newValue)
{
	typename tlTDoubleLinkedList<T>::tlTNodeDoubleLinkedList *newNode;

	if ((list_->getTail() == NULL) || (curNode_->next_ == NULL)) {
		list_->append(newValue);
		curNode_ = list_->getTail();
		return 0;
	}

	DBG_CHECK_BAD_DELETE(curNode_);

	newNode = new typename
					tlTDoubleLinkedList<T>::tlTNodeDoubleLinkedList(
								newValue,
								list_
						    );

	newNode->prev_ = curNode_;
	curNode_->next_->prev_ = newNode;

	newNode->next_ = curNode_->next_;
	curNode_->next_ = newNode;

	curNode_ = newNode;

	list_->length_++;

	return 1;
}

template <class T>
int
tlTIteratorDoubleLinkedList<T>::prepend(T newValue)
{
	list_->prepend(newValue);
	curNode_ = list_->getHead();
	return 1;
}

template <class T>
int
tlTIteratorDoubleLinkedList<T>::append(T newValue)
{
	list_->append(newValue);
	curNode_ = list_->getTail();
	return 1;
}

template <class T>
int
tlTIteratorDoubleLinkedList<T>::deleteCurrent()
{
	typename tlTDoubleLinkedList<T>::tlTNodeDoubleLinkedList *deleteNode;
	int retStatus = 1;

	MSG_ASSERT(list_->getHead() != NULL, "Attempt to delete from empty list");
	DBG_CHECK_BAD_DELETE(curNode_);

	deleteNode = curNode_;

	curNode_ = curNode_->prev_;

	if (deleteNode->prev_ == NULL) {
		list_->setHead_(deleteNode->next_);
		curNode_ = list_->getHead();
		retStatus = 0;

	} else {
		deleteNode->prev_->next_ = deleteNode->next_;
	}

	if (deleteNode->next_ == NULL) {
		list_->setTail_(deleteNode->prev_);
	} else {
		deleteNode->next_->prev_ = deleteNode->prev_;
	}

	delete deleteNode;

	return retStatus;
}

template <class T>
T&
tlTIteratorDoubleLinkedList<T>::operator --(void)
{
	prev();
	T& result = node()->data();
	return result;
}

template <class T>
T&
tlTIteratorDoubleLinkedList<T>::operator ++(int)
{
	T& result = node()->data();
	next();
	return result;
}

#endif /* __TOOL_TEMPLATE_DOUBLE_LINKED_LIST_HEADER__ */

