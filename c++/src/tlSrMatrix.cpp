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
 * $Id: tlSrMatrix.cpp 57 2013-12-13 21:33:01Z andrew $
 */


#ifndef MAKEDEPEND
# include <stdlib.h>
# include <string.h>
# include <stdarg.h>
# include <ctype.h>
#endif

#include "os_defs.h"
#include "tclCkalloc.h"
#include "listalloc.h"

#include "tlSrMatrix.h"

/** access for static global "nil" matrix */
tlSrMatrix tlSrMatrix::nil(0);

void
tlSrMatrix::sExitCleanup()
{
#ifdef		OS_WINDOWS_NT
# ifdef OS_WINDOWS_DELETE_NIL
	if (tlSrMatrix::nil.data_ != NULL)
	{
		/**
		 * This is protected as on MFC programs it is difficult
		 * to determine whether or not sExitCleanup may be
		 * being called _after_ the nil cleanup that
		 * occurs statically may already be done.  See also
		 * the "set to null" in tlSrString's destructor that
		 * is only required to maintain this if statement.
		 */
		tlSrMatrix::nil.data_->unref();
		tlSrMatrix::nil.data_ = NULL;
	}
# endif
#else
	tlSrMatrix::nil.data_->unref();
	tlSrMatrix::nil.data_ = NULL;
#endif

	if (tlSrMatrix::nil.inverse_ != NULL)
	{
		tlSrMatrix::nil.inverse_->unref();
		tlSrMatrix::nil.inverse_ = NULL;
	}
	if (tlSrMatrix::nil.transpose_ != NULL)
	{
		tlSrMatrix::nil.transpose_->unref();
		tlSrMatrix::nil.transpose_ = NULL;
	}
}

tlSrMatrix::tlSrMatrix(tlMatrix *m)
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
}

tlSrMatrix::tlSrMatrix(const tlSrMatrix &sibling)
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

	transpose_ = sibling.transpose_;;
}

tlSrMatrix::tlSrMatrix(int /* dummy */)
{
	/** we are the nil instance, initialize */
	MSG_ASSERT(this == &nil, "we are not the nil instance");
	data_ = new tlMatrix(0, 0);
	data_->ref();

	inverse_ = NULL;

	determinant_ = (-1);
	determinantValid_ = 0;

	transpose_ = NULL;

	indexHelper_.mat_ = this;
}

tlSrMatrix::tlSrMatrix()
{
	nil.data_->ref();
	data_ = nil.data_;

	inverse_ = NULL;

	determinant_ = (-1);
	determinantValid_ = 0;

	transpose_ = NULL;

	indexHelper_.mat_ = this;
}

tlSrMatrix::tlSrMatrix(const tlTuple *tuple)
{
	data_ = new tlMatrix(tuple);
	data_->ref();

	inverse_ = NULL;

	determinant_ = (-1);
	determinantValid_ = 0;

	transpose_ = NULL;

	indexHelper_.mat_ = this;
}

tlSrMatrix::tlSrMatrix(int nRows, int nColumns, double value)
{
    data_ = new tlMatrix(nRows, nColumns);
    data_->ref();

    data_->flood(value);

    inverse_ = NULL;

    determinant_ = (-1);
    determinantValid_ = 0;

    transpose_ = NULL;

    indexHelper_.mat_ = this;
}

tlSrMatrix::~tlSrMatrix()
{
    if (data_ != NULL)
    {
		data_->unref();
#ifdef OS_WINDOWS_NT
		data_ = NULL;
#endif
    }
    if (inverse_ != NULL)
    {
		inverse_->unref();
		inverse_ = NULL;
    }
    if (transpose_ != NULL)
    {
		transpose_->unref();
		transpose_ = NULL;
    }
}

void
tlSrMatrix::setValues(tlTable *src)
{
    if (data_ != NULL)
    {
		data_->unref();
    }
    data_ = new tlMatrix(src);
    data_->ref();

    if (inverse_ != NULL)
    {
		inverse_->unref();
    }
    inverse_ = NULL;

    determinant_ = (-1);
    determinantValid_ = 0;

    transpose_ = NULL;
}

void
tlSrMatrix::setValues(tlTuple *src)
{
    if (data_ != NULL)
    {
		data_->unref();
    }
    data_ = new tlMatrix(src);

    if (inverse_ != NULL)
    {
		inverse_->unref();
    }
    inverse_ = NULL;

    determinant_ = (-1);
    determinantValid_ = 0;

    transpose_ = NULL;
}

tlSrMatrix
tlSrMatrix::getSubMatrix(
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
		tlSrMatrix result(nRows, nColumns);

		/** select the data from the supplied matrix */
		for (i = rowBegin; i < rowEnd; i++)
		{
		    for (j = columnBegin; j < columnEnd; j++)
		    {
				result[i - rowBegin][j - columnBegin] = data_->getValue(i, j);
		    }
		}

		return result;
    }
}

tlSrMatrix &
tlSrMatrix::operator= (const tlSrMatrix &sibling)
{
    if (sibling.data_ != NULL)
		sibling.data_->ref();

    if (data_ != NULL)
    {
		data_->unref();
    }
    data_ = sibling.data_;

    if (sibling.inverse_ != NULL)
		sibling.inverse_->ref();

    if (inverse_ != NULL)
    {
		inverse_->unref();
    }
    inverse_ = sibling.inverse_;

    determinant_ = sibling.determinant_;
    determinantValid_ = sibling.determinantValid_;

    transpose_ = sibling.transpose_;

    return *this;
}


tlSrMatrix &
tlSrMatrix::operator= (const tlTuple *src)
{
    if (data_ != NULL)
    {
		data_->unref();
    }
    data_ = new tlMatrix( src );
    data_->ref();

    if (inverse_ != NULL)
    {
		inverse_->unref();
    }
    inverse_ = NULL;

    determinant_ = (-1);
    determinantValid_ = 0;

    transpose_ = NULL;

    return *this;
}

int
operator== (
		const tlSrMatrix &mat1,
		const tlSrMatrix &mat2
    )
{
    if (mat1.data_->equals(mat2.data_))
		return 1;

    return 0;
}

int
operator!= (
		const tlSrMatrix &mat1,
		const tlSrMatrix &mat2
    )
{
    if (mat1.data_->equals(mat2.data_))
		return 0;
    return 1;
}

tlSrMatrix
operator* (
		const tlSrMatrix &mat1,
		const tlSrMatrix &mat2
    )
{
    tlSrMatrix result;
    tlMatrix *product;

    product = mat1.data_->product(mat2.data_);

    result.setValue(product);
    return result;
}

tlSrMatrix
operator* (
		double scalar,
		const tlSrMatrix &mat
    )
{
    tlSrMatrix result;
    tlMatrix *product;

    product = mat.data_->scalarProduct(scalar);

    result.setValue(product);
    return result;
}

tlSrMatrix
operator* (
		const tlSrMatrix &mat,
		double scalar
    )
{
    tlSrMatrix result;
    tlMatrix *product;

    product = mat.data_->scalarProduct(scalar);

    result.setValue(product);
    return result;
}

tlSrMatrix
operator+ (
		const tlSrMatrix &mat1,
		const tlSrMatrix &mat2
    )
{
    tlSrMatrix result;
    tlMatrix *sum;

    sum = mat1.data_->sum(mat2.data_);

    result.setValue(sum);
    return result;
}

tlSrMatrix
operator+ (
		double scalar,
		const tlSrMatrix &mat
    )
{
    tlSrMatrix result;
    tlMatrix *sum;

    sum = mat.data_->scalarSum(scalar);

    result.setValue(sum);
    return result;
}


tlSrMatrix
operator+ (
		const tlSrMatrix &mat,
		double scalar
    )
{
    tlSrMatrix result;
    tlMatrix *sum;

    sum = mat.data_->scalarSum(scalar);

    result.setValue(sum);
    return result;
}

tlSrMatrix
operator- (
		const tlSrMatrix &mat1,
		const tlSrMatrix &mat2
    )
{
    tlSrMatrix result;
    tlMatrix *sum;

    sum = mat1.data_->difference(mat2.data_);

    result.setValue(sum);
    return result;
}

tlSrMatrix
operator- (
		double scalar,
		const tlSrMatrix &mat
    )
{
    tlSrMatrix result;
    tlMatrix *sum;

    sum = mat.data_->scalarMinusMatrix(scalar);

    result.setValue(sum);
    return result;
}


tlSrMatrix
operator- (
		const tlSrMatrix &mat,
		double scalar
    )
{
    tlSrMatrix result;
    tlMatrix *sum;

    sum = mat.data_->matrixMinusScalar(scalar);

    result.setValue(sum);
    return result;
}

tlSrMatrix
operator/ (
		double scalar,
		const tlSrMatrix &mat
    )
{
    tlSrMatrix result;
    tlMatrix *dividend;

    dividend = mat.data_->scalarDividedByMatrix(scalar);

    result.setValue(dividend);
    return result;
}


tlSrMatrix
operator/ (
		const tlSrMatrix &mat,
		double scalar
    )
{
    tlSrMatrix result;
    tlMatrix *dividend;

    dividend = mat.data_->matrixDividedByScalar(scalar);

    result.setValue(dividend);
    return result;
}

void
tlSrMatrix::setValue(tlMatrix *matrix)
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

    if (inverse_ != NULL)
    {
		inverse_->unref();
		inverse_ = NULL;
    }

    determinant_ = (-1);
    determinantValid_ = 0;

    transpose_ = NULL;
}

void
tlSrMatrix::setSize(int nRows, int nColumns, double value)
{
    if (data_ != NULL)
    {
		data_->unref();
    }

    data_ = new tlMatrix(nRows, nColumns);
    data_->ref();

    data_->flood(value);

    if (inverse_ != NULL)
    {
		inverse_->unref();
		inverse_ = NULL;
    }

    determinant_ = (-1);
    determinantValid_ = 0;

    transpose_ = NULL;
}

const tlSrMatrix
tlSrMatrix::inverse()
{
    tlSrMatrix result;

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


const tlSrMatrix
tlSrMatrix::transpose()
{
    tlSrMatrix result;

    if (transpose_ == NULL)
    {
		transpose_ = data_->transpose();
		transpose_->ref();
    }

    result.setValue( transpose_ );

    return result;
}

double tlSrMatrix::determinant()
{
    if ((! determinantValid_) && (data_ != NULL))
    {
		determinant_ = data_->determinant();
    }
    return determinant_;
}

