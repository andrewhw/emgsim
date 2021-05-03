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
 * $Id: tlSrGslMatrix.cpp 57 2013-12-13 21:33:01Z andrew $
 */


#ifdef		GSL_LIBRARY_INSTALLED
#ifndef MAKEDEPEND
# include <stdlib.h>
# include <string.h>
# include <stdarg.h>
# include <ctype.h>
#endif

#include "tclCkalloc.h"
#include "listalloc.h"

#include "tlSrGslMatrix.h"


#define UNREFANDCLEAR(x)		do { \
				if ((x) != NULL) { \
				    (x)->unref(); \
				} \
				(x) = NULL; \
		    } while (0)

#define ASSIGNANDREF(x, n)		do { \
				if ((x) != NULL) { \
				    (x)->unref(); \
				} \
				(x) = n; \
				if ((x) != NULL) { \
				    (x)->ref(); \
				} \
		    } while (0)



/** access for static global "nil" matrix */
tlSrGslMatrix tlSrGslMatrix::nil(0);

void
tlSrGslMatrix::sExitCleanup()
{
	tlSrGslMatrix::nil.data_->unref();
	tlSrGslMatrix::nil.data_ = NULL;

	UNREFANDCLEAR(tlSrGslMatrix::nil.inverse_);
	UNREFANDCLEAR(tlSrGslMatrix::nil.transpose_);
	UNREFANDCLEAR(tlSrGslMatrix::nil.eigenValues_);
	UNREFANDCLEAR(tlSrGslMatrix::nil.eigenVectors_);
}

tlSrGslMatrix::tlSrGslMatrix(tlGslMatrix *m)
{
	data_ = m;
	if (data_ != NULL)
	{
		data_->ref();
	}

	inverse_ = NULL;
	indexHelper_.mat_ = this;

	determinant_ = (-1);
	determinantValid_ = 0;

	transpose_ = NULL;
	eigenValues_ = NULL;
	eigenVectors_ = NULL;
}

tlSrGslMatrix::tlSrGslMatrix(const tlSrGslMatrix &sibling)
{
	if (sibling.data_ != NULL)
		sibling.data_->ref();
	data_ = sibling.data_;

	if (sibling.inverse_ != NULL)
		sibling.inverse_->ref();
	inverse_ = sibling.inverse_;

	indexHelper_.mat_ = this;

	determinant_ = sibling.determinant_;
	determinantValid_ = sibling.determinantValid_;

	if (sibling.transpose_ != NULL)
		sibling.transpose_->ref();
	transpose_ = sibling.transpose_;;

	if (sibling.eigenValues_ != NULL)
		sibling.eigenValues_->ref();
	eigenValues_ = sibling.eigenValues_;;

	if (sibling.eigenVectors_ != NULL)
		sibling.eigenVectors_->ref();
	eigenVectors_ = sibling.eigenVectors_;;
}

tlSrGslMatrix::tlSrGslMatrix(int /* dummy */)
{
	/** we are the nil instance, initialize */
	MSG_ASSERT(this == &nil, "we are not the nil instance");
	data_ = new tlGslMatrix(1, 1);
	data_->ref();

	inverse_ = NULL;

	determinant_ = (-1);
	determinantValid_ = 0;

	transpose_ = NULL;
	eigenValues_ = NULL;
	eigenVectors_ = NULL;

	indexHelper_.mat_ = this;
}

tlSrGslMatrix::tlSrGslMatrix()
{
	nil.data_->ref();
	data_ = nil.data_;

	inverse_ = NULL;

	determinant_ = (-1);
	determinantValid_ = 0;

	transpose_ = NULL;
	eigenValues_ = NULL;
	eigenVectors_ = NULL;

	indexHelper_.mat_ = this;
}

tlSrGslMatrix::tlSrGslMatrix(const tlTuple *tuple)
{
	data_ = new tlGslMatrix(tuple);
	data_->ref();

	inverse_ = NULL;

	determinant_ = (-1);
	determinantValid_ = 0;

	transpose_ = NULL;
	eigenValues_ = NULL;
	eigenVectors_ = NULL;

	indexHelper_.mat_ = this;
}

tlSrGslMatrix::tlSrGslMatrix(int nRows, int nColumns, double value)
{
	data_ = new tlGslMatrix(nRows, nColumns);
	data_->ref();

	data_->flood(value);

	inverse_ = NULL;

	determinant_ = (-1);
	determinantValid_ = 0;

	transpose_ = NULL;
	eigenValues_ = NULL;
	eigenVectors_ = NULL;

	indexHelper_.mat_ = this;
}

tlSrGslMatrix::~tlSrGslMatrix()
{
	if (data_ != NULL)
	{
		data_->unref();
	}
	if (inverse_ != NULL)
	{
		inverse_->unref();
	}
	if (transpose_ != NULL)
	{
		transpose_->unref();
	}
	if (eigenValues_ != NULL)
	{
		eigenValues_->unref();
	}
	if (eigenVectors_ != NULL)
	{
		eigenVectors_->unref();
	}
}

void
tlSrGslMatrix::setValues(tlTable *src)
{
	if (data_ != NULL)
	{
		data_->unref();
	}
	data_ = new tlGslMatrix(src);
	data_->ref();

	determinant_ = (-1);
	determinantValid_ = 0;

	UNREFANDCLEAR(inverse_);
	UNREFANDCLEAR(transpose_);
	UNREFANDCLEAR(eigenValues_);
	UNREFANDCLEAR(eigenVectors_);
}

void
tlSrGslMatrix::setValues(tlTuple *src)
{
	if (data_ != NULL)
	{
		data_->unref();
	}
	data_ = new tlGslMatrix(src);

	determinant_ = (-1);
	determinantValid_ = 0;

	UNREFANDCLEAR(inverse_);
	UNREFANDCLEAR(transpose_);
	UNREFANDCLEAR(eigenValues_);
	UNREFANDCLEAR(eigenVectors_);
}

tlSrGslMatrix
tlSrGslMatrix::getSubGslMatrix(
		int rowBegin, int columnBegin,
		int rowEnd, int columnEnd
    )
{
	int nRows, nColumns;
	int i, j;

	nRows = rowEnd - rowBegin;
	nColumns = columnEnd - columnBegin;

	MSG_ASSERT(nRows > 0 && nColumns > 0, "submatrix with no rows");

	{
		tlSrGslMatrix result(nRows, nColumns);

		/** select the data from the supplied matrix */
		for (i = rowBegin; i < rowEnd; i++)
		{
			for (j = columnBegin; j < columnEnd; j++)
			{
				result.setValue(i - rowBegin,
								j - columnBegin,
								data_->getValue(i, j));
			}
		}

		return result;
	}
}

tlSrGslMatrix &
tlSrGslMatrix::operator= (const tlSrGslMatrix &sibling)
{
	if (sibling.data_ != NULL)
		sibling.data_->ref();

	if (data_ != NULL)
	{
		data_->unref();
	}
	data_ = sibling.data_;




	determinant_ = sibling.determinant_;
	determinantValid_ = sibling.determinantValid_;

	ASSIGNANDREF(inverse_, sibling.inverse_);
	ASSIGNANDREF(transpose_, sibling.transpose_);
	ASSIGNANDREF(eigenValues_, sibling.eigenValues_);
	ASSIGNANDREF(eigenVectors_, sibling.eigenVectors_);

	return *this;
}


tlSrGslMatrix &
tlSrGslMatrix::operator= (const tlTuple *src)
{
	if (data_ != NULL)
	{
		data_->unref();
	}
	data_ = new tlGslMatrix( src );
	data_->ref();

	determinant_ = (-1);
	determinantValid_ = 0;

	UNREFANDCLEAR(inverse_);
	UNREFANDCLEAR(transpose_);
	UNREFANDCLEAR(eigenValues_);
	UNREFANDCLEAR(eigenVectors_);

	return *this;
}

int
operator== (
		const tlSrGslMatrix &mat1,
		const tlSrGslMatrix &mat2
    )
{
	if (mat1.data_->equals(mat2.data_))
		return 1;

	return 0;
}

int
operator!= (
		const tlSrGslMatrix &mat1,
		const tlSrGslMatrix &mat2
    )
{
	if (mat1.data_->equals(mat2.data_))
		return 0;
	return 1;
}

tlSrGslMatrix
operator* (
		const tlSrGslMatrix &mat1,
		const tlSrGslMatrix &mat2
    )
{
	tlSrGslMatrix result;
	tlGslMatrix *product;

	product = mat1.data_->product(mat2.data_);

	result.setValue(product);
	return result;
}

tlSrGslMatrix
operator* (
		double scalar,
		const tlSrGslMatrix &mat
    )
{
	tlSrGslMatrix result;
	tlGslMatrix *product;

	product = mat.data_->scalarProduct(scalar);

	result.setValue(product);
	return result;
}

tlSrGslMatrix
operator* (
		const tlSrGslMatrix &mat,
		double scalar
    )
{
	tlSrGslMatrix result;
	tlGslMatrix *product;

	product = mat.data_->scalarProduct(scalar);

	result.setValue(product);
	return result;
}

tlSrGslMatrix
operator+ (
		const tlSrGslMatrix &mat1,
		const tlSrGslMatrix &mat2
    )
{
	tlSrGslMatrix result;
	tlGslMatrix *sum;

	sum = mat1.data_->sum(mat2.data_);

	result.setValue(sum);
	return result;
}

tlSrGslMatrix
operator+ (
		double scalar,
		const tlSrGslMatrix &mat
    )
{
	tlSrGslMatrix result;
	tlGslMatrix *sum;

	sum = mat.data_->scalarSum(scalar);

	result.setValue(sum);
	return result;
}


tlSrGslMatrix
operator+ (
		const tlSrGslMatrix &mat,
		double scalar
    )
{
	tlSrGslMatrix result;
	tlGslMatrix *sum;

	sum = mat.data_->scalarSum(scalar);

	result.setValue(sum);
	return result;
}

tlSrGslMatrix
operator- (
		const tlSrGslMatrix &mat1,
		const tlSrGslMatrix &mat2
    )
{
	tlSrGslMatrix result;
	tlGslMatrix *sum;

	sum = mat1.data_->difference(mat2.data_);

	result.setValue(sum);
	return result;
}

tlSrGslMatrix
operator- (
		double scalar,
		const tlSrGslMatrix &mat
    )
{
	tlSrGslMatrix result;
	tlGslMatrix *sum;

	sum = mat.data_->scalarMinusGslMatrix(scalar);

	result.setValue(sum);
	return result;
}


tlSrGslMatrix
operator- (
		const tlSrGslMatrix &mat,
		double scalar
    )
{
	tlSrGslMatrix result;
	tlGslMatrix *sum;

	sum = mat.data_->matrixMinusScalar(scalar);

	result.setValue(sum);
	return result;
}

tlSrGslMatrix
operator/ (
		double scalar,
		const tlSrGslMatrix &mat
    )
{
	tlSrGslMatrix result;
	tlGslMatrix *dividend;

	dividend = mat.data_->scalarDividedByGslMatrix(scalar);

	result.setValue(dividend);
	return result;
}


tlSrGslMatrix
operator/ (
		const tlSrGslMatrix &mat,
		double scalar
    )
{
	tlSrGslMatrix result;
	tlGslMatrix *dividend;

	dividend = mat.data_->matrixDividedByScalar(scalar);

	result.setValue(dividend);
	return result;
}

void
tlSrGslMatrix::setValue(tlGslMatrix *matrix)
{
	if (data_ != NULL)
	{
		data_->unref();
	}

	data_ = matrix;
	if (data_ != NULL)
	{
		data_->ref();
	}

	determinant_ = (-1);
	determinantValid_ = 0;

	UNREFANDCLEAR(inverse_);
	UNREFANDCLEAR(transpose_);
	UNREFANDCLEAR(eigenValues_);
	UNREFANDCLEAR(eigenVectors_);
}

void
tlSrGslMatrix::setSize(int nRows, int nColumns, double value)
{
	if (data_ != NULL)
	{
		data_->unref();
	}

	data_ = new tlGslMatrix(nRows, nColumns);
	data_->ref();

	data_->flood(value);

	determinant_ = (-1);
	determinantValid_ = 0;

	UNREFANDCLEAR(inverse_);
	UNREFANDCLEAR(transpose_);
	UNREFANDCLEAR(eigenValues_);
	UNREFANDCLEAR(eigenVectors_);
}

const tlSrGslMatrix
tlSrGslMatrix::inverse()
{
	tlSrGslMatrix result;

	if (inverse_ == NULL)
	{
		inverse_ = data_->inverse();
		if (inverse_ != NULL)
		{
			inverse_->ref();
		}
	}

	result.inverse_ = data_;
	result.inverse_->ref();

	if (inverse_ != NULL)
	{
		result.data_ = inverse_;
		result.data_->ref();
	} else
	{
		result.data_ = NULL;
	}

	return result;
}


const tlSrGslMatrix
tlSrGslMatrix::transpose()
{
	tlSrGslMatrix result;

	if (transpose_ == NULL)
	{
		transpose_ = data_->transpose();
		transpose_->ref();
	}

	result.setValue( transpose_ );
	result.transpose_ = data_;
	result.transpose_->ref();

	return result;
}


const tlSrGslMatrix
tlSrGslMatrix::eigenvalues()
{
	int status;
	tlSrGslMatrix result;

	if (eigenValues_ == NULL)
	{

		MSG_ASSERT(data_->getNumRows() == data_->getNumColumns(),
				"Eigen calculations need square (symmetric) matrix");

		eigenValues_ = new tlGslMatrix(1, data_->getNumRows());
		eigenVectors_ = new tlGslMatrix(
						data_->getNumRows(), data_->getNumColumns());

		status = data_->eigenCalculations(eigenValues_, eigenVectors_);
		MSG_ASSERT(status, "eigen calculations failed");

		eigenValues_->ref();
		eigenVectors_->ref();
	}

	result.setValue( eigenValues_ );

	return result;
}

const tlSrGslMatrix
tlSrGslMatrix::eigenvectors()
{
	int status;
	tlSrGslMatrix result;

	if (eigenValues_ == NULL)
	{

		MSG_ASSERT(data_->getNumRows() == data_->getNumColumns(),
				"Eigen calculations need square (symmetric) matrix");

		eigenValues_ = new tlGslMatrix(1, data_->getNumRows());
		eigenVectors_ = new tlGslMatrix(
						data_->getNumRows(), data_->getNumColumns());

		status = data_->eigenCalculations(eigenValues_, eigenVectors_);
		MSG_ASSERT(status, "eigen calculations failed");

		eigenValues_->ref();
		eigenVectors_->ref();
	}

	result.setValue( eigenVectors_ );

	return result;
}

double tlSrGslMatrix::determinant()
{
	if ((! determinantValid_) && (data_ != NULL))
	{
		determinant_ = data_->determinant();
	}
	return determinant_;
}

double
tlSrGslMatrix::norm() const
{
	if (data_ != NULL)
		return data_->norm();

	return -1.0;
}

double
tlSrGslMatrix::dot(const tlSrGslMatrix &other) const
{
	if (data_ != NULL)
		return data_->dot(other.data_);

	return 0.0;
}


#endif /* GSL_LIBRARY_INSTALLED */

