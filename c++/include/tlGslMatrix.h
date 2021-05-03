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
 * $Id: tlGslMatrix.h 57 2013-12-13 21:33:01Z andrew $
 */


#ifdef		GSL_LIBRARY_INSTALLED
#ifndef		__GSL_TOOL_MATRIX_HEADER__
#define		__GSL_TOOL_MATRIX_HEADER__

#include "os_defs.h"

#include "tlRef.h"
#include "tlSrString.h"

#include <gsl/gsl_matrix.h>

/** friend for non-reffed handle wrapper */
class tlGslMatrix;
class tlTuple;
class tlTable;

class tlGslMatrixIndexHelper {
public:
	int row_;
	class tlGslMatrix *m_;

	////////////////////////////////////////
	// return the actual double stored in the
	// accompanying matrix.
	double operator[] (int column) const;
};


/**
CLASS
		tlGslMatrix

	A matrix class with inverse, multiply and transpose.  Used
	in calculation tools to produce covariance and MICD classifiers.
*/
class OS_EXPORT tlGslMatrix : public tlRef
{
public:
	////////////////////////////////////////
	// Constructor
	tlGslMatrix();

	////////////////////////////////////////
	// Constructor
	tlGslMatrix(int nRows, int nCols);

	////////////////////////////////////////
	// Copy Constructor
	tlGslMatrix(tlGslMatrix &copy);

	////////////////////////////////////////
	// Copy Constructor
	tlGslMatrix(tlGslMatrix *copy);

	////////////////////////////////////////
	// Construct using a tuple (one column will be produced)
	tlGslMatrix(const tlTuple *src);

	////////////////////////////////////////
	// Construct using a table
	tlGslMatrix(const tlTable *src);

	////////////////////////////////////////
	// Construct using a Gsl Matrix
	tlGslMatrix(int nRows, int nCols, gsl_matrix *newMat);

protected:
	~tlGslMatrix();

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
	tlSrString getValueString(
					int breakLines = 0,
				int matlabLineContinue = 0
		    ) const;

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
	tlGslMatrix *product(tlGslMatrix *operand) const;

	////////////////////////////////////////
	// Produce the sum of this matrix and
	// the referenced matrix as a new object
	tlGslMatrix *sum(const tlGslMatrix *operand) const; 

	////////////////////////////////////////
	// Produce the difference of this matrix and
	// the referenced matrix as a new object
	tlGslMatrix *difference(const tlGslMatrix *operand) const; 

	////////////////////////////////////////
	// Multiply all values in the current
	// matrix by <i>scalar</i> and return
	// the resulting matrix as a new object
	tlGslMatrix *scalarProduct(double scalar) const; 

	////////////////////////////////////////
	// Add <i>scalar</i> to all values in the
	// current matrix and return
	// the resulting matrix as a new object
	tlGslMatrix *scalarSum(double scalar) const; 

	////////////////////////////////////////
	// Subtract <i>scalar</i> from all values in the
	// current matrix and return
	// the resulting matrix as a new object
	tlGslMatrix *matrixMinusScalar(double scalar) const; 

	////////////////////////////////////////
	// Subtract matrix values from <i>scalar</i> 
	// and return
	// the resulting matrix as a new object
	tlGslMatrix *scalarMinusGslMatrix(double scalar) const; 

	////////////////////////////////////////
	// Divide <i>scalar</i> to all values in the
	// current matrix and return
	// the resulting matrix as a new object
	tlGslMatrix *matrixDividedByScalar(double scalar) const; 

	////////////////////////////////////////
	// Divide <i>scalar</i> to all values in the
	// current matrix and return
	// the resulting matrix as a new object
	tlGslMatrix *scalarDividedByGslMatrix(double scalar) const; 

	////////////////////////////////////////
	// Produce the inverse of this matrix as
	// a new object
	tlGslMatrix *inverse();

	////////////////////////////////////////
	// Produce the transpose of this matrix as
	// a new object
	tlGslMatrix *transpose() const;

	////////////////////////////////////////
	// produce the dot product of this vector
	double dot(tlGslMatrix const *other) const;

	////////////////////////////////////////
	// calculate the vector length
	double norm() const;

	////////////////////////////////////////
	// Calculate the eigen values/eigen vectors
	// and place in the resulting matrices,
	// which must already be scaled to fit
	int eigenCalculations(
					tlGslMatrix *eigenvalues,
				tlGslMatrix *eigenVectors
		    ) const;

	////////////////////////////////////////
	// Calculate the square root of a matrix 
	const tlGslMatrix* sqrt(tlGslMatrix* m) const;

	////////////////////////////////////////
	// Return the truth value of equality between
	// this matrix and a friend
	int equals(tlGslMatrix *operand) const;

	////////////////////////////////////////
		// figure out the smallest difference between
		// this matrix and another
		float minDifference(tlGslMatrix *B) const;

	////////////////////////////////////////
	// print out the result on stdout
	void dump() const;

public:
	////////////////////////////////////////
	// bracket operator.  Note use of tlGslMatrixIndexHelper class.
	tlGslMatrixIndexHelper &operator[] (int row);

protected:

	void allocate_(int nRows, int nCols);
	void init_();

	int nRows_;
	int nCols_;
	gsl_matrix *mat_;
	double *pivot_;

	tlGslMatrixIndexHelper indexHelper_;

	friend class tlGslMatrixIndexHelper;
};

inline int tlGslMatrix::getNumRows() const
{
	return nRows_;
}

inline int tlGslMatrix::getNumColumns() const
{
	return nCols_;
}

#endif /* __GSL_TOOL_MATRIX_HEADER__ */
#endif /* GSL_LIBRARY_INSTALLED */

