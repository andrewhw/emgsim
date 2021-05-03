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
 * $Id: tlSrMatrix.h 57 2013-12-13 21:33:01Z andrew $
 */


#ifndef		__TOOL_MATRIX_HANDLE_HEADER__
#define		__TOOL_MATRIX_HANDLE_HEADER__

#ifndef MAKDEPEND
# include <stdio.h>
# include <stdarg.h>
#endif

#include "os_defs.h"

#include "tlMatrix.h"
#include "tlTuple.h"


/**
CLASS
		tlSrMatrix

	A smart self-reffing matrix wrapper for tlMatrix.
*/
class OS_EXPORT tlSrMatrix
{
protected:
	class IndexHelper;

public:

	////////////////////////////////////////
	// Construct with an initial data value;
	tlSrMatrix(tlMatrix *mat);

	////////////////////////////////////////
	// Construct with Nil value.
	tlSrMatrix();

protected:
	////////////////////////////////////////
	// Construct the Nil value (fudge).
	tlSrMatrix(int dummy);

public:
	////////////////////////////////////////
	// Copy constructor -- internal matrix
	// storage will be common between the
	// two siblings
	tlSrMatrix(const tlSrMatrix &sibling);

	////////////////////////////////////////
	// Construct with data from a tuple
	tlSrMatrix(const tlTuple *tuple);

	////////////////////////////////////////
	// Construct a matrix with the specified
	// number of columns, rows.
	//
	// Matrix will be filled with the provided
	// value
	tlSrMatrix(int nColumns, int nRows, double value = 0);

	////////////////////////////////////////
	// Destructor.  Internal matrix is only
	// deleted if this is the last handle
	// to it.
	~tlSrMatrix();

	////////////////////////////////////////
	// Set the values using a table
	void setValues(tlTuple *source);

	////////////////////////////////////////
	// Set the values using a table
	void setValues(tlTable *table);

	////////////////////////////////////////
	// Re-initialize the matrix to the
	// specified size; all values will be
	// set to the value provided (0 by default)
	void setSize(int nColumns, int nRows, double value = 0);

	////////////////////////////////////////
	// return the data from a subsection of this
	// matrix as a new matrix
	tlSrMatrix getSubMatrix(
				int rowBegin, int columnBegin,
				int rowEnd, int columEnd
		    );

	////////////////////////////////////////
	// Make this object share the internal
	// storage of <b>sibling</b>, managing
	// the reference count for these objects
	// intelligently.
	tlSrMatrix& operator= (const tlSrMatrix &sibling);

	////////////////////////////////////////
	// convert the tuple into a new internally
	// managed tlMatrix
	tlSrMatrix& operator= (const tlTuple *src);

	////////////////////////////////////////
	// bracket operator.  Note use of IndexHelper class.
	IndexHelper &operator[] (int row);

	////////////////////////////////////////
	// Comparison between two matrices, return true
	// if both internal buffers contain the
	// same matrix.
	friend int operator== (
		    const tlSrMatrix &mat1,
		    const tlSrMatrix &mat2
		);

	////////////////////////////////////////
	// Comparison between two matrices, return true
	// if both internal buffers contain
	// different matrix.
	friend int operator!= (
		    const tlSrMatrix &mat1,
		    const tlSrMatrix &mat2
		);

	////////////////////////////////////////
	// return a new matrix which is the
	// product of the two arguments
	friend tlSrMatrix operator* (
		    const tlSrMatrix &mat1,
		    const tlSrMatrix &mat2
		);

	////////////////////////////////////////
	// return a new matrix which is the
	// product of the two arguments
	friend tlSrMatrix operator* (
		    const tlSrMatrix &mat,
		    double scalar
		);

	////////////////////////////////////////
	// return a new matrix which is the
	// product of the two arguments
	friend tlSrMatrix operator* (
		    double scalar,
		    const tlSrMatrix &mat
		);

	////////////////////////////////////////
	// return a new matrix which is the
	// sum of the two arguments
	friend tlSrMatrix operator+ (
		    const tlSrMatrix &mat1,
		    const tlSrMatrix &mat2
		);

	////////////////////////////////////////
	// return a new matrix which is the
	// sum of the two arguments
	friend tlSrMatrix operator+ (
		    double scalar,
		    const tlSrMatrix &mat
		);

	////////////////////////////////////////
	// return a new matrix which is the
	// sum of the two arguments
	friend tlSrMatrix operator+ (
		    const tlSrMatrix &mat,
		    double scalar
		);

	////////////////////////////////////////
	// return a new matrix which is the
	// difference of the two arguments
	friend tlSrMatrix operator- (
		    const tlSrMatrix &mat1,
		    const tlSrMatrix &mat2
		);

	////////////////////////////////////////
	// return a new matrix which is the
	// difference of the two arguments
	friend tlSrMatrix operator- (
		    double scalar,
		    const tlSrMatrix &mat
		);

	////////////////////////////////////////
	// return a new matrix which is the
	// difference of the two arguments
	friend tlSrMatrix operator- (
		    const tlSrMatrix &mat,
		    double scalar
		);

	////////////////////////////////////////
	// return a new matrix which is the
	// scaled by the scalar value
	friend tlSrMatrix operator/ (
		    const tlSrMatrix &mat,
		    double scalar
		);

	////////////////////////////////////////
	// return a new matrix which is the
	// scaled by the scalar value
	friend tlSrMatrix operator/ (
		    double scalar,
		    const tlSrMatrix &mat
		);

	////////////////////////////////////////
	// Throw away the old internal representation
	// and set it to be shared with <b>newValue</b>
	void setValue(tlSrMatrix newValue);

	////////////////////////////////////////
	// Use the supplied value as the new wrapped representation
	void setValue(tlMatrix *newValue);

	////////////////////////////////////////
	// Return a pointer to the internally managed data
	const tlMatrix *getValue() const;

	////////////////////////////////////////
	// Return a handle to the inverse, calculating this
	// internally if need be
	const tlSrMatrix inverse();

	////////////////////////////////////////
	// Return a handle to the transpose, calculating this
	// internally if need be
	const tlSrMatrix transpose();

	////////////////////////////////////////
	// Calculate the determinant if required, and return
	// the result
	double determinant();

	////////////////////////////////////////
	// Return the values in a string
	tlSrString getValueString(int breakLines = 0) const;

	////////////////////////////////////////
	// return the number of rows
	int getNumRows() const;

	////////////////////////////////////////
	// return the number of columns
	int getNumColumns() const;


protected:
	tlMatrix *data_;
	tlMatrix *inverse_;
	tlMatrix *transpose_;

	double determinant_;
	int determinantValid_;

	class IndexHelper {
	public:
		int row_;
		class tlSrMatrix *mat_;

		////////////////////////////////////////
		// return the actual double stored in the
		// accompanying wrapped matrix.
		double &operator[] (int column) const;
	};

	IndexHelper indexHelper_;

public:
	////////////////////////////////////////
	// Statically declared matrix which is
	// guaranteed to be 'nil'.  This is
	// provided for assignment and comparison
	// purposes.
	static tlSrMatrix nil;

protected:
	////////////////////////////////////////
	// clean up static refs
	static void sExitCleanup();

public:
	friend class tlRefManager;
	friend class IndexHelper;
};


inline int tlSrMatrix::getNumRows() const
{
	return data_->getNumRows();
}

inline int tlSrMatrix::getNumColumns() const
{
	return data_->getNumColumns();
}

inline const tlMatrix *tlSrMatrix::getValue() const
{
	return data_;
}

inline void tlSrMatrix::setValue(tlSrMatrix s)
{
	if (data_ != NULL) {
		data_->unref();
	}
	*this = s;
}

inline tlSrString tlSrMatrix::getValueString(int breakLines) const
{
	if (data_ == NULL) {
		tlSrString s;
		s = "[]";
		return s;
	}

	return data_->getValueString(breakLines);
}

inline tlSrMatrix::IndexHelper &tlSrMatrix::operator[] (int row) {
	indexHelper_.row_ = row;
	return indexHelper_;
}

inline double &tlSrMatrix::IndexHelper::operator[] (int column) const {
	return (*(mat_->data_))[row_][column];
}

#endif /* __TOOL_MATRIX_HANDLE_HEADER__ */

