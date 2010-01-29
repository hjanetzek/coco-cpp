/*---------------------------------*- C++ -*---------------------------------*\
Compiler Generator Coco/R,
Copyright (c) 1990, 2004 Hanspeter Moessenboeck, University of Linz
extended by M. Loeberbauer & A. Woess, Univ. of Linz
ported to C++ by Csaba Balazs, University of Szeged
with improvements by Pat Terry, Rhodes University

License
    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation; either version 2, or (at your option) any
    later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

    As an exception, it is allowed to write an extension of Coco/R that is
    used as a plugin in non-free software.

    If not otherwise stated, any source code generated by Coco/R (other than
    Coco/R itself) does not fall under the GNU General Public License.
\*---------------------------------------------------------------------------*/

/**
 * @file Utils.h
 * @brief Miscellaneous utilities
 *
 * These utilities are used with Coco/R itself, but are generally not
 * needed in other generated Scanner/Parser code.
 *
 */

#ifndef COCO_UTILS_H__
#define COCO_UTILS_H__

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <wchar.h>

#include "Scanner.h"

#ifdef _WIN32
# if _MSC_VER >= 1400     // VC++ 8.0
#  define coco_swprintf swprintf_s
# elif _MSC_VER >= 1300   // VC++ 7.0
#  define coco_swprintf _snwprintf
# elif defined (__MINGW32__)        // MINGW has(had) wrong swprintf args
#  define coco_swprintf _snwprintf
# endif
#endif

// assume every other compiler knows swprintf
#ifndef coco_swprintf
# define coco_swprintf swprintf
#endif


namespace Coco
{

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

//! Create by copying ASCII byte str
std::wstring coco_stdWString(const std::string& str);

//! Compare strings, return true if they are equal
inline bool coco_string_equal(const wchar_t* str1, const wchar_t* str2)
{
    return !wcscmp(str1, str2);
}

//! Compare strings, return true if they are equal
inline bool coco_string_equal(const char* str1, const char* str2)
{
    return !strcmp(str1, str2);
}

//! Compare string contents, return true if they are equal
bool  coco_string_equal(const wchar_t* str1, const char* str2);


//! Roughly equivalent to std::getline, but uses stdio file handle
bool getLine(FILE*, std::string& line);


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Coco

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
