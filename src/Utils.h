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

#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <string>
#include <iostream>

#include "Scanner.h"

namespace Coco
{

// * * * * * * * * * *  Wide Character String Routines * * * * * * * * * * * //

//
// string handling, wide character
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//! Create by copying ASCII byte str
wchar_t* coco_string_create(const char* str);

//! Create a string by concatenating str1 and str2
wchar_t* coco_string_create_append(const wchar_t* str1, const wchar_t* str2);

//! Create a string by concatenating str1 and str2
wchar_t* coco_string_create_append(const wchar_t* str1, const std::string& str2);


//! Create by copying ASCII byte str
std::wstring coco_stdWString(const std::string& str);


//! Create by concatenating str1 and str2
std::wstring coco_stdWString_append
(
    const std::wstring& str1,
    const std::string& str2
);


//! Append str to dest
void coco_string_merge(wchar_t* &dest, const wchar_t* str);

//! Append str to dest
void coco_string_merge(wchar_t* &dest, const std::string& str);


//! Compare strings, return 0 if they are equal
inline int coco_string_compare(const wchar_t* str1, const wchar_t* str2)
{
    return wcscmp(str1, str2);
}

//! Compare strings, return 0 if they are equal
inline int coco_string_compare(const char* str1, const char* str2)
{
    return strcmp(str1, str2);
}


//! Return the index of the first occurrence of ch.
//! Return -1 if nothing is found.
int coco_string_indexof(const wchar_t* str, const wchar_t ch);


//! Check for bool values. Return 1 for 'true', 0 for 'false' and -1 for unknown
int coco_string_checkBool(const wchar_t* str);


//! Roughly equivalent to std::getline, but uses stdio file handle
bool getLine(FILE* istr, std::string& line);


//! Output wchar_t as UTF8.
std::ostream& operator<<(std::ostream& os, const wchar_t wc);

//! Output a wchar_t string as UTF8.
std::ostream& operator<<(std::ostream& os, const wchar_t* ws);

//! Output std::wstring as UTF8.
std::ostream& operator<<(std::ostream& os, const std::wstring&);


// * * * * * * * * * End of Wide Character String Routines * * * * * * * * * //



// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Coco

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
