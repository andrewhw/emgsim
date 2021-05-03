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
 * $Id: tlMatrix.h 57 2013-12-13 21:33:01Z andrew $
 */


#ifndef		__TOOL_MATRIX_HEADER__
#define		__TOOL_MATRIX_HEADER__

#include "os_defs.h"

#include "tlRef.h"
#include "tlSrString.h"

/** friend for non-reffed handle wrapper */
class tlMatrix;
class tlTuple;
class tlTable;

class tlMatrixIndexHelper {
public:
	int row_;
	class tlMatrix *mat_;

	////////////////////////////////////////
	// return the actual double stored in the
	// accompanying matrix.
	double &operator[] (int column) const;
};


/**
CLASS
		tlMatrix

	A matrix class with inverse, multiply and transpose.  Used
	in calculation tools to produce covariance and MICD classifiers.
*/
class OS_EXPORT tlMatrix : public tlRef
{
public:
	////////////////////////////////////////
	// Constructor
	tlMatrix();

	////////////////////////////////////////
	// Constructor
	tlMatrix(int nRows, int nCols);

	////////////////////////////////////////
	// Copy Constructor
	tlMatrix(tlMatrix &copy);

	////////////////////////////////////////
	// Copy Constructor
	tlMatrix(tlMatrix *copy);

	////////////////////////////////////////
	// Construct using a tuple (one column will be produced)
	tlMatrix(const tlTuple *src);

	////////////////////////////////////////
	// Construct using a table
	tlMatrix(const tlTable *src);

protected:
	~tlMatrix();

	////////////////////////////////////////
	// Return the typename for
	// reference debugging
	const char *clsId() const;

public:

	////////////////////////////////////////
	// return the number of rows
	int getNumRows() const;

	////////////////////////////////////////
	// return the number of columns
	int getNumColumns() const;

	////////////////////////////////////////
	// return the value at [ <b>rowIndex</b> <b>colIndex</b> ]
	double getValue(int rowIndex, int colIndex) const;

	////////////////////////////////////////
	// Return the values in a string
	tlSrString getValueString(int breakLines = 0) const;

	////////////////////////////////////////
	// Set the value at [ <b>rowIndex</b> <b>colIndex</b> ]
	void setValue(int rowIndex, int colIndex, double value);

	////////////////////////////////////////
	// Set the values using a string.  Returns -1 on failure
	int setValues(tlSrString valueString);

	////////////////////////////////////////
	// Set the values using a string
	int setValues(const char *valueString);

	////////////////////////////////////////
	// set all locations with the supplied value
	void flood(double value);

	////////////////////////////////////////
	// set this matrix to be an identity matrix.
	// This call fails if the matrix is not square
	int eye();

	////////////////////////////////////////
	// Return the determinant of this matrix
	double determinant();

	////////////////////////////////////////
	// Produce the product of this matrix and
	// the referenced matrix as a new object
	tlMatrix *product(tlMatrix *operand) const;

	////////////////////////////////////////
	// Produce the sum of this matrix and
	// the referenced matrix as a new object
	tlMatrix *sum(const tlMatrix *operand) const; 

	////////////////////////////////////////
	// Produce the difference of this matrix and
	// the referenced matrix as a new object
	tlMatrix *difference(const tlMatrix *operand) const; 

	////////////////////////////////////////
	// Multiply all values in the current
	// matrix by <i>scalar</i> and return
	// the resulting matrix as a new object
	tlMatrix *scalarProduct(double scalar) const; 

	////////////////////////////////////////
	// Add <i>scalar</i> to all values in the
	// current matrix and return
	// the resulting matrix as a new object
	tlMatrix *scalarSum(double scalar) const; 

	////////////////////////////////////////
	// Subtract <i>scalar</i> from all values in the
	// current matrix and return
	// the resulting matrix as a new object
	tlMatrix *matrixMinusScalar(double scalar) const; 

	////////////////////////////////////////
	// Subtract matrix values from <i>scalar</i> 
	// and return
	// the resulting matrix as a new object
	tlMatrix *scalarMinusMatrix(double scalar) const; 

	////////////////////////////////////////
	// Divide <i>scalar</i> to all values in the
	// current matrix and return
	// the resulting matrix as a new object
	tlMatrix *matrixDividedByScalar(double scalar) const; 

	////////////////////////////////////////
	// Divide <i>scalar</i> to all values in the
	// current matrix and return
	// the resulting matrix as a new object
	tlMatrix *scalarDividedByMatrix(double scalar) const; 

	////////////////////////////////////////
	// Produce the inverse of this matrix as
	// a new object
	tlMatrix *inverse();

	////////////////////////////////////////
	// Produce the transpose of this matrix as
	// a new object
	tlMatrix *transpose() const;

	////////////////////////////////////////
	// Return the truth value of equality between
	// this matrix and a friend
	int equals(tlMatrix *operand) const;

public:
	////////////////////////////////////////
	// bracket operator.  Note use of tlMatrixIndexHelper class.
	tlMatrixIndexHelper &operator[] (int row);

protected:

	void allocate_(int nRows, int nCols);
	void init_();

	int nRows_;
	int nCols_;
	double **value_;
	double *pivot_;

	tlMatrixIndexHelper indexHelper_;

	friend class tlMatrixIndexHelper;
};

inline int tlMatrix::getNumRows() const
{
	return nRows_;
}

inline int tlMatrix::getNumColumns() const
{
	return nCols_;
}

inline void tlMatrix::setValue(int i, int j, double value)
{
	value_[i][j] = value;
}

#endif /* __TOOL_MATRIX_HEADER__ */

