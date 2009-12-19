/*-------------------------------------------------------------------------
DFA -- Generation of the Scanner Automaton
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

#ifndef COCO_DFA_H__
#define COCO_DFA_H__

#include <stddef.h>
#include "Action.h"
#include "Comment.h"
#include "State.h"
#include "Symbol.h"
#include "Melted.h"
#include "Node.h"
#include "Target.h"

namespace Coco {

// forward declarations
class Parser;
class Tab;
class BitArray;

/*---------------------------------------------------------------------------*\
                             Class DFA Declaration
\*---------------------------------------------------------------------------*/
//! Generation of the Scanner Automaton
class DFA
{
public:
	static const int eoF = -1;  //!< End-of-file? ... unused?

	int maxStates;

	int lastStateNr;    //!< highest state number
	State *firstState;
	State *lastState;   //!< last allocated state
	int lastSimState;   //!< last non melted state
	FILE* fram;         //!< scanner frame input
	FILE* gen;          //!< generated scanner file
	Symbol *curSy;      //!< current token to be recognized (in FindTrans)
	Node *curGraph;     //!< start of graph for current token (in FindTrans)
	bool ignoreCase;    //!< true if input should be treated case-insensitively
	bool dirtyDFA;      //!< DFA may become nondeterministic in MatchLiteral
	bool hasCtxMoves;   //!< DFA has context transitions
	bool *existLabel;   //!< checking the Labels (to avoid the warning messages)

	//! other Coco objects
	Parser     *parser;
	Tab        *tab;
	Errors     *errors;

	FILE* trace;            //!< trace file
	Melted *firstMelted;    //!< head of melted state list
	Comment *firstComment;  //!< list of comments

	//---------- Output primitives
	wchar_t* Ch(wchar_t ch);
	wchar_t* ChCond(wchar_t ch);
	void  PutRange(CharSet *s);

	//---------- State handling
	State* NewState();
	void NewTransition(State *from, State *to, int typ, int sym, int tc);
	void CombineShifts();
	void FindUsedStates(State *state, BitArray *used);
	void DeleteRedundantStates();
	State* TheState(Node *p);
	void Step(State *from, Node *p, BitArray *stepped);
	void NumberNodes(Node *p, State *state, bool renumIter);
	void FindTrans(Node *p, bool start, BitArray *marked);
	void ConvertToStates(Node *p, Symbol *sym);

	// match string against current automaton; store it either as a fixedToken or as a litToken
	void MatchLiteral(wchar_t* s, Symbol *sym);
	void SplitActions(State *state, Action *a, Action *b);
	bool Overlap(Action *a, Action *b);
	bool MakeUnique(State *state); // return true if actions were split
	void MeltStates(State *state);
	void FindCtxStates();
	void MakeDeterministic();
	void PrintStates();
	void CheckLabels();

	//---------------------------- actions --------------------------------
	Action* FindAction(State *state, wchar_t ch);
	void GetTargetStates(Action *a, BitArray* &targets, Symbol* &endOf, bool &ctx);

	//------------------------- melted states ------------------------------
	Melted* NewMelted(BitArray *set, State *state);
	BitArray* MeltedSet(int nr);
	Melted* StateWithSet(BitArray *s);

	//------------------------ comments --------------------------------
	wchar_t* CommentStr(Node *p);
	void NewComment(Node *from, Node *to, bool nested);

	//------------------------ scanner generation ----------------------
	void GenComBody(Comment *com);
	void GenCommentHeader(Comment *com, int i);
	void GenComment(Comment *com, int i);
	void CopyFramePart(const wchar_t* stop, const bool doOutput = true);
	wchar_t* SymName(Symbol *sym); //!< real name value is stored in Tab.literals
	void GenLiterals();
	void WriteState(State *state);
	void WriteStartTab();
	void WriteScanner();
	DFA(Parser *parser);
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Coco

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
