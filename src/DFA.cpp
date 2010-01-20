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

#include <stdlib.h>
#include <wchar.h>

#include "DFA.h"
#include "Action.h"
#include "Tab.h"
#include "Parser.h"
#include "BitArray.h"
#include "Scanner.h"
#include "Utils.h"

namespace Coco
{

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

//---------- Output primitives
wchar_t* DFA::Ch(wchar_t ch)
{
	wchar_t* format = new wchar_t[10];
	if (ch < 32 || ch >= 0x7F || ch == '\'' || ch == '\\')
		coco_swprintf(format, 10, L"%d\0", int(ch));
	else
		coco_swprintf(format, 10, L"'%c'\0", int(ch));
	return format;
}


wchar_t* DFA::ChCond(wchar_t ch)
{
	wchar_t* format = new wchar_t[20];
	wchar_t* res = Ch(ch);
	coco_swprintf(format, 20, L"ch == %ls\0", res);
	delete[] res;
	return format;
}


void DFA::PutRange(CharSet *s)
{
	for (CharSet::Range *r = s->head; r != NULL; r = r->next)
	{
		if (r->from == r->to)
		{
			wchar_t *from = Ch(r->from);
			fwprintf(gen, L"ch == %ls", from);
			delete[] from;
		}
		else if (r->from == 0)
		{
			wchar_t *to = Ch(r->to);
			fwprintf(gen, L"ch <= %ls", to);
			delete[] to;
		}
		else
		{
			wchar_t *from = Ch(r->from);
			wchar_t *to = Ch(r->to);
			fwprintf(gen, L"(ch >= %ls && ch <= %ls)", from, to);
			delete[] from;
			delete[] to;
		}
		if (r->next != NULL) fwprintf(gen, L" || ");
	}
}


//---------- State handling

State* DFA::NewState()
{
	State *s = new State(); s->nr = ++lastStateNr;
	if (firstState == NULL) firstState = s; else lastState->next = s;
	lastState = s;
	return s;
}


void DFA::NewTransition
(
    State *from, State *to,
    Node::nodeType typ, int sym, Node::transitionType tc
)
{
	Target *t = new Target(to);
	Action *a = new Action(typ, sym, tc); a->target = t;
	from->AddAction(a);
	if (typ == Node::clas) curSy->tokenKind = Symbol::classToken;
}


void DFA::CombineShifts()
{
	State *state;
	Action *a, *b, *c;
	CharSet *seta, *setb;
	for (state = firstState; state != NULL; state = state->next)
	{
		for (a = state->firstAction; a != NULL; a = a->next)
		{
			b = a->next;
			while (b != NULL)
			{
				if (a->target->state == b->target->state && a->tc == b->tc)
				{
					seta = a->Symbols(tab); setb = b->Symbols(tab);
					seta->Or(setb);
					a->ShiftWith(seta, tab);
					c = b; b = b->next; state->DetachAction(c);
				}
				else
				{
					b = b->next;
				}
			}
		}
	}
}


void DFA::FindUsedStates(State *state, BitArray *used)
{
	if ((*used)[state->nr]) return;
	used->Set(state->nr, true);
	for (Action *a = state->firstAction; a != NULL; a = a->next)
	{
		FindUsedStates(a->target->state, used);
	}
}


void DFA::DeleteRedundantStates()
{
	//State *newState = new State[State::lastNr + 1];
	State **newState = (State**) malloc (sizeof(State*) * (lastStateNr + 1));
	BitArray *used = new BitArray(lastStateNr + 1);
	FindUsedStates(firstState, used);
	// combine equal final states
	for (State *s1 = firstState->next; s1 != NULL; s1 = s1->next) // firstState cannot be final
	{
		if
		(
		    (*used)[s1->nr]
		 && s1->endOf != NULL
		 && s1->firstAction == NULL
		 && !(s1->ctx)
		)
		{
			for (State *s2 = s1->next; s2 != NULL; s2 = s2->next)
			{
				if
				(
				    (*used)[s2->nr]
				 && s1->endOf == s2->endOf
				 && s2->firstAction == NULL
				 && !(s2->ctx)
				)
				{
					used->Set(s2->nr, false); newState[s2->nr] = s1;
				}
			}
		}
	}

	State *state;
	for (state = firstState; state != NULL; state = state->next)
	{
		if ((*used)[state->nr])
		{
			for (Action *a = state->firstAction; a != NULL; a = a->next)
			{
				if (!((*used)[a->target->state->nr]))
				{
					a->target->state = newState[a->target->state->nr];
				}
			}
		}
	}

	// delete unused states
	lastState = firstState; lastStateNr = 0; // firstState has number 0
	for (state = firstState->next; state != NULL; state = state->next)
	{
		if ((*used)[state->nr])
		{
			state->nr = ++lastStateNr;
			lastState = state;
		}
		else
		{
			lastState->next = state->next;
		}
	}

	free (newState);
	delete used;
}


State* DFA::TheState(Node *p)
{
	if (p == NULL)
	{
		State *state = NewState();
		state->endOf = curSy;
		return state;
	}
	else
	{
		return p->state;
	}
}


void DFA::Step(State *from, Node *p, BitArray *stepped)
{
	if (p == NULL) return;
	stepped->Set(p->n, true);

	if (p->typ == Node::clas || p->typ == Node::chr)
	{
		NewTransition(from, TheState(p->next), p->typ, p->val, p->code);
	}
	else if (p->typ == Node::alt)
	{
		Step(from, p->sub, stepped); Step(from, p->down, stepped);
	}
	else if (p->typ == Node::iter || p->typ == Node::opt)
	{
		if (p->next != NULL && !((*stepped)[p->next->n]))
		{
			Step(from, p->next, stepped);
		}

		Step(from, p->sub, stepped);
		if ((p->typ == Node::iter) && (p->state != from))
		{
			BitArray *newStepped = new BitArray(tab->nodes.Count);
			Step(p->state, p, newStepped);
			delete newStepped;
		}
	}
}


// Assigns a state n.state to every node n. There will be a transition from
// n.state to n.next.state triggered by n.val. All nodes in an alternative
// chain are represented by the same state.
// Numbering scheme:
//  - any node after a chr, clas, opt, or alt, must get a new number
//  - if a nested structure starts with an iteration the iter node must get a new number
//  - if an iteration follows an iteration, it must get a new number
void DFA::NumberNodes(Node *p, State *state, bool renumIter)
{
	if (p == NULL) return;
	if (p->state != NULL) return; // already visited;
	if (state == NULL || (p->typ == Node::iter && renumIter))
	{
		state = NewState();
	}
	p->state = state;
	if (tab->DelGraph(p)) state->endOf = curSy;

	if (p->typ == Node::clas || p->typ == Node::chr)
	{
		NumberNodes(p->next, NULL, false);
	}
	else if (p->typ == Node::opt)
	{
		NumberNodes(p->next, NULL, false);
		NumberNodes(p->sub, state, true);
	}
	else if (p->typ == Node::iter)
	{
		NumberNodes(p->next, state, true);
		NumberNodes(p->sub, state, true);
	}
	else if (p->typ == Node::alt)
	{
		NumberNodes(p->next, NULL, false);
		NumberNodes(p->sub, state, true);
		NumberNodes(p->down, state, renumIter);
	}
}


void DFA::FindTrans(Node *p, bool start, BitArray *marked)
{
	if (p == NULL || (*marked)[p->n]) return;
	marked->Set(p->n, true);
	if (start)
	{
		BitArray *stepped = new BitArray(tab->nodes.Count);
		Step(p->state, p, stepped); // start of group of equally numbered nodes
		delete stepped;
	}

	if (p->typ == Node::clas || p->typ == Node::chr)
	{
		FindTrans(p->next, true, marked);
	}
	else if (p->typ == Node::opt)
	{
		FindTrans(p->next, true, marked); FindTrans(p->sub, false, marked);
	}
	else if (p->typ == Node::iter)
	{
		FindTrans(p->next, false, marked); FindTrans(p->sub, false, marked);
	}
	else if (p->typ == Node::alt)
	{
		FindTrans(p->sub, false, marked); FindTrans(p->down, false, marked);
	}
}


void DFA::ConvertToStates(Node *p, Symbol *sym)
{
	curGraph = p; curSy = sym;
	if (tab->DelGraph(curGraph)) parser->SemErr(L"token might be empty");
	NumberNodes(curGraph, firstState, true);
	FindTrans(curGraph, true, new BitArray(tab->nodes.Count));
	if (p->typ == Node::iter)
	{
		BitArray *stepped = new BitArray(tab->nodes.Count);
		Step(firstState, p, stepped);
		delete stepped;
	}
}


// match string against current automaton; store it either as a fixedToken or as a litToken
void DFA::MatchLiteral(wchar_t* s, Symbol *sym)
{
	wchar_t *subS = coco_string_create(s, 1, coco_string_length(s)-2);
	s = tab->Unescape(subS);
	coco_string_delete(subS);
	int i, len = coco_string_length(s);
	State *state = firstState;
	Action *a = NULL;
	for (i = 0; i < len; i++)  // try to match s against existing DFA
	{
		a = FindAction(state, s[i]);
		if (a == NULL) break;
		state = a->target->state;
	}
	// if s was not totally consumed or leads to a non-final state => make new DFA from it
	if (i != len || state->endOf == NULL)
	{
		state = firstState; i = 0; a = NULL;
		dirtyDFA = true;
	}
	for (; i < len; i++)  // make new DFA for s[i..len-1]
	{
		State *to = NewState();
		NewTransition(state, to, Node::chr, s[i], Node::normalTrans);
		state = to;
	}
	coco_string_delete(s);
	Symbol *matchedSym = state->endOf;
	if (state->endOf == NULL)
	{
		state->endOf = sym;
	}
	else if
	(
	    matchedSym->tokenKind == Symbol::fixedToken
	 || (a != NULL && a->tc == Node::contextTrans)
	)
	{
		// s matched a token with a fixed definition or a token with an appendix that will be cut off
		wchar_t format[200];
		coco_swprintf(format, 200, L"tokens %ls and %ls cannot be distinguished", sym->name, matchedSym->name);
		parser->SemErr(format);
	}
	else    // matchedSym == classToken || classLitToken
	{
		matchedSym->tokenKind = Symbol::classLitToken;
		sym->tokenKind = Symbol::litToken;
	}
}


void DFA::SplitActions(State *state, Action *a, Action *b)
{
	Action *c; CharSet *seta, *setb, *setc;
	seta = a->Symbols(tab); setb = b->Symbols(tab);
	if (seta->Equals(setb))
	{
		a->AddTargets(b);
		state->DetachAction(b);
	}
	else if (seta->Includes(setb))
	{
		setc = seta->Clone(); setc->Subtract(setb);
		b->AddTargets(a);
		a->ShiftWith(setc, tab);
	}
	else if (setb->Includes(seta))
	{
		setc = setb->Clone(); setc->Subtract(seta);
		a->AddTargets(b);
		b->ShiftWith(setc, tab);
	}
	else
	{
		setc = seta->Clone(); setc->And(setb);
		seta->Subtract(setc);
		setb->Subtract(setc);
		a->ShiftWith(seta, tab);
		b->ShiftWith(setb, tab);
		c = new Action(Node::invalid_, 0, Node::normalTrans);  // typ and sym are set in ShiftWith
		c->AddTargets(a);
		c->AddTargets(b);
		c->ShiftWith(setc, tab);
		state->AddAction(c);
	}
}


bool DFA::Overlap(Action *a, Action *b)
{
	CharSet *seta, *setb;
	if (a->typ == Node::chr)
		if (b->typ == Node::chr) return (a->sym == b->sym);
		else {setb = tab->CharClassSet(b->sym); return setb->Get(a->sym);}
	else
	{
		seta = tab->CharClassSet(a->sym);
		if (b->typ == Node::chr) return seta->Get(b->sym);
		else {setb = tab->CharClassSet(b->sym); return seta->Intersects(setb);}
	}
}


bool DFA::MakeUnique(State *state) // return true if actions were split
{
	bool changed = false;
	for (Action *a = state->firstAction; a != NULL; a = a->next)
		for (Action *b = a->next; b != NULL; b = b->next)
			if (Overlap(a, b))
			{
				SplitActions(state, a, b);
				changed = true;
			}
	return changed;
}


void DFA::MeltStates(State *state)
{
	bool changed, ctx;
	BitArray *targets;
	Symbol *endOf;
	for (Action *action = state->firstAction; action != NULL; action = action->next)
	{
		if (action->target->next != NULL)
		{
			GetTargetStates(action, targets, endOf, ctx);
			Melted *melt = StateWithSet(targets);
			if (melt == NULL)
			{
				State *s = NewState(); s->endOf = endOf; s->ctx = ctx;
				for (Target *targ = action->target; targ != NULL; targ = targ->next)
				{
					s->MeltWith(targ->state);
				}

				do
				{
					changed = MakeUnique(s);
				} while (changed);
				melt = NewMelted(targets, s);
			}
			action->target->next = NULL;
			action->target->state = melt->state;
		}
	}
}


void DFA::FindCtxStates()
{
	for (State *state = firstState; state != NULL; state = state->next)
	{
		for (Action *a = state->firstAction; a != NULL; a = a->next)
		{
			if (a->tc == Node::contextTrans)
			{
				a->target->state->ctx = true;
			}
		}
	}
}


void DFA::MakeDeterministic()
{
	State *state;
	bool changed;
	lastSimState = lastState->nr;
	maxStates = 2 * lastSimState; // heuristic for set size in Melted.set
	FindCtxStates();
	for (state = firstState; state != NULL; state = state->next)
	{
		do
		{
			changed = MakeUnique(state);
		}
		while (changed);
	}

	for (state = firstState; state != NULL; state = state->next)
	{
		MeltStates(state);
	}

	DeleteRedundantStates();
	CombineShifts();
}


void DFA::PrintStates()
{
	FILE* trace = Tab::trace;

	fwprintf(trace, L"\n");
	fwprintf(trace, L"---------- states ----------\n");
	for (State *state = firstState; state != NULL; state = state->next)
	{
		bool first = true;
		if (state->endOf == NULL)
		{
			fwprintf(trace, L"               ");
		}
		else
		{
			fwprintf(trace, L"E(%-12ls)", state->endOf->name);
		}
		fwprintf(trace, L"%3d:", state->nr);
		if (state->firstAction == NULL) fwprintf(trace, L"\n");
		for (Action *action = state->firstAction; action != NULL; action = action->next)
		{
			if (first)
			{
				fwprintf(trace, L" ");
				first = false;
			}
			else
			{
				fwprintf(trace, L"                    ");
			}

			if (action->typ == Node::clas)
			{
				fwprintf(trace, L"%ls", tab->classes[action->sym]->name);
			}
			else
			{
				wchar_t* res = Ch(action->sym);
				fwprintf(trace, L"%ls", res);
				delete[] res;
			}
			for (Target *targ = action->target; targ != NULL; targ = targ->next)
			{
				fwprintf(trace, L"%3d", targ->state->nr);
			}
			if (action->tc == Node::contextTrans)
				fwprintf(trace, L" context\n");
			else
				fwprintf(trace, L"\n");
		}
	}
	fwprintf(trace, L"\n---------- character classes ----------\n");
	tab->WriteCharClasses();
}


void DFA::PrintStatistics() const
{}


//---------------------------- actions --------------------------------

Action* DFA::FindAction(State *state, wchar_t ch)
{
	for (Action *a = state->firstAction; a != NULL; a = a->next)
	{
		if (a->typ == Node::chr && ch == a->sym)
		{
			return a;
		}
		else if (a->typ == Node::clas)
		{
			CharSet *s = tab->CharClassSet(a->sym);
			if (s->Get(ch))
				return a;
		}
	}

	return NULL;
}


void DFA::GetTargetStates
(
    Action *a, BitArray* &targets, Symbol* &endOf, bool &ctx
)
{
	// compute the set of target states
	targets = new BitArray(maxStates); endOf = NULL;
	ctx = false;
	for (Target *t = a->target; t != NULL; t = t->next)
	{
		int stateNr = t->state->nr;

		if (stateNr <= lastSimState)
		{
			targets->Set(stateNr, true);
		}
		else
		{
			targets->Or(MeltedSet(stateNr));
		}
		if (t->state->endOf != NULL)
		{
			if (endOf == NULL || endOf == t->state->endOf)
			{
				endOf = t->state->endOf;
			}
			else
			{
				wprintf(L"Tokens %ls and %ls cannot be distinguished\n", endOf->name, t->state->endOf->name);
				errors->count++;
			}
		}
		if (t->state->ctx)
		{
			ctx = true;
			// The following check seems to be unnecessary. It reported an error
			// if a symbol + context was the prefix of another symbol, e.g.
			//   s1 = "a" "b" "c".
			//   s2 = "a" CONTEXT("b").
			// But this is ok.
			// if (t.state.endOf != null) {
			//   Console.WriteLine("Ambiguous context clause");
			//	 Errors.count++;
			// }
		}
	}
}


//------------------------- melted states ------------------------------


Melted* DFA::NewMelted(BitArray *set, State *state)
{
	Melted *m = new Melted(set, state);
	m->next = firstMelted; firstMelted = m;
	return m;
}


BitArray* DFA::MeltedSet(int nr)
{
	Melted *m = firstMelted;
	while (m != NULL)
	{
		if (m->state->nr == nr)
			return m->set;
		else
			m = m->next;
	}
	//Errors::Exception("-- compiler error in Melted::Set");
	//throw new Exception("-- compiler error in Melted::Set");
	return NULL;
}


Melted* DFA::StateWithSet(BitArray *s)
{
	for (Melted *m = firstMelted; m != NULL; m = m->next)
	{
		if (Sets::Equals(s, m->set))
			return m;
	}
	return NULL;
}


//------------------------ comments --------------------------------

wchar_t* DFA::CommentStr(Node *p)
{
	StringBuilder s = StringBuilder();
	while (p != NULL)
	{
		if (p->typ == Node::chr)
		{
			s.Append(wchar_t(p->val));
		}
		else if (p->typ == Node::clas)
		{
			CharSet *set = tab->CharClassSet(p->val);
			if (set->Elements() != 1) parser->SemErr(L"character set contains more than 1 character");
			s.Append(wchar_t(set->First()));
		}
		else
		{
			parser->SemErr(L"comment delimiters may not be structured");
		}

		p = p->next;
	}
	if (s.GetLength() == 0 || s.GetLength() > 2)
	{
		parser->SemErr(L"comment delimiters must be 1 or 2 characters long");
		s = StringBuilder(L"?");
	}
	return s.ToString();
}


void DFA::NewComment(Node *from, Node *to, bool nested)
{
	Comment *c = new Comment(CommentStr(from), CommentStr(to), nested);
	c->next = firstComment; firstComment = c;
}


//------------------------ scanner generation ----------------------

void DFA::GenComBody(Comment *com)
{
	fwprintf(gen, L"\t\tfor(;;) {\n");

	wchar_t* res = ChCond(com->stop[0]);
	fwprintf(gen, L"\t\t\tif (%ls) ", res);
	fwprintf(gen, L"{\n");
	delete[] res;

	if (coco_string_length(com->stop) == 1)
	{
		fwprintf(gen, L"\t\t\t\tlevel--;\n");
		fwprintf(gen, L"\t\t\t\tif (level == 0) { oldEols = line - line0; NextCh(); return true; }\n");
		fwprintf(gen, L"\t\t\t\tNextCh();\n");
	}
	else
	{
		fwprintf(gen, L"\t\t\t\tNextCh();\n");
		wchar_t* res = ChCond(com->stop[1]);
		fwprintf(gen, L"\t\t\t\tif (%ls) {\n", res);
		delete[] res;
		fwprintf(gen, L"\t\t\t\t\tlevel--;\n");
		fwprintf(gen, L"\t\t\t\t\tif (level == 0) { oldEols = line - line0; NextCh(); return true; }\n");
		fwprintf(gen, L"\t\t\t\t\tNextCh();\n");
		fwprintf(gen, L"\t\t\t\t}\n");
	}
	if (com->nested)
	{
			fwprintf(gen, L"\t\t\t}");
			wchar_t* res = ChCond(com->start[0]);
			fwprintf(gen, L" else if (%ls) ", res);
			delete[] res;
			fwprintf(gen, L"{\n");
		if (coco_string_length(com->stop) == 1)
		{
			fwprintf(gen, L"\t\t\t\tlevel++; NextCh();\n");
		}
		else
		{
			fwprintf(gen, L"\t\t\t\tNextCh();\n");
			wchar_t* res = ChCond(com->start[1]);
			fwprintf(gen, L"\t\t\t\tif (%ls) ", res);
			delete[] res;
			fwprintf(gen, L"{\n");
			fwprintf(gen, L"\t\t\t\t\tlevel++; NextCh();\n");
			fwprintf(gen, L"\t\t\t\t}\n");
		}
	}
	fwprintf(gen, L"\t\t\t} else if (ch == buffer->EoF) return false;\n");
	fwprintf(gen, L"\t\t\telse NextCh();\n");
	fwprintf(gen, L"\t\t}\n");
}


void DFA::GenCommentHeader(Comment *com, int i)
{
	fwprintf(gen, L"\tbool Comment%d();\n", i);
}


void DFA::GenComment(Comment *com, int i)
{
	fwprintf(gen, L"\n");
	fwprintf(gen, L"bool Scanner::Comment%d() ", i);
	fwprintf(gen, L"{\n");
	fwprintf(gen, L"\tint level = 1, pos0 = pos, line0 = line, col0 = col;\n");
	if (coco_string_length(com->start) == 1)
	{
		fwprintf(gen, L"\tNextCh();\n");
		GenComBody(com);
	}
	else
	{
		fwprintf(gen, L"\tNextCh();\n");
		wchar_t* res = ChCond(com->start[1]);
		fwprintf(gen, L"\tif (%ls) ", res);
		delete[] res;
		fwprintf(gen, L"{\n");

		fwprintf(gen, L"\t\tNextCh();\n");
		GenComBody(com);

		fwprintf(gen, L"\t} else {\n");
		fwprintf(gen, L"\t\tbuffer->SetPos(pos0); NextCh(); line = line0; col = col0;\n");
		fwprintf(gen, L"\t}\n");
		fwprintf(gen, L"\treturn false;\n");
	}
	fwprintf(gen, L"}\n");
}


void DFA::CopyFramePart(const wchar_t* stop, const bool doOutput)
{
	bool ok = tab->CopyFramePart(gen, fram, stop, doOutput);
	if (!ok)
	{
		errors->Exception(L" -- incomplete or corrupt scanner frame file");
	}
}


wchar_t* DFA::SymName(Symbol *sym)  // real name value is stored in Tab.literals
{
	if   // Char::IsLetter(sym->name[0])
	(
	    (sym->name[0] >= 'a' && sym->name[0] <= 'z')
	 || (sym->name[0] >= 'A' && sym->name[0] <= 'Z')
	)
	{
		HashTable<Symbol>::Iterator iter = tab->literals.GetIterator();
		while (iter.HasNext())
		{
			HashTable<Symbol>::Entry *e = iter.Next();
			if (e->val == sym)
			{
				return e->key;
			}
		}
	}
	return sym->name;
}


void DFA::GenLiterals()
{
	ArrayList<Symbol> *ts[2];
	ts[0] = &(tab->terminals);
	ts[1] = &(tab->pragmas);

	for (int i = 0; i < 2; ++i)
	{
		for (int j = 0; j < ts[i]->Count; j++)
		{
			Symbol *sym = (*(ts[i]))[j];
			if (sym->tokenKind == Symbol::litToken)
			{
				wchar_t* name = coco_string_create(SymName(sym));
				if (ignoreCase)
				{
					wchar_t *oldName = name;
					name = coco_string_create_lower(name);
					coco_string_delete(oldName);
				}
				// sym.name stores literals with quotes, e.g. "\"Literal\""

				fwprintf(gen, L"\tkeywords.set(L");
				// write keyword, escape non printable characters
				for (int k = 0; name[k] != '\0'; k++)
				{
					char c = name[k];
					fwprintf(gen, (c >= 32 && c <= 0x7F) ? L"%lc" : L"\\x%04x", c);
				}
				fwprintf(gen, L", %d);\n", sym->n);

				coco_string_delete(name);
			}
		}
	}
}


void DFA::CheckLabels()
{
	for (int i=0; i < lastStateNr+1; i++)
	{
		existLabel[i] = false;
	}

	for (State* state = firstState->next; state; state = state->next)
	{
		for (Action* action = state->firstAction; action; action = action->next)
		{
			existLabel[action->target->state->nr] = true;
		}
	}
}


void DFA::WriteState(State *state)
{
	Symbol *endOf = state->endOf;
	fwprintf(gen, L"\t\tcase %d:\n", state->nr);
	if (existLabel[state->nr])
		fwprintf(gen, L"\t\t\tcase_%d:\n", state->nr);
	bool ctxEnd = state->ctx;

	for (Action *action = state->firstAction; action != NULL; action = action->next)
	{
		if (action == state->firstAction)
			fwprintf(gen, L"\t\t\tif (");
		else
			fwprintf(gen, L"\t\t\telse if (");
		if (action->typ == Node::chr)
		{
			wchar_t* res = ChCond(wchar_t(action->sym));
			fwprintf(gen, L"%ls", res);
			delete[] res;
		}
		else
		{
			PutRange(tab->CharClassSet(action->sym));
		}
		fwprintf(gen, L") {");

		if (action->tc == Node::contextTrans)
		{
			fwprintf(gen, L"apx++; ");
			ctxEnd = false;
		}
		else if (state->ctx)
		{
			fwprintf(gen, L"apx = 0; ");
		}

		fwprintf(gen, L"AddCh(); goto case_%d;", action->target->state->nr);
		fwprintf(gen, L"}\n");
	}
	if (state->firstAction == NULL)
		fwprintf(gen, L"\t\t\t{");
	else
		fwprintf(gen, L"\t\t\telse {");

	if (ctxEnd)  // final context state: cut appendix
	{
		fwprintf(gen, L"\n");
		fwprintf(gen, L"\t\t\t\ttlen -= apx;\n");
		fwprintf(gen, L"\t\t\t\tbuffer->SetPos(t->pos); NextCh(); line = t->line; col = t->col;\n");
		fwprintf(gen, L"\t\t\t\tfor (int i = 0; i < tlen; i++) NextCh();\n");
		fwprintf(gen, L"\t\t\t\t");
	}
	if (endOf == NULL)
	{
		fwprintf(gen, L"t->kind = noSym; break;}\n");
	}
	else
	{
		fwprintf(gen, L"t->kind = %d; ", endOf->n);
		if (endOf->tokenKind == Symbol::classLitToken)
		{
			if (ignoreCase)
			{
				fwprintf(gen, L"wchar_t *literal = coco_string_create_lower(tval, 0, tlen); t->kind = keywords.get(literal, t->kind); coco_string_delete(literal); break;}\n");
			}
			else
			{
				fwprintf(gen, L"wchar_t *literal = coco_string_create(tval, 0, tlen); t->kind = keywords.get(literal, t->kind); coco_string_delete(literal); break;}\n");
			}
		}
		else
		{
			fwprintf(gen, L"break;}\n");
		}
	}
}


void DFA::WriteStartTab()
{
	for (Action *action = firstState->firstAction; action; action = action->next)
	{
		int targetState = action->target->state->nr;
		if (action->typ == Node::chr)
		{
			fwprintf(gen, L"\tstart.set(%d, %d);\n", action->sym, targetState);
		}
		else
		{
			CharSet *s = tab->CharClassSet(action->sym);
			for (CharSet::Range *r = s->head; r; r = r->next)
			{
				fwprintf(gen, L"\tfor (int i = %d; i <= %d; ++i) start.set(i, %d);\n", r->from, r->to, targetState);
			}
		}
	}
	fwprintf(gen, L"\tstart.set(Buffer::EoF, -1);\n\n");
}


void DFA::WriteScanner()
{
	int oldPos = tab->buffer->GetPos();  // Pos is modified by CopySourcePart

	fram = tab->OpenFrameFile("Scanner.frame");
	if (fram == NULL)
	{
		errors->Exception(L"-- Cannot open Scanner frame.\n");
	}


	if (dirtyDFA) MakeDeterministic();

	//
	// Header
	//
	gen = tab->OpenGenFile("Scanner.h");
	if (gen == NULL)
	{
		errors->Exception(L"-- Cannot generate Scanner header");
	}

	CopyFramePart(L"-->begin", false);
	tab->CopySourcePart(gen, tab->copyPos, 0);  // copy without emitLines
	CopyFramePart(L"-->namespace_open");
	int nrOfNs = tab->GenNamespaceOpen(gen);

	CopyFramePart(L"-->constantsheader");
	fwprintf(gen, L"\tstatic const int maxT = %d;\n", tab->terminals.Count-1);
	fwprintf(gen, L"\tstatic const int noSym = %d;\n", tab->noSym->n);

	CopyFramePart(L"-->casing0");
	if (ignoreCase)
	{
		fwprintf(gen, L"\twchar_t valCh;       // current input character (for token.val)\n");
	}

	CopyFramePart(L"-->commentsheader");
	Comment *com = firstComment;
	for (int i=0; com; com = com->next, ++i)
	{
		GenCommentHeader(com, i);
	}

	CopyFramePart(L"-->namespace_close");
	tab->GenNamespaceClose(gen, nrOfNs);
	CopyFramePart(L"-->implementation");
	fclose(gen);

	//
	// Source
	//
	gen = tab->OpenGenFile("Scanner.cpp");
	if (gen == NULL)
	{
		errors->Exception(L"-- Cannot generate Scanner source");
	}

	CopyFramePart(L"-->begin", false);
	tab->CopySourcePart(gen, tab->copyPos, 0);  // copy without emitLines
	CopyFramePart(L"-->namespace_open");
	nrOfNs = tab->GenNamespaceOpen(gen);

	CopyFramePart(L"-->declarations");
	WriteStartTab();
	GenLiterals();

	CopyFramePart(L"-->initialization");
	CopyFramePart(L"-->casing1");
	if (ignoreCase)
	{
		fwprintf(gen, L"\t\tvalCh = ch;\n");
		fwprintf(gen, L"\t\tif (ch >= 'A' && ch <= 'Z') ch += ('a' - 'A'); // ch.ToLower()");
	}
	CopyFramePart(L"-->casing2");
	fwprintf(gen, L"\t\ttval[tlen++] = ");
	if (ignoreCase) fwprintf(gen, L"valCh;"); else fwprintf(gen, L"ch;");

	CopyFramePart(L"-->comments");
	com = firstComment;
	for (int i=0; com; com = com->next, ++i)
	{
		GenComment(com, i);
	}

	CopyFramePart(L"-->scan1");
	fwprintf(gen, L"\t\t\t");
	if (tab->ignored->Elements() > 0)
	{
		PutRange(tab->ignored);
	}
	else
	{
		fwprintf(gen, L"false");
	}

	CopyFramePart(L"-->scan2");
	if (firstComment)
	{
		fwprintf(gen, L"\tif (");
		com = firstComment;
		for (int i=0; com; com = com->next, ++i)
		{
			wchar_t* res = ChCond(com->start[0]);
			fwprintf(gen, L"(%ls && Comment%d())", res, i);
			delete[] res;
			if (com->next != NULL)
			{
				fwprintf(gen, L" || ");
			}
		}
		fwprintf(gen, L") return NextToken();");
	}
	if (hasCtxMoves)   /* pdt */
	{
		fwprintf(gen, L"\n");
		fwprintf(gen, L"\tint apx = 0;");
	}
	CopyFramePart(L"-->scan3");

	/* CSB 02-10-05 check the Labels */
	existLabel = new bool[lastStateNr+1];
	CheckLabels();
	for (State *state = firstState->next; state != NULL; state = state->next)
	{
		WriteState(state);
	}

	delete[] existLabel;

	CopyFramePart(L"-->namespace_close");
	tab->GenNamespaceClose(gen, nrOfNs);

	CopyFramePart(L"$$$");
	fclose(gen);
	tab->buffer->SetPos(oldPos);    // restore Pos
}


DFA::DFA(Parser *theParser)
:
	maxStates(0),
	lastStateNr(-1),
	firstState(NULL),
	lastState(NULL),
	ignoreCase(false),
	dirtyDFA(false),
	hasCtxMoves(false),
	existLabel(NULL),
	parser(theParser),
	tab(parser->tab),
	errors(parser->errors),
	firstMelted(NULL),
	firstComment(NULL)
{
	firstState = NewState();
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Coco

// ************************************************************************* //
