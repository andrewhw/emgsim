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
 * $Id: tlMatrix.cpp 57 2013-12-13 21:33:01Z andrew $
 */


#ifndef MAKEDEPEND
# include <stdio.h>
# include <string.h>
# include <errno.h>
# include <ctype.h>
#endif

#include "os_defs.h"

#ifdef OS_WINDOWS
		// disable _CRT_SECURE_NO_WARNINGS related flags for now,
		// as they completely break the POSIX interface, as we
		// will have to re-write wrappers for things like fopen
		// to make this work more gracefully
# pragma warning(disable : 4996)
#endif

#include "tclCkalloc.h"
#include "bitstring.h"
#include "mathtools.h"

#include "tlMatrix.h"
#include "tlTuple.h"
#include "tlTable.h"

#define		MAT_FLAG		DETERMINANT		0x01


tlMatrix::tlMatrix()
{
	init_();
}

tlMatrix::tlMatrix(int nRows, int nCols)
{
	int i;

	init_();
	allocate_(nRows, nCols);
	for (i = 0; i < nRows_; i++)
	{
		memset(value_[i], 0, sizeof(double) * nCols_);
	}
}

tlMatrix::tlMatrix(tlMatrix &copy)
{
	int i, j;

	init_();
	allocate_(copy.nRows_, copy.nCols_);
	for (i = 0; i < nRows_; i++)
	{
		for (j = 0; j < nRows_; j++)
		{
			value_[i][j] = copy.value_[i][j];
		}
	}
}

tlMatrix::tlMatrix(tlMatrix *copy)
{
	int i, j;

	init_();
	allocate_(copy->nRows_, copy->nCols_);
	for (i = 0; i < nRows_; i++)
	{
		for (j = 0; j < nRows_; j++)
		{
			value_[i][j] = copy->value_[i][j];
		}
	}
}

tlMatrix::tlMatrix(const tlTuple *src)
{
	int i;

	init_();
	allocate_(src->getNumValues(), 1);
	for (i = 0; i < nRows_; i++)
	{
		value_[i][0] = src->getValue(i).getRealValue();
	}
}

tlMatrix::tlMatrix(const tlTable *src)
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
			value_[i][j] = column->getRealValue(i);
		}
	}
}

void
tlMatrix::init_()
{
	nRows_ = 0;
	nCols_ = 0;
	value_ = NULL;
	indexHelper_.mat_ = this;
	pivot_ = NULL;
}

tlMatrix::~tlMatrix()
{
	int i;

	if (value_ != NULL)
	{
		for (i = 0; i < nRows_; i++)
		{
			ckfree(value_[i]);
		}
		ckfree(value_);
	}
	if (pivot_ != NULL)
	{
		ckfree(pivot_);
	}
}

const char *
tlMatrix::clsId() const
{
	return "tlMatrix";
}

void
tlMatrix::allocate_(int nRows, int nCols)
{
	int i;

	nRows_ = nRows;
	nCols_ = nCols;

	if ((nRows_ > 0) && (nCols_ > 0))
	{
		value_ = (double **) ckalloc(sizeof(double *) * nRows_);
		for (i = 0; i < nRows_; i++)
		{
			value_[i] = (double *) ckalloc(sizeof(double) * nCols_);
		}
	} else
	{
		value_ = NULL;
	}
}

double tlMatrix::getValue(int i, int j) const
{
	return value_[i][j];
}

tlSrString
tlMatrix::getValueString(int breakLines) const
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
				s.sprintf("%S %-9.3g", &s, value_[i][j]);
			}
			if (i < nRows_ - 1)
			{
				s = s + " ;";
				if (breakLines)
					s = s + "\n ";
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
tlMatrix::setValues(const char *s)
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
			value_[i][j] = v;

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
tlMatrix::setValues(tlSrString s)
{
	return setValues(s.getValue());
}

void
tlMatrix::flood(double value)
{
	int i, j;

	for (i = 0; i < nRows_; i++)
	{
		for (j = 0; j < nCols_; j++)
		{
			value_[i][j] = value;
		}
	}
}

int
tlMatrix::eye()
{
	int i, j;

	if (nRows_ != nCols_)
		return 0;

	for (i = 0; i < nRows_; i++)
	{
		for (j = 0; j < nCols_; j++)
		{
			if (i == j)
				value_[i][j] = 1;
			else
				value_[i][j] = 0;
		}
	}
	return 1;
}

double
tlMatrix::determinant()
{
	double result;

	MSG_ASSERT(nRows_ == nCols_, "matrix must be square");
	if (nRows_ != nCols_ )
		return 0;

	if (nRows_ == 0)
		return 0;

	if (nRows_ == 1)
		return value_[0][0];

	if (nRows_ == 2)
	{
		return (value_[0][0] * value_[1][1])
			 - (value_[0][1] * value_[1][0]);
	}

	result = 0;
	{
		tlMatrix *subMatrix;
		double sign = 1;
		int i, j, k, l;

		subMatrix = new tlMatrix(nRows_ - 1, nCols_ - 1);
		subMatrix->ref();

		for (i = 0; i < nCols_; i++)
		{

			/**
			 * fill the sub-matrix with all rows but the first,
			 * and all columns but i
			 */
			for (j = 1; j < nRows_; j++)
			{
				l = 0;
				for (k = 0; k < nCols_; k++)
				{
					if (i != k)
					{
						(*subMatrix)[j-1][l] = value_[j][k];
						l++;
					}
				}
			}

			result = result + (sign * value_[0][i] * subMatrix->determinant());
			sign = sign * (-1);
		}
		subMatrix->unref();
	}

	return result;
}

tlMatrix *
tlMatrix::inverse()
{
	tlMatrix *result, *work;
	double factor;
	int i, j, k;

	if (nRows_ != nCols_)
		return NULL;

	/**
	 * use Gauss-Jordan to invert the matrix
	 */

	/** get working space */
	work = new tlMatrix(nRows_, nCols_ * 2);
	work->ref();

//    printf("Calc Inverse of:\n %s\n", getValueString(1).getValue());
	/** load the matrix and find first max value */
	for (i = 0; i < nRows_; i++)
	{
		for (j = 0; j < nCols_; j++)
		{
			work->value_[i][j] = value_[i][j];
		}
		/** and I */
		work->value_[i][nCols_ + i] = 1;
	}
//    printf("After Load: work =\n %s\n", work->getValueString(1).getValue());

	/**
	 * Gauss forward work -- make matrix upper triangular
	 *
	 * i indexes the beginning column we are working to clear
	 * j indexes the row we are in
	 */
	for (i = 0; i < (nRows_ - 1); i++)
	{

		for (j = i + 1; j < nRows_; j++)
		{
			if (work->value_[j][i] != 0)
			{
				factor = -work->value_[j][i] / work->value_[i][i];
//				printf("factor %d,%d : %f\n", i, j, factor);
				for (k = 0; k < nCols_ * 2; k++)
				{
					work->value_[j][k] =
							(factor * work->value_[i][k])
							+ work->value_[j][k];
					/**
					 * ensure that roundoff is not preventing the
					 * zeroes from appearing
					 */
					if (fabs(work->value_[j][k]) < 0.0001)
					{
						work->value_[j][k] = 0.0;
					}
				}
			}
		}

//		printf(" %3d work =\n %s\n", i, work->getValueString(1).getValue());
	}
//    printf("After Gauss: work =\n %s\n", work->getValueString(1).getValue());


	/** save the pivots -- we will need them later */
	if (pivot_ == NULL)
	{
		pivot_ = (double *) ckalloc(sizeof(double) * nRows_);
		for (i = 0; i < nRows_; i++)
		{
			pivot_[i] = work->value_[i][i];
		}
	}


	/**
	 * Jordan reverse work -- only items on diagonal will remain
	 *
	 * i indexes the beginning column we are working to clear
	 * j indexes the row we are in
	 */
	for (i = (nRows_ - 1); i > 0; i--)
	{

		for (j = i - 1; j >= 0; j--)
		{
//		    printf("Working on [%3d][%3d]\n", j, i);

			if (work->value_[j][i] != 0)
			{
				factor = -work->value_[j][i] / work->value_[i][i];
//				printf("factor %d,%d : %f\n", i, j, factor);
				for (k = 0; k < nCols_ * 2; k++)
				{
					work->value_[j][k] =
							(factor * work->value_[i][k])
							+ work->value_[j][k];
					if (fabs(work->value_[j][k]) < 0.0001)
					{
						work->value_[j][k] = 0;
					}
				}
//				printf("work after row %d factor of %f  =\n %s\n",
//						i, factor,
//						work->getValueString(1).getValue());
			}
		}

//		printf(" %3d work =\n %s\n", i, work->getValueString(1).getValue());
	}
//    printf("After Jordan: work =\n %s\n", work->getValueString(1).getValue());



//    for (i = 0; i < nRows_; i++)
//    {
//		printf("Column %3d pivot in row %3d (%f)\n",
//				i, i, pivot_[i]);
//    }


	/** now divide each row by its pivot */
	for (i = 0; i < nRows_; i++)
	{
//		printf("Dividing row %d by %f\n", i, pivot_[i]);
		for (j = 0; j < nCols_ * 2; j++)
		{
			work->value_[i][j] = work->value_[i][j] / pivot_[i];
		}
//		printf("after scale of row %d =\n %s\n",
//						i, work->getValueString(1).getValue());
	}

//    printf("After scaling:\n %s\n", work->getValueString(1).getValue());


	/** now transfer the results into the returned matrix */
	result = new tlMatrix(nRows_, nCols_);
	for (i = 0; i < nRows_; i++)
	{
		for (j = 0; j < nCols_; j++)
		{
			result->value_[i][j] = work->value_[i][j + nCols_];
		}
	}
	work->unref();
//    printf("result =\n %s\n", result->getValueString(1).getValue());

	return result;
}

tlMatrix *
tlMatrix::transpose() const
{
	tlMatrix *result;
	int i, j;

	result = new tlMatrix(nCols_, nRows_);
	for (i = 0; i < nRows_; i++)
	{
		for (j = 0; j < nCols_; j++)
		{
			result->value_[j][i] = value_[i][j];
		}
	}

	return result;
}

int
tlMatrix::equals(tlMatrix *B) const
{
	double ratio;
	int i, j;

	for (i = 0; i < nRows_; i++)
	{
		for (j = 0; j < nCols_; j++)
		{

			/** protect against division by zero */
			if (value_[i][j] == 0)
			{
				if (fabs(B->value_[i][j]) > 0.0001)
				{
//				    printf("zero mismatch in A [%3d][%3d] : %f : %f\n",
//				    				i, j,
//				    				value_[i][j], B->value_[i][j]);
//				    printf("A%s\n", getValueString(1).getValue());
//				    printf("B%s\n", B->getValueString(1).getValue());
					return 0;
				}

			} else if (B->value_[i][j] == 0)
			{
				if (fabs(value_[i][j]) > 0.0001)
				{
//				    printf("zero mismatch in B [%3d][%3d] : %f : %f\n",
//				    				i, j,
//				    				value_[i][j], B->value_[i][j]);
					return 0;
				}

			} else
			{
				ratio = value_[i][j] / B->value_[i][j];

				if (ratio < 0.9 || ratio > 1.1)
				{
//				    printf("mismatch ratio [%3d][%3d]: %f / %f = %f\n",
//				    				i, j,
//				    				value_[i][j], B->value_[i][j],
//				    				value_[i][j] / B->value_[i][j]);
					return 0;
				}
			}
		}
	}

	return 1;
}

tlMatrix *
tlMatrix::product(tlMatrix *B) const
{
	tlMatrix *result;
	tlMatrix const *A;
	double sum;
	int i, j, k;

	A = this;

	/** if sizes are not right, we cannot continue */
	if (A->nCols_ != B->nRows_)
		return NULL;


	/** make a new matrix with the correct final size */
	result = new tlMatrix(A->nRows_, B->nCols_);


	/**
	 * for each intersection in the final matrix, calculate the product
	 * i - row in final matrix and row in A
	 * j - column in final matrix and column in B
	 * k - column in A and row in B
	 */
	for (i = 0; i < result->nRows_; i++)
	{
		for (j = 0; j < result->nCols_; j++)
		{

			/** do the product for this point */
			sum = 0;

			for (k = 0; k < A->nCols_; k++)
			{
				sum += A->value_[i][k] * B->value_[k][j];
			}

			(*result)[i][j] = sum;
		}
	}

	return result;
}

tlMatrix *
tlMatrix::sum(const tlMatrix *B) const
{
	tlMatrix *result;
	tlMatrix const *A;
	int i, j;

	A = this;

	/** if sizes are not right, we cannot continue */
	if (A->nCols_ != B->nCols_)
		return NULL;

	if (A->nRows_ != B->nRows_)
		return NULL;


	/** make a new matrix with the correct final size */
	result = new tlMatrix(A->nRows_, A->nCols_);


	/** for each value, calculate the sum from both sources */
	for (i = 0; i < A->nRows_; i++)
	{
		for (j = 0; j < A->nCols_; j++)
		{
			result->value_[i][j] = A->value_[i][j] + B->value_[i][j];
		}
	}

	return result;
}

tlMatrix *
tlMatrix::difference(const tlMatrix *B) const
{
	tlMatrix *result;
	tlMatrix const *A;
	int i, j;

	A = this;

	/** if sizes are not right, we cannot continue */
	if (A->nCols_ != B->nCols_)
		return NULL;

	if (A->nRows_ != B->nRows_)
		return NULL;


	/** make a new matrix with the correct final size */
	result = new tlMatrix(A->nRows_, A->nCols_);


	/** for each value, calculate the sum from both sources */
	for (i = 0; i < A->nRows_; i++)
	{
		for (j = 0; j < A->nCols_; j++)
		{
			result->value_[i][j] = A->value_[i][j] - B->value_[i][j];
		}
	}

	return result;
}

tlMatrix *
tlMatrix::scalarProduct(double scalar) const
{
	tlMatrix *result;
	int i, j;

	/** make a new matrix with the correct final size */
	result = new tlMatrix(nRows_, nCols_);


	/** for each value, calculate the sum from both sources */
	for (i = 0; i < nRows_; i++)
	{
		for (j = 0; j < nCols_; j++)
		{
			result->value_[i][j] = value_[i][j] * scalar;
		}
	}

	return result;
}

tlMatrix *
tlMatrix::scalarSum(double scalar) const
{
	tlMatrix *result;
	int i, j;

	/** make a new matrix with the correct final size */
	result = new tlMatrix(nRows_, nCols_);


	/** for each value, calculate the sum from both sources */
	for (i = 0; i < nRows_; i++)
	{
		for (j = 0; j < nCols_; j++)
		{
			result->value_[i][j] = value_[i][j] + scalar;
		}
	}

	return result;
}

tlMatrix *
tlMatrix::matrixMinusScalar(double scalar) const
{
	tlMatrix *result;
	int i, j;

	/** make a new matrix with the correct final size */
	result = new tlMatrix(nRows_, nCols_);


	/** for each value, calculate the sum from both sources */
	for (i = 0; i < nRows_; i++)
	{
		for (j = 0; j < nCols_; j++)
		{
			result->value_[i][j] = value_[i][j] - scalar;
		}
	}

	return result;
}

tlMatrix *
tlMatrix::scalarMinusMatrix(double scalar) const
{
	tlMatrix *result;
	int i, j;

	/** make a new matrix with the correct final size */
	result = new tlMatrix(nRows_, nCols_);


	/** for each value, calculate the sum from both sources */
	for (i = 0; i < nRows_; i++)
	{
		for (j = 0; j < nCols_; j++)
		{
			result->value_[i][j] = scalar - value_[i][j];
		}
	}

	return result;
}

tlMatrix *
tlMatrix::matrixDividedByScalar(double scalar) const
{
	tlMatrix *result;
	int i, j;

	/** make a new matrix with the correct final size */
	result = new tlMatrix(nRows_, nCols_);


	/** for each value, calculate the sum from both sources */
	for (i = 0; i < nRows_; i++)
	{
		for (j = 0; j < nCols_; j++)
		{
			result->value_[i][j] = value_[i][j] / scalar;
		}
	}

	return result;
}

tlMatrix *
tlMatrix::scalarDividedByMatrix(double scalar) const
{
	tlMatrix *result;
	int i, j;

	/** make a new matrix with the correct final size */
	result = new tlMatrix(nRows_, nCols_);


	/** for each value, calculate the sum from both sources */
	for (i = 0; i < nRows_; i++)
	{
		for (j = 0; j < nCols_; j++)
		{
			result->value_[i][j] = scalar / value_[i][j];
		}
	}

	return result;
}

tlMatrixIndexHelper &tlMatrix::operator[] (int row)
{
	MSG_ASSERT(row >= 0 && row < nRows_, "row out of range");
	indexHelper_.row_ = row;
	return indexHelper_;
}

double &tlMatrixIndexHelper::operator[] (int column) const
{
	MSG_ASSERT(column >= 0 && column < mat_->nCols_, "column out of range");
	return ((mat_->value_[row_])[column]);
}

