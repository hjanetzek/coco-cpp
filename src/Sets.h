/*---------------------------------*- C++ -*---------------------------------*\
    Compiler Generator Coco/R,
    Copyright (c) 1990, 2004 Hanspeter Moessenboeck, University of Linz
    extended by M. Loeberbauer & A. Woess, Univ. of Linz
    ported to C++ by Csaba Balazs, University of Szeged
    with improvements by Pat Terry, Rhodes University
-------------------------------------------------------------------------------
License
    This file is part of Compiler Generator Coco/R

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation; either version 2, or (at your option) any
    later version.

    This program is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
    or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

    As an exception, it is allowed to write an extension of Coco/R that is
    used as a plugin in non-free software.

    If not otherwise stated, any source code generated by Coco/R (other than
    Coco/R itself) does not fall under the GNU General Public License.
\*---------------------------------------------------------------------------*/

#ifndef COCO_SETS_H__
#define COCO_SETS_H__

#include "BitArray.h"

namespace Coco
{

/*---------------------------------------------------------------------------*\
                            Class Sets Declaration
\*---------------------------------------------------------------------------*/

//! Functions for BitArray operations
struct Sets
{
	//! Return the first element set in b, -1 if nothing is set
	static int First(const BitArray& b)
	{
		const int max = b.size();
		for (int i=0; i<max; i++)
		{
			if (b[i])
			{
				return i;
			}
		}
		return -1;
	}


	//! Return the number of elements set in b
	static int Elements(const BitArray& b)
	{
		int n = 0;
		const int max = b.size();
		for (int i=0; i<max; i++)
		{
			if (b[i])
			{
				n++;
			}
		}
		return n;
	}


	//! Check: (a == b)?
	static bool Equals(const BitArray& a, const BitArray& b)
	{
		const int max = a.size();
		for (int i=0; i<max; i++)
		{
			if (a[i] != b[i])
			{
				return false;
			}
		}
		return true;
	}


	//! Check: (a > b)?
	static bool Includes(const BitArray& a, const BitArray& b)
	{
		const int max = a.size();
		for (int i=0; i<max; i++)
		{
			if (b[i] && ! a[i])
			{
				return false;
			}
		}
		return true;
	}


	//! Check: (a * b != {})
	static bool Intersect(const BitArray& a, const BitArray& b)
	{
		const int max = a.size();
		for (int i=0; i<max; i++)
			if (a[i] && b[i]) return true;
		return false;
	}


	//! Operation: a = a - b
	static void Subtract(BitArray& a, const BitArray& b)
	{
		BitArray c(b);
		c.Not();
		a.And(c);
	}


	// Member Functions with pointers

	//! Return the first element set in b, -1 if nothing is set
	static inline int First(const BitArray *b)
	{
		return Sets::First(*b);
	}

	//! Return the number of elements set in b
	static inline int Elements(const BitArray *b)
	{
		return Sets::Elements(*b);
	}

	//! Check: (a == b)?
	static inline bool Equals(const BitArray *a, const BitArray *b)
	{
		return Sets::Equals(*a, *b);
	}

	//! Check: (a > b)?
	static inline bool Includes(const BitArray *a, const BitArray *b)
	{
		return Sets::Includes(*a, *b);
	}

	//! Check: (a * b != {})
	static inline bool Intersect(const BitArray *a, const BitArray *b)
	{
		return Sets::Intersect(*a, *b);
	}

	//! Operation: a = a - b
	static inline void Subtract(BitArray *a, const BitArray *b)
	{
		Sets::Subtract(*a, *b);
	}

};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Coco

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
