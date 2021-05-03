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
 * $Id: tlSrGslMatrix.h 57 2013-12-13 21:33:01Z andrew $
 */


#ifdef		GSL_LIBRARY_INSTALLED
#ifndef		__GSL_TOOL_MATRIX_HANDLE_HEADER__
#define		__GSL_TOOL_MATRIX_HANDLE_HEADER__

#ifndef MAKDEPEND
# include <stdio.h>
# include <stdarg.h>
#endif

#include "os_defs.h"

#include "tlGslMatrix.h"
#include "tlTuple.h"


/**
CLASS
		tlSrGslMatrix

	A smart self-reffing matrix wrapper for tlGslMatrix.
*/
class OS_EXPORT tlSrGslMatrix
{
protected:
	class IndexHelper;

public:

	////////////////////////////////////////
	// Construct with an initial data value;
	tlSrGslMatrix(tlGslMatrix *mat);

	////////////////////////////////////////
	// Construct with Nil value.
	tlSrGslMatrix();

protected:
	////////////////////////////////////////
	// Construct the Nil value (fudge).
	tlSrGslMatrix(int dummy);

public:
	////////////////////////////////////////
	// Copy constructor -- internal matrix
	// storage will be common between the
	// two siblings
	tlSrGslMatrix(const tlSrGslMatrix &sibling);

	////////////////////////////////////////
	// Construct with data from a tuple
	tlSrGslMatrix(const tlTuple *tuple);

	////////////////////////////////////////
	// Construct a matrix with the specified
	// number of columns, rows.
	//
	// GslMatrix will be filled with the provided
	// value
	tlSrGslMatrix(int nColumns, int nRows, double value = 0);

	////////////////////////////////////////
	// Destructor.  Internal matrix is only
	// deleted if this is the last handle
	// to it.
	~tlSrGslMatrix();

	////////////////////////////////////////
	// Set the values using a table
	void setValues(tlTuple *source);

	////////////////////////////////////////
	// Set the values using a table
	void setValues(tlTable *table);

	////////////////////////////////////////
	// Set all locations with the supplied value
	void flood(double value);

	////////////////////////////////////////
	// Re-initialize the matrix to the
	// specified size; all values will be
	// set to the value provided (0 by default)
	void setSize(int nColumns, int nRows, double value = 0);

	////////////////////////////////////////
	// return the data from a subsection of this
	// matrix as a new matrix
	tlSrGslMatrix getSubGslMatrix(
				int rowBegin, int columnBegin,
				int rowEnd, int columEnd
		    );

	////////////////////////////////////////
	// Make this object share the internal
	// storage of <b>sibling</b>, managing
	// the reference count for these objects
	// intelligently.
	tlSrGslMatrix& operator= (const tlSrGslMatrix &sibling);

	////////////////////////////////////////
	// convert the tuple into a new internally
	// managed tlGslMatrix
	tlSrGslMatrix& operator= (const tlTuple *src);

	////////////////////////////////////////
	// bracket operator.  Note use of IndexHelper class.
	IndexHelper &operator[] (int row);

	////////////////////////////////////////
	// Comparison between two matrices, return true
	// if both internal buffers contain the
	// same matrix.
	friend int operator== (
		    const tlSrGslMatrix &mat1,
		    const tlSrGslMatrix &mat2
		);

	////////////////////////////////////////
	// Comparison between two matrices, return true
	// if both internal buffers contain
	// different matrix.
	friend int operator!= (
		    const tlSrGslMatrix &mat1,
		    const tlSrGslMatrix &mat2
		);

	////////////////////////////////////////
	// return a new matrix which is the
	// product of the two arguments
	friend tlSrGslMatrix operator* (
		    const tlSrGslMatrix &mat1,
		    const tlSrGslMatrix &mat2
		);

	////////////////////////////////////////
	// return a new matrix which is the
	// product of the two arguments
	friend tlSrGslMatrix operator* (
		    const tlSrGslMatrix &mat,
		    double scalar
		);

	////////////////////////////////////////
	// return a new matrix which is the
	// product of the two arguments
	friend tlSrGslMatrix operator* (
		    double scalar,
		    const tlSrGslMatrix &mat
		);

	////////////////////////////////////////
	// return a new matrix which is the
	// sum of the two arguments
	friend tlSrGslMatrix operator+ (
		    const tlSrGslMatrix &mat1,
		    const tlSrGslMatrix &mat2
		);

	////////////////////////////////////////
	// return a new matrix which is the
	// sum of the two arguments
	friend tlSrGslMatrix operator+ (
		    double scalar,
		    const tlSrGslMatrix &mat
		);

	////////////////////////////////////////
	// return a new matrix which is the
	// sum of the two arguments
	friend tlSrGslMatrix operator+ (
		    const tlSrGslMatrix &mat,
		    double scalar
		);

	////////////////////////////////////////
	// return a new matrix which is the
	// difference of the two arguments
	friend tlSrGslMatrix operator- (
		    const tlSrGslMatrix &mat1,
		    const tlSrGslMatrix &mat2
		);

	////////////////////////////////////////
	// return a new matrix which is the
	// difference of the two arguments
	friend tlSrGslMatrix operator- (
		    double scalar,
		    const tlSrGslMatrix &mat
		);

	////////////////////////////////////////
	// return a new matrix which is the
	// difference of the two arguments
	friend tlSrGslMatrix operator- (
		    const tlSrGslMatrix &mat,
		    double scalar
		);

	////////////////////////////////////////
	// return a new matrix which is the
	// scaled by the scalar value
	friend tlSrGslMatrix operator/ (
		    const tlSrGslMatrix &mat,
		    double scalar
		);

	////////////////////////////////////////
	// return a new matrix which is the
	// scaled by the scalar value
	friend tlSrGslMatrix operator/ (
		    double scalar,
		    const tlSrGslMatrix &mat
		);

	////////////////////////////////////////
	// set a given row, column value in the internal matrix
	void setValue(int row, int column, double value);

	////////////////////////////////////////
	// get a given row, column value in the internal matrix
	double getValue(int row, int column) const;

	////////////////////////////////////////
	// Throw away the old internal representation
	// and set it to be shared with <b>newValue</b>
	void setValue(tlSrGslMatrix newValue);

	////////////////////////////////////////
	// Use the supplied value as the new wrapped representation
	void setValue(tlGslMatrix *newValue);

	////////////////////////////////////////
	// Return a pointer to the internally managed data
	const tlGslMatrix *getValue() const;

	////////////////////////////////////////
	// Return a handle to the inverse, calculating this
	// internally if need be
	const tlSrGslMatrix inverse();

	////////////////////////////////////////
	// Return a handle to the transpose, calculating this
	// internally if need be
	const tlSrGslMatrix transpose();

	////////////////////////////////////////
	// Return the (unsorted) eigenvalues of the current
	// matrix.
	// Note that both the eigenvalues and eigenvectors
	// are computed together, and this function
	// with then return the eigenvalues.  The eigenvectors
	// can then be accessed (at no cost) via the
	// related eigenvectors() function, or vice versa.
	const tlSrGslMatrix eigenvalues();

	////////////////////////////////////////
	// calculate the dot product of ourselves
	// and a friend and return the result
	double dot(const tlSrGslMatrix &other) const;

	////////////////////////////////////////
	// calculate the vector length
	double norm() const;

	////////////////////////////////////////
	// Return the (unsorted) eigenvectors of the current
	// matrix as the columns of the returned matrix.
	//
	// The eigenvectors are in the same order as the
	// corresponding eigenvalues accessed through
	// eigenvalues() function above.
	//
	// Note that both the eigenvalues and eigenvectors
	// are computed together, and this function
	// with then return the eigenvectors.  The eigenvalues
	// can then be accessed (at no cost) via the
	// related eigenvalues() function, or vice versa.
	const tlSrGslMatrix eigenvectors();

	////////////////////////////////////////
	// Calculate the determinant if required, and return
	// the result
	double determinant();

	////////////////////////////////////////
	// Return the values in a string
	tlSrString getValueString(
					int breakLines = 0,
				int matlabLineContinue = 0
		    ) const;

	////////////////////////////////////////
	// return the number of rows
	int getNumRows() const;

	////////////////////////////////////////
	// return the number of columns
	int getNumColumns() const;

	////////////////////////////////////////
	// print out the matrix to stdout
	void dump() const;


protected:
	tlGslMatrix *data_;
	tlGslMatrix *inverse_;
	tlGslMatrix *transpose_;
	tlGslMatrix *eigenValues_;
	tlGslMatrix *eigenVectors_;

	double determinant_;
	int determinantValid_;

	class IndexHelper {
	public:
		int row_;
		class tlSrGslMatrix *mat_;

		////////////////////////////////////////
		// return the actual double stored in the
		// accompanying wrapped matrix.
		double operator[] (int column) const;
	};

	IndexHelper indexHelper_;

public:
	////////////////////////////////////////
	// Statically declared matrix which is
	// guaranteed to be 'nil'.  This is
	// provided for assignment and comparison
	// purposes.
	static tlSrGslMatrix nil;

protected:
	////////////////////////////////////////
	// clean up static refs
	static void sExitCleanup();

public:
	friend class tlRefManager;
	friend class IndexHelper;
};


inline int tlSrGslMatrix::getNumRows() const
{
	return data_->getNumRows();
}

inline int tlSrGslMatrix::getNumColumns() const
{
	return data_->getNumColumns();
}

inline double tlSrGslMatrix::getValue(int i, int j) const
{
	if (data_ != NULL) {
		return data_->getValue(i, j);
	}
	return 0;
}

inline void tlSrGslMatrix::setValue(int i, int j, double v)
{
	if (data_ != NULL) {
		data_->setValue(i, j, v);
	}
}

inline const tlGslMatrix *tlSrGslMatrix::getValue() const
{
	return data_;
}

inline void tlSrGslMatrix::setValue(tlSrGslMatrix s)
{
	if (data_ != NULL) {
		data_->unref();
	}
	*this = s;
}

inline void tlSrGslMatrix::flood(double v)
{
	if (data_ != NULL) {
		data_->flood(v);
	}
}

inline tlSrString tlSrGslMatrix::getValueString(
		int breakLines,
		int matlabFormatContinue
	) const
{
	if (data_ == NULL) {
		tlSrString s;
		s = "[]";
		return s;
	}

	return data_->getValueString(breakLines, matlabFormatContinue);
}

inline tlSrGslMatrix::IndexHelper &tlSrGslMatrix::operator[] (int row) {
	indexHelper_.row_ = row;
	return indexHelper_;
}

inline double tlSrGslMatrix::IndexHelper::operator[] (int column) const {
	return (*(mat_->data_))[row_][column];
}

inline void tlSrGslMatrix::dump() const
{
	if (data_ != NULL) {
		data_->dump();
	}
}

#endif /* __GSL_TOOL_MATRIX_HANDLE_HEADER__ */
#endif /* GSL_LIBRARY_INSTALLED */

