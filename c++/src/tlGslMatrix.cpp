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
 * $Id: tlGslMatrix.cpp 57 2013-12-13 21:33:01Z andrew $
 */

#ifdef		GSL_LIBRARY_INSTALLED
#ifndef MAKEDEPEND
# include <stdio.h>
# include <string.h>
# include <errno.h>
# include <ctype.h>
#endif

#include "tclCkalloc.h"
#include "bitstring.h"
#include "mathtools.h"

#include "tlGslMatrix.h"
#include "tlTuple.h"
#include "tlTable.h"

#include <gsl/gsl_matrix.h>
#include <gsl/gsl_blas.h>
#include <gsl/gsl_linalg.h>
#include <gsl/gsl_eigen.h>

#define		MAT_FLAG		DETERMINANT		0x01


tlGslMatrix::tlGslMatrix()
{
	init_();
}

tlGslMatrix::tlGslMatrix(int nRows, int nCols)
{
	init_();
	allocate_(nRows, nCols);
	gsl_matrix_set_zero(mat_);
}

tlGslMatrix::tlGslMatrix(tlGslMatrix &copy)
{
	int i, j;

	init_();
	allocate_(copy.nRows_, copy.nCols_);
	for (i = 0; i < nRows_; i++)
	{
		for (j = 0; j < nRows_; j++)
		{
			setValue(i, j, copy.getValue(i, j));
		}
	}
}

tlGslMatrix::tlGslMatrix(tlGslMatrix *copy)
{
	int i, j;

	init_();
	allocate_(copy->nRows_, copy->nCols_);
	for (i = 0; i < nRows_; i++)
	{
		for (j = 0; j < nRows_; j++)
		{
			setValue(i, j, copy->getValue(i, j));
		}
	}
}

tlGslMatrix::tlGslMatrix(const tlTuple *src)
{
	int i;

	init_();
	allocate_(src->getNumValues(), 1);
	for (i = 0; i < nRows_; i++)
	{
		setValue(i, 0, src->getValue(i).getRealValue());
	}
}

tlGslMatrix::tlGslMatrix(const tlTable *src)
{
	tlColumn *column;
	int nColumns, nRows;
	int i, j;

	init_();
	nColumns = src->getNumColumns();
	nRows = src->getNumRows();
	allocate_( nRows, nColumns );
	for (j = 0; j < nColumns; j++)
	{
		column = src->getColumn(j);
		for (i = 0; i < nRows; i++)
		{
			setValue(i, j, column->getRealValue(i));
		}
	}
}

tlGslMatrix::tlGslMatrix(int nRows, int nCols, gsl_matrix *src)
{
	init_();

	allocate_(nRows, nCols);
	gsl_matrix_memcpy(mat_, src);
}

void
tlGslMatrix::init_()
{
	nRows_ = 0;
	nCols_ = 0;
	mat_ = NULL;
	indexHelper_.m_ = this;
	pivot_ = NULL;
}

tlGslMatrix::~tlGslMatrix()
{
	gsl_matrix_free(mat_);
	mat_ = NULL;

	if (pivot_ != NULL)
	{
		ckfree(pivot_);
	}
}

const char *
tlGslMatrix::clsId() const
{
	return "tlGslMatrix";
}

void
tlGslMatrix::allocate_(int nRows, int nCols)
{
	nRows_ = nRows;
	nCols_ = nCols;

	if ((nRows_ > 0) && (nCols_ > 0))
	{
		mat_ = gsl_matrix_alloc(nRows_, nCols_);
	} else
	{
		mat_ = NULL;
	}
}

double tlGslMatrix::getValue(int i, int j) const
{
	return gsl_matrix_get(mat_, i, j);
}

void tlGslMatrix::setValue(int i, int j, double value)
{
	gsl_matrix_set(mat_, i, j, value);
}

tlSrString
tlGslMatrix::getValueString(
		int breakLines,
		int matlabLineContinue
	) const
{
	tlSrString s;
	int i, j;

	if (nRows_ == 0)
	{
		s = "[ nil ]";
	} else
	{

		s = "[";
		for (i = 0; i < nRows_; i++)
		{
			for (j = 0; j < nCols_; j++)
			{
				s.sprintf("%S %-9.3g", &s, getValue(i, j));
			}
			if (i < nRows_ - 1)
			{
				s = s + " ;";
				if (breakLines)
				{
					if (matlabLineContinue)
					{
						s = s + " ...\n ";
					} else
					{
						s = s + "\n ";
					}
				}
			} else
			{
				s = s + " ]";
			}
		}
	}
	return s;
}

#define		STATE_BEGIN		0x01
#define		STATE_NUMBER		0x02
#define		STATE_EOL		0x03
#define		STATE_END		0x04
#define		STATE_DONE		0x05

int
tlGslMatrix::setValues(const char *s)
{
	int state = STATE_BEGIN;
	int i, j;
	int c;
	double v;

	i = j = 0;
	while ((c = *s) != 0)
	{
		if (isspace(c))
		{
			s++;
			continue;

		/** check for extra chars after end */
		} else if (state == STATE_DONE)
		{
			return (-1);


		/** check for first char */
		} else if ((state == STATE_BEGIN) && (c == '['))
		{
			state = STATE_NUMBER;


		/** check for last char */
		} else if ((state == STATE_END) && (c == ']'))
		{
			state = STATE_DONE;


		} else if ((state == STATE_EOL) && (c == ';'))
		{
			state = STATE_NUMBER;

		} else if (state == STATE_NUMBER)
		{
			if (sscanf(s, "%lg", &v) != 1)
			{

				return (-1);
			}

			/** assign value */
			setValue(i, j, v);

			/** increment indexes */
			if (++j == nCols_)
			{
				j = 0;
				state = STATE_EOL;
				if (++i == nRows_)
					state = STATE_END;
			}

			/** walk past number */
			while (isdigit(*s)
						|| (*s == 'e') /* exp */
						|| (*s == 'i')
						|| (*s == 'n')
						|| (*s == 'f')
						|| (*s == 't')
						|| (*s == 'y')
						|| (*s == '.')
						|| (*s == '+')
						|| (*s == '-'))
				s++;
			/** back up one */
			s--;

		} else
		{
			/** error -- invalid state! */
			return (-1);
		}
		s++;
	}

	if (state != STATE_DONE)
		return (-1);

	return 0;
}

int
tlGslMatrix::setValues(tlSrString s)
{
	return setValues(s.getValue());
}

void
tlGslMatrix::flood(double value)
{
	gsl_matrix_set_all(mat_, value);
}

int
tlGslMatrix::eye()
{
	if (nRows_ != nCols_)
		return 0;

	gsl_matrix_set_identity(mat_);

	return 1;
}

double
tlGslMatrix::determinant()
{
	gsl_matrix *tmpA;
	gsl_permutation *p;
	int sign;
	double result;


	MSG_ASSERT(nRows_ == nCols_, "matrix must be square");

	if (nRows_ != nCols_ )
		return 0;

	if (nRows_ == 0)
		return 0;

	if (nRows_ == 1)
		return getValue(0, 0);

	if (nRows_ == 2)
	{
		return (getValue(0, 0) * getValue(1, 1))
				- (getValue(0, 1) * getValue(1, 0));
	}


	/** do the real work */
	p = gsl_permutation_alloc(MAX(nRows_, nCols_));

	tmpA = gsl_matrix_alloc(nRows_, nCols_);
	gsl_matrix_memcpy(tmpA, mat_);

	if ( gsl_linalg_LU_decomp(tmpA, p, &sign) != GSL_SUCCESS )
		goto FAIL;


	result = gsl_linalg_LU_det(tmpA, sign);

	gsl_matrix_free(tmpA);
	gsl_permutation_free(p);

	return result;

FAIL:
	gsl_matrix_free(tmpA);
	gsl_permutation_free(p);
	return 0;
}

tlGslMatrix *
tlGslMatrix::inverse()
{
	gsl_matrix *tmpA;
	tlGslMatrix *result;
	gsl_permutation *p;
	int sign;


	MSG_ASSERT(nRows_ == nCols_, "matrix must be square");

	if (nRows_ == 0)
		return 0;

	/** do the real work */
	p = gsl_permutation_alloc(nRows_);

	tmpA = gsl_matrix_alloc(nRows_, nCols_);
	gsl_matrix_memcpy(tmpA, mat_);

	if ( gsl_linalg_LU_decomp(tmpA, p, &sign) != GSL_SUCCESS )
		goto FAIL;


	result = new tlGslMatrix(nRows_, nCols_);

	if ( gsl_linalg_LU_invert(tmpA, p, result->mat_) != GSL_SUCCESS )
		goto FAIL;

	gsl_matrix_free(tmpA);
	gsl_permutation_free(p);

	return result;

FAIL:
	gsl_matrix_free(tmpA);
	gsl_permutation_free(p);
	return 0;
}

tlGslMatrix *
tlGslMatrix::transpose() const
{
	tlGslMatrix *result;
	int i, j;

	result = new tlGslMatrix(nCols_, nRows_);
	for (i = 0; i < nRows_; i++)
	{
		for (j = 0; j < nCols_; j++)
		{
			result->setValue(j, i, getValue(i, j));
		}
	}

	return result;
}

double
tlGslMatrix::dot(tlGslMatrix const *other) const
{
	double result = 0;
	int i;

	MSG_ASSERT((nRows_ == 1 || nCols_ == 1),
					"dot products are only valid for vectors");

	MSG_ASSERT(nRows_ == other->nRows_,
					"row size mismatch in dot calculation");

	MSG_ASSERT(nCols_ == other->nCols_,
					"column size mismatch in dot calculation");


	if (nRows_ == 1)
	{
		/** row vectors */
		for (i = 0; i < nCols_; i++)
		{
			result = result + getValue(0, i) * other->getValue(0, i);
		}
	} else
	{
		/** column vectors */
		for (i = 0; i < nCols_; i++)
		{
			result = result + getValue(i, 0) * other->getValue(i, 0);
		}
	}

	return result;
}


double
tlGslMatrix::norm() const
{
	double d;
	MSG_ASSERT(nRows_ == 1 || nCols_ == 1,
					"norm valid only for vectors");

	d = dot(this);
	return ::sqrt(d);
}

int
tlGslMatrix::eigenCalculations(
		tlGslMatrix *eigenValues,
		tlGslMatrix *eigenVectors
    ) const
{
	gsl_eigen_symmv_workspace *workspace;
	gsl_matrix *copy;
	gsl_vector *tmpEVector;
	int i;
	int status = 0;


	/** we can only work on real, symmetric matrices */
	if (nRows_ != nCols_)
	{
		return 0;
	}

	/**
	 * these calculations destroy the matrix they work on,
	 * so we must first make a copy
	 */
	copy = gsl_matrix_alloc(nRows_, nCols_);
	gsl_matrix_memcpy(copy, mat_);


	/** allocate the eigensystem workspace */
	workspace = gsl_eigen_symmv_alloc (nRows_);

	/** allocate a vector, which we will load into a matrix */
	tmpEVector = gsl_vector_alloc(nRows_);

	if (gsl_eigen_symmv(copy, tmpEVector, eigenVectors->mat_,
					workspace) != GSL_SUCCESS)
	{
		goto CLEANUP;
	}

	/** copy vector into "eigenvector" matrix */
	for (i = 0; i < nRows_; i++)
	{
		gsl_matrix_set(eigenValues->mat_, 0, i,
						gsl_vector_get(tmpEVector, i));
	}

	/** we are done, so clean up and return success */
	status = 1;

CLEANUP:
	gsl_eigen_symmv_free(workspace);
	gsl_matrix_free(copy);
	gsl_vector_free(tmpEVector);

	return status;
}

const tlGslMatrix *
tlGslMatrix::sqrt(tlGslMatrix *M) const
{
	/**
	 * From material at http://yarchive.net/comp/sqrtm.html
	 * moler@mathworks.com (Cleve Moler)
	 *
	 * To calculate the square root of positive semidefinite matrix A:
	 *  
	 *  [ E D ] = eig(A);
	 */

	MSG_ASSERT(0, "Need to rewrite this so we can use LAPACK or similar\n"
			"so we can incorporate Shuh decomposition and/or calculate a\n"
		"matrix square root.  See the discussion of sqrtm() in Matlab\n"
		" and the code at http://seehuhn.de/comp/matrixfn.html");

		/* NOTREACHED */
		return NULL;
}

int
tlGslMatrix::equals(tlGslMatrix *B) const
{
	int i, j;

	for (i = 0; i < nRows_; i++)
	{
		for (j = 0; j < nCols_; j++)
		{

			/** protect against division by zero */
			if (getValue(i, j) != B->getValue(i, j))
			{
				return 0;
			}
		}
	}

	return 1;
}


float
tlGslMatrix::minDifference(tlGslMatrix *B) const
{
	int i, j;
	float cmp, min = FLT_MIN;

	for (i = 0; i < nRows_; i++)
	{
		for (j = 0; j < nCols_; j++)
		{
			cmp = (float) fabs(getValue(i, j) - B->getValue(i, j));
			if (min > cmp)
				min = cmp;
		}
	}

	return min;
}

tlGslMatrix *
tlGslMatrix::product(tlGslMatrix *B) const
{
	tlGslMatrix *result;
	gsl_matrix *gslTemp;
	tlGslMatrix const *A;
	int status;

	A = this;

	/** if sizes are not right, we cannot continue */
	if (A->nCols_ != B->nRows_)
		return NULL;


	gslTemp = gsl_matrix_alloc(A->nRows_, B->nCols_);

	status = gsl_blas_dgemm(
					CblasNoTrans,
				CblasNoTrans,
				1.0,
				A->mat_,
				B->mat_,
				0.0,
				gslTemp);
	MSG_ASSERT(status == GSL_SUCCESS, "matrix product failed");


	/** make a new matrix with the correct final size */
	result = new tlGslMatrix(A->nRows_, B->nCols_, gslTemp);

	gsl_matrix_free(gslTemp);

	return result;
}

tlGslMatrix *
tlGslMatrix::sum(const tlGslMatrix *B) const
{
	tlGslMatrix *result;
	tlGslMatrix const *A;
	int i, j;

	A = this;

	/** if sizes are not right, we cannot continue */
	if (A->nCols_ != B->nCols_)
		return NULL;

	if (A->nRows_ != B->nRows_)
		return NULL;


	/** make a new matrix with the correct final size */
	result = new tlGslMatrix(A->nRows_, A->nCols_);


	/** for each value, calculate the sum from both sources */
	for (i = 0; i < A->nRows_; i++)
	{
		for (j = 0; j < A->nCols_; j++)
		{
			result->setValue(i, j, A->getValue(i, j) + B->getValue(i, j));
		}
	}

	return result;
}

tlGslMatrix *
tlGslMatrix::difference(const tlGslMatrix *B) const
{
	tlGslMatrix *result;
	tlGslMatrix const *A;
	int i, j;

	A = this;

	/** if sizes are not right, we cannot continue */
	if (A->nCols_ != B->nCols_)
		return NULL;

	if (A->nRows_ != B->nRows_)
		return NULL;


	/** make a new matrix with the correct final size */
	result = new tlGslMatrix(A->nRows_, A->nCols_);


	/** for each value, calculate the sum from both sources */
	for (i = 0; i < A->nRows_; i++)
	{
		for (j = 0; j < A->nCols_; j++)
		{
			result->setValue(i, j, A->getValue(i, j) - B->getValue(i, j));
		}
	}

	return result;
}

tlGslMatrix *
tlGslMatrix::scalarProduct(double scalar) const
{
	tlGslMatrix *result;
	int i, j;

	/** make a new matrix with the correct final size */
	result = new tlGslMatrix(nRows_, nCols_);


	/** for each value, calculate the sum from both sources */
	for (i = 0; i < nRows_; i++)
	{
		for (j = 0; j < nCols_; j++)
		{
			result->setValue(i, j, getValue(i, j) * scalar);
		}
	}

	return result;
}

tlGslMatrix *
tlGslMatrix::scalarSum(double scalar) const
{
	tlGslMatrix *result;
	int i, j;

	/** make a new matrix with the correct final size */
	result = new tlGslMatrix(nRows_, nCols_);


	/** for each value, calculate the sum from both sources */
	for (i = 0; i < nRows_; i++)
	{
		for (j = 0; j < nCols_; j++)
		{
			result->setValue(i, j, getValue(i, j) + scalar);
		}
	}

	return result;
}

tlGslMatrix *
tlGslMatrix::matrixMinusScalar(double scalar) const
{
	tlGslMatrix *result;
	int i, j;

	/** make a new matrix with the correct final size */
	result = new tlGslMatrix(nRows_, nCols_);


	/** for each value, calculate the sum from both sources */
	for (i = 0; i < nRows_; i++)
	{
		for (j = 0; j < nCols_; j++)
		{
			result->setValue(i, j, getValue(i, j) - scalar);
		}
	}

	return result;
}

tlGslMatrix *
tlGslMatrix::scalarMinusGslMatrix(double scalar) const
{
	tlGslMatrix *result;
	int i, j;

	/** make a new matrix with the correct final size */
	result = new tlGslMatrix(nRows_, nCols_);


	/** for each value, calculate the sum from both sources */
	for (i = 0; i < nRows_; i++)
	{
		for (j = 0; j < nCols_; j++)
		{
			result->setValue(i, j, scalar - getValue(i, j));
		}
	}

	return result;
}

tlGslMatrix *
tlGslMatrix::matrixDividedByScalar(double scalar) const
{
	tlGslMatrix *result;
	int i, j;

	/** make a new matrix with the correct final size */
	result = new tlGslMatrix(nRows_, nCols_);


	/** for each value, calculate the sum from both sources */
	for (i = 0; i < nRows_; i++)
	{
		for (j = 0; j < nCols_; j++)
		{
			result->setValue(i, j, getValue(i, j) / scalar);
		}
	}

	return result;
}

tlGslMatrix *
tlGslMatrix::scalarDividedByGslMatrix(double scalar) const
{
	tlGslMatrix *result;
	int i, j;

	/** make a new matrix with the correct final size */
	result = new tlGslMatrix(nRows_, nCols_);


	/** for each value, calculate the sum from both sources */
	for (i = 0; i < nRows_; i++)
	{
	for (j = 0; j < nCols_; j++)
	{
		result->setValue(i, j, scalar / getValue(i, j));
	}
	}

	return result;
}

void
tlGslMatrix::dump() const
{
	tlSrString s;

	s = getValueString(1, 0);
	fprintf(stdout, "%s\n", s.getValue());
}

tlGslMatrixIndexHelper &tlGslMatrix::operator[] (int row)
{
	MSG_ASSERT(row >= 0 && row < nRows_, "row out of range");
	indexHelper_.row_ = row;
	return indexHelper_;
}

double tlGslMatrixIndexHelper::operator[] (int column) const
{
	double result;

	MSG_ASSERT(column >= 0 && column < m_->nCols_, "column out of range");

	result = m_->getValue(row_, column);

	return result;
}
#endif /* GSL_LIBRARY_INSTALLED */

