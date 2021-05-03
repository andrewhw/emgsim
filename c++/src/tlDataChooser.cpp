/**
 * Copyright (c) 2013
 * All rights reserved.
 *
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
 * ----------------------------------------------------------------
 *
 * Creation and storage of a Kohonen Self Organizing Map
 *
 * ----------------------------------------------------------------
 * $Id: tlDataChooser.cpp 57 2013-12-13 21:33:01Z andrew $
 */

#include "tlDataChooser.h"
#include "random.h"



tlDataChooser::tlDataChooser(long size)
{
	nElements_ = size;
	usebit_ = ALLOC_BITSTRING(nElements_);
	reset();
}

tlDataChooser::~tlDataChooser()
{
	FREE_BITSTRING(usebit_);
}

void
tlDataChooser::reset()
{
	ZERO_BITSTRING(usebit_, nElements_);
	nUsed_ = nCycles_ = 0;
}

long
tlDataChooser::choose()
{
	int index;

	/**
	 * If we exhausted the list last time, reset the
	 * counters
	 */
	if (nUsed_ >= nElements_)
	{
		nCycles_++;
		nUsed_ = 0;
		ZERO_BITSTRING(usebit_, nElements_);
	}


	/**
	 * choose a random index, then search until the
	 * bitmap indicates an unchosen element
	 */
	index = localRandom() % nElements_;


	while (GET_BIT(usebit_, index) != 0)
	{
		index = (index + 1) % nElements_;
	}

	SET_BIT(usebit_, index, 1);
	nUsed_++;

	return index;
}

