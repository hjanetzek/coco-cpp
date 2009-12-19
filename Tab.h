/*-------------------------------------------------------------------------
Tab -- Symbol Table Management
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

#ifndef COCO_TAB_H__
#define COCO_TAB_H__

#include "ArrayList.h"
#include "HashTable.h"
#include "StringBuilder.h"
#include "SortedList.h"
#include "Scanner.h"
#include "Position.h"
#include "Symbol.h"
#include "Node.h"
#include "Graph.h"
#include "Sets.h"
#include "CharClass.h"


namespace Coco {

// forward declarations
class Errors;
class Parser;
class BitArray;

/*---------------------------------------------------------------------------*\
                             Class Tab Declaration
\*---------------------------------------------------------------------------*/
//! Symbol Table Management
class Tab {
public:
	//! String representations for the node types (in Node class)
	static const char* nTyp[];

	//! String representations for the token kinds (in Symbol class)
	static const char* tKind[];

	//! String for replacing the file prefix name
	static const wchar_t* prefixMacro;

	Position *semDeclPos;   //!< position of global semantic declarations
	CharSet *ignored;       //!< characters ignored by the scanner
	bool ddt[10];           //!< debug and test switches
	Symbol *gramSy;         //!< root nonterminal; filled by ATG
	Symbol *eofSy;          //!< end of file symbol
	Symbol *noSym;          //!< used in case of an error
	BitArray *allSyncSets;  //!< union of all synchronisation sets
	HashTable *literals;    //!< symbols that are used as literals
	wchar_t* srcName;       //!< name of the atg file (including path)
	wchar_t* srcDir;        //!< directory path of the atg file
	wchar_t* nsName;        //!< namespace for generated files
	wchar_t* prefixName;    //!< prefix for generated files
	wchar_t* frameDir;      //!< directory containing the frame files
	wchar_t* outDir;        //!< directory for generated files
	bool emitLines;         //!< emit line directives in generated parser
	bool makeBackup;        //!< create .bak files for generated parser/scanner
	BitArray *visited;      //!< mark list for graph traversals
	Symbol *curSy;          //!< current symbol in computation of sets
	Parser *parser;         //!< other Coco objects
	FILE* trace;            //!< trace file

	Errors *errors;

	ArrayList *terminals;
	ArrayList *pragmas;
	ArrayList *nonterminals;


	ArrayList *nodes;
	Node *dummyNode;

	ArrayList *classes;
	int dummyName;


	Tab(Parser *parser);

	//---------------------------------------------------------------------
	//  Symbol list management
	//---------------------------------------------------------------------


	Symbol* NewSym(int typ, const wchar_t* name, int line);
	Symbol* FindSym(const wchar_t* name);
	int Num(Node *p);
	void PrintSym(Symbol *sym);
	void PrintSymbolTable();
	void PrintSet(BitArray *s, int indent);

	//---------------------------------------------------------------------
	//  Syntax graph management
	//---------------------------------------------------------------------

	Node* NewNode(int typ, Symbol *sym, int line);
	Node* NewNode(int typ, Node* sub);
	Node* NewNode(int typ, int val, int line);
	void MakeFirstAlt(Graph *g);
	void MakeAlternative(Graph *g1, Graph *g2);
	void MakeSequence(Graph *g1, Graph *g2);
	void MakeIteration(Graph *g);
	void MakeOption(Graph *g);
	void Finish(Graph *g);
	void DeleteNodes();
	Graph* StrToGraph(const wchar_t* str);

	//! set transition code in the graph rooted at p
	void SetContextTrans(Node *p);

	//------------ graph deletability check -----------------

	bool DelGraph(Node* p);
	bool DelSubGraph(Node* p);
	bool DelNode(Node* p);

	//----------------- graph printing ----------------------

	int Ptr(Node *p, bool up);
	wchar_t* Pos(Position *pos);
	wchar_t* Name(const wchar_t* name);
	void PrintNodes();

	//---------------------------------------------------------------------
	//  Character class management
	//---------------------------------------------------------------------

	CharClass* NewCharClass(const wchar_t* name, CharSet *s);
	CharClass* FindCharClass(const wchar_t* name);
	CharClass* FindCharClass(CharSet *s);
	CharSet* CharClassSet(int i);

	//----------- character class printing

	wchar_t* Ch(const wchar_t ch);
	void WriteCharSet(CharSet *s);
	void WriteCharClasses();

	//---------------------------------------------------------------------
	//  Symbol set computations
	//---------------------------------------------------------------------

	//! Computes the first set for the given Node
	BitArray* First0(Node *p, BitArray *mark);
	BitArray* First(Node *p);
	void CompFirstSets();
	void CompFollow(Node *p);
	void Complete(Symbol *sym);
	void CompFollowSets();
	Node* LeadingAny(Node *p);
	void FindAS(Node *p); // find ANY sets
	void CompAnySets();
	BitArray* Expected(Node *p, Symbol *curSy);
	//! does not look behind resolvers; only called during LL(1) test and in CheckRes
	BitArray* Expected0(Node *p, Symbol *curSy);
	void CompSync(Node *p);
	void CompSyncSets();
	void SetupAnys();
	void CompDeletableSymbols();
	void RenumberPragmas();
	void CompSymbolSets();

	//---------------------------------------------------------------------
	//  String handling
	//---------------------------------------------------------------------

	wchar_t  Hex2Char(const wchar_t* s);
	wchar_t* Char2Hex(const wchar_t ch);
	wchar_t* Unescape(const wchar_t* s);
	wchar_t* Escape(const wchar_t* s);

	//---------------------------------------------------------------------
	//  Grammar checks
	//---------------------------------------------------------------------

	bool GrammarOk();

	//--------------- check for circular productions ----------------------

	//! node of list for finding circular productions
	class CNode {
	public:
		Symbol *left, *right;
		CNode(Symbol *l, Symbol *r) : left(l), right(r) {}
	};

	void GetSingles(Node *p, ArrayList *singles);
	bool NoCircularProductions();

	//--------------- check for LL(1) errors ----------------------

	void LL1Error(int cond, Symbol *sym);
	void CheckOverlap(BitArray *s1, BitArray *s2, int cond);
	void CheckAlts(Node *p);
	void CheckLL1();

	//------------- check if resolvers are legal  --------------------

	void ResErr(Node *p, const wchar_t* msg);
	void CheckRes(Node *p, bool rslvAllowed);
	void CheckResolvers();

	//------------- check if every nts has a production --------------------

	bool NtsComplete();

	//-------------- check if every nts can be reached  -----------------

	void MarkReachedNts(Node *p);
	bool AllNtReached();

	//--------- check if every nts can be derived to terminals  ------------

	//! true if graph can be derived to terminals
	bool IsTerm(Node *p, BitArray *mark);
	bool AllNtToTerm();

	//---------------------------------------------------------------------
	//  Cross reference list
	//---------------------------------------------------------------------

	void XRef();
	void SetDDT(const wchar_t* str);

	//---------------------------------------------------------------------
	//  Output file generation, common to DFA and ParserGen
	//---------------------------------------------------------------------

	//! Generate begin of namespace enclosure
	static int GenNamespaceOpen(FILE*, const wchar_t *nsName);
	//! Generate end of namespace enclosure
	static void GenNamespaceClose(FILE*, int nrOfNs);

	//! Check if Grammar file name appears to be Coco itself
	static bool checkIsCocoAtg(const wchar_t* srcName);

	//! Open a generated file, return NULL on failure
	FILE* OpenGen(const wchar_t* dir, const wchar_t* name, bool backUp);

	//! Copy frame part from istr, to ostr until stop mark is seen
	//  Return true on success
	bool CopyFramePart(FILE* ostr, FILE* istr, const wchar_t* stop);
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Coco

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
