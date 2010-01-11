/*-------------------------------------------------------------------------
Compiler Generator Coco/R,
Copyright (c) 1990, 2004 Hanspeter Moessenboeck, University of Linz
extended by M. Loeberbauer & A. Woess, Univ. of Linz
ported to C++ by Csaba Balazs, University of Szeged
with improvements by Pat Terry, Rhodes University

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
-------------------------------------------------------------------------*/
/*!

@mainpage The Compiler Generator Coco/R, C++ version.

http://www.ssw.uni-linz.ac.at/coco/

@section usage Program Usage

@verbatim
Coco Grammar.atg {Option}
Options:
  -namespace <Name>      eg, My::Name::Space
  -prefix    <Name>      for unique Parser/Scanner file names
  -frames    <Dir>       for frames not in the source directory
  -trace     <String>    trace with output to trace.txt
  -trace2    <String>    trace with output on stderr
  -o         <Dir>       output directory
  -lines                 include #line pragmas in the generated code
  -single                include the Scanner code in the Parser file
  -bak                   save existing Parser/Scanner files as .bak
  -help                  print this usage
@endverbatim
The valid trace string values are listed below.

The Scanner.frame and Parser.frame must be located in one of these
directories:
-# In the specified -frames directory.
-# The current directory.
-# The same directory as the atg grammar.

Unless specified with the @em -o option, the generated scanner/parser
files are written in the same directory as the atg grammar!


@section trace Trace output options

 - 0 | A: prints the states of the scanner automaton
 - 1 | F: prints the First and Follow sets of all nonterminals
 - 2 | G: prints the syntax graph of the productions
 - 3 | I: traces the computation of the First sets
 - 4 | J: prints the sets associated with ANYs and synchronisation sets
 - 6 | S: prints the symbol table (terminals, nonterminals, pragmas)
 - 7 | X: prints a cross reference list of all syntax symbols
 - 8 | P: prints statistics about the Coco run
 - 9    : unused

The trace output can be switched on as a command-line option or by
the pragma:
@verbatim
    $ { digit | letter }
@endverbatim
in the attributed grammar.

The extended directive format may also be used in the attributed grammar:
@verbatim
    $trace=(digit | letter){ digit | letter }
@endverbatim

@section Compiler Directives (Extended Pragmas)

To improve the reliability of builds in complex environments, it is
possible to specify the desired namespace and/or file prefix as a
directive within the grammar. For example, when compiling the
Coco-cpp.atg itself, it can be compiled within the 'Coco' namespace
as specified on the command-line. For example,

@verbatim
    Coco -namespace Coco Coco-cpp.atg
@endverbatim

As an alternative, it can be specified within the Coco-cpp.atg file:
@verbatim
    COMPILER Coco
    $namespace=Coco
@endverbatim

The @em -prefix command-line option can be similarly specified. For
example,

@verbatim
    COMPILER GramA
    $namespace=MyProj:GramA
    $prefix=GramA

    COMPILER GramB
    $namespace=MyProj:GramB
    $prefix=GramB
@endverbatim

would generate the scanner/parser files in the respective namespaces,
but with these names:

@verbatim
    GramAParser.{h,cpp}
    GramAScanner.{h,cpp}

    GramBParser.{h,cpp}
    GramBScanner.{h,cpp}
@endverbatim

The resulting scanner/parsers are separated not only in the C++
namespace, but also on the filesystem. Even if the resulting files are
to be located in separate directories, using a prefix can be useful to
avoid confusion. For example, when the compiler -I includes both
directories, an include "Parser.h" will only be able to address one of
the parsers.

For completeness, it is also possible to add in trace string parameters
with the same syntax. For example,

@verbatim
    COMPILER GramC
    $trace=ags
@endverbatim

*/
/*-------------------------------------------------------------------------*/

#include <stdio.h>
#include "Scanner.h"
#include "Parser.h"
#include "Utils.h"
#include "Tab.h"

//! @namespace Coco
//! @brief Compiler Generator Coco/R, C++ version
using namespace Coco;

void printUsage(const char* message = NULL)
{
	if (message) {
		wprintf(L"\nError: %s\n\n", message);
	}

	wprintf(L"Usage: Coco Grammar.atg {Option}\n");
	wprintf(L"Options:\n");
	wprintf(L"  -namespace <Name>      eg, My::Name::Space\n");
	wprintf(L"  -prefix    <Name>      for unique Parser/Scanner file names\n");
	wprintf(L"  -frames    <Dir>       for frames not in the source directory\n");
	wprintf(L"  -trace     <String>    trace with output to trace.txt\n");
	wprintf(L"  -trace2    <String>    trace with output on stderr\n");
	wprintf(L"  -o         <Dir>       output directory\n");
	wprintf(L"  -lines                 include #line pragmas in the generated code\n");
	wprintf(L"  -single                include the Scanner code in the Parser file\n");
	wprintf(L"  -bak                   save existing Parser/Scanner files as .bak\n");
	wprintf(L"  -help                  print this usage\n");
	wprintf(L"\nValid characters in the trace string:\n");
	wprintf(L"  A  trace automaton\n");
	wprintf(L"  F  list first/follow sets\n");
	wprintf(L"  G  print syntax graph\n");
	wprintf(L"  I  trace computation of first sets\n");
	wprintf(L"  J  list ANY and SYNC sets\n");
	wprintf(L"  P  print statistics\n");
	wprintf(L"  S  list symbol table\n");
	wprintf(L"  X  list cross reference table\n");
	wprintf(L"Scanner.frame and Parser.frame must be located in one of these directories:\n");
	wprintf(L"  1. In the specified -frames directory.\n");
	wprintf(L"  2. The current directory.\n");
	wprintf(L"  3. The same directory as the atg grammar.\n\n");
	wprintf(L"http://www.ssw.uni-linz.ac.at/coco/\n\n");
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#ifdef _WIN32
int wmain(int argc, wchar_t *argv[]) {
#else
int main(int argc, char *argv_[]) {
	wchar_t** argv = new wchar_t*[argc];
	for (int i = 0; i < argc; ++i) {
		argv[i] = coco_string_create(argv_[i]);
	}
#endif
	wprintf(L"Coco/R C++ (11 Jan 2010)\n");

	wchar_t *srcName = NULL;
	bool traceToFile = true;

	for (int i = 1; i < argc; i++) {
		if (coco_string_equal(argv[i], L"-help"))
		{
			printUsage();
			return 0;
		}
		else if (coco_string_equal(argv[i], L"-namespace")) {
			if (++i == argc) {
				printUsage("missing parameter on -namespace");
				return 1;
			}
			Tab::nsName = coco_string_create(argv[i]);
		}
		else if (coco_string_equal(argv[i], L"-prefix")) {
			if (++i == argc) {
				printUsage("missing parameter on -prefix");
				return 1;
			}
			Tab::prefixName = coco_string_create(argv[i]);
		}
		else if (coco_string_equal(argv[i], L"-frames")) {
			if (++i == argc) {
				printUsage("missing parameter on -frames");
				return 1;
			}
			Tab::frameDir = coco_string_create(argv[i]);
		}
		else if (coco_string_equal(argv[i], L"-trace")) {
			if (++i == argc) {
				printUsage("missing parameter on -trace");
				return 1;
			}
			traceToFile = true;
			Tab::SetDDT(argv[i]);
		}
		else if (coco_string_equal(argv[i], L"-trace2")) {
			if (++i == argc) {
				printUsage("missing parameter on -trace2");
				return 1;
			}
			traceToFile = false;
			Tab::SetDDT(argv[i]);
		}
		else if (coco_string_equal(argv[i], L"-o")) {
			if (++i == argc) {
				printUsage("missing parameter on -o");
				return 1;
			}
			Tab::outDir = coco_string_create_append(argv[i], L"/");
		}
		else if (coco_string_equal(argv[i], L"-lines")) {
			Tab::emitLines = true;
		}
		else if (coco_string_equal(argv[i], L"-single")) {
			Tab::singleOutput = true;
		}
		else if (coco_string_equal(argv[i], L"-bak")) {
			Tab::makeBackup = true;
		}
		else if (argv[i][0] == L'-') {
			wprintf(L"\nError: unknown option: '%ls'\n\n", argv[i]);
			printUsage();
			return 1;
		}
		else if (srcName != NULL) {
			printUsage("grammar can only be specified once");
			return 1;
		}
		else {
			srcName = coco_string_create(argv[i]);
		}
	}

#ifndef _WIN32
	// deallocate the wide-character strings
	for (int i = 0; i < argc; ++i) {
		coco_string_delete(argv[i]);
	}
	delete [] argv; argv = NULL;
#endif

	if (srcName != NULL) {
		int pos = coco_string_lastindexof(srcName, '/');
		if (pos < 0) pos = coco_string_lastindexof(srcName, '\\');
		wchar_t* srcDir = coco_string_create(srcName, 0, pos+1);
		Tab::srcDir     = srcDir;
		if (Tab::outDir == NULL) {
			Tab::outDir = Tab::srcDir;
		}

		Coco::Scanner *scanner = new Coco::Scanner(srcName);
		Coco::Parser  *parser  = new Coco::Parser(scanner);
		Coco::Tab     *tab     = new Coco::Tab(parser);

		tab->srcName    = srcName;

		wchar_t *traceFileName = coco_string_create_append(Tab::outDir, L"trace.txt");
		char *chTrFileName = coco_string_create_char(traceFileName);

		if (traceToFile && (Tab::trace = fopen(chTrFileName, "w")) == NULL) {
			wprintf(L"-- cannot write trace file to %ls\n", traceFileName);
			return 1;
		}

		// attach Tab before creating Scanner/Parser generators
		parser->tab  = tab;
		parser->dfa  = new Coco::DFA(parser);
		parser->pgen = new Coco::ParserGen(parser);

		parser->Parse();

		// see if anything was written
		if (traceToFile) {
			fclose(Tab::trace);

			// obtain the FileSize
			Tab::trace = fopen(chTrFileName, "r");
			fseek(Tab::trace, 0, SEEK_END);
			long fileSize = ftell(Tab::trace);
			fclose(Tab::trace);
			if (fileSize == 0)
				remove(chTrFileName);
			else
				wprintf(L"trace output is in %ls\n", traceFileName);
		}

		wprintf(L"%d errors detected\n", parser->errors->count);
		if (parser->errors->count != 0) {
			return 1;
		}

		coco_string_delete(Tab::nsName);
		coco_string_delete(Tab::prefixName);
		coco_string_delete(Tab::frameDir);

		delete parser->pgen;
		delete parser->dfa;
		delete tab;
		delete parser;
		delete scanner;
		coco_string_delete(chTrFileName);
		coco_string_delete(traceFileName);
		coco_string_delete(srcDir);
	} else {
		printUsage();
	}

	coco_string_delete(srcName);

	return 0;
}


// ************************************************************************* //
