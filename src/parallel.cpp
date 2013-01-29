/*
 * parallel.cpp
 *
 *  Created on: Nov 1, 2012
 *      Author: Ned Bingham and Nicholas Kramer
 *
 *  DO NOT DISTRIBUTE
 */

#include "parallel.h"
#include "conditional.h"
#include "loop.h"
#include "block.h"

parallel::parallel()
{
	chp = "";
	_kind = "parallel";
}
parallel::parallel(string chp, map<string, keyword*> types, map<string, variable> *globals, map<string, variable> *label, string tab, int verbosity)
{
	clear();

	_kind = "parallel";
	this->chp = chp;
	this->tab = tab;
	this->verbosity = verbosity;
	this->global = globals;
	this->label = label;

	expand_shortcuts();
	parse(types);
}
parallel::~parallel()
{
	chp = "";
	_kind = "parallel";

	instrs.clear();
}

void parallel::expand_shortcuts()
{

}

void parallel::parse(map<string, keyword*> types)
{
	if (verbosity >= VERB_PARSE)
		cout << tab << "Parallel: " << chp << endl;


	string		raw_instr;	// chp of a sub block

	instruction *instr; 	// instruction parser
	variable	v;			// variable instantiation parser

	map<string, state> current_state;

	list<instruction*>		::iterator	ii, ij;
	map<string, variable>	::iterator	vi;
	//map<string, space>		::iterator	si, sj, sk;
	map<string, state>		::iterator	l, m;
	list<state>				::iterator	a, b;
	map<string, keyword*>	::iterator	t;
	list<bool>				::iterator	di;
	string					::iterator	i, j;

	map<string, variable>				affected;
	list<bool>							delta_out;

	bool sequential	= false;
	bool vdef		= false;

	state xstate;
	state tstate;

	// Parse the instructions, making sure to stay in the current scope (outside of any bracket/parenthesis)
	int depth[3] = {0};
	for (i = chp.begin(), j = chp.begin(); i != chp.end()+1; i++)
	{
		if (*i == '(')
			depth[0]++;
		else if (*i == '[')
			depth[1]++;
		else if (*i == '{')
			depth[2]++;
		else if (*i == ')')
			depth[0]--;
		else if (*i == ']')
			depth[1]--;
		else if (*i == '}')
			depth[2]--;

		// We are in the current scope, and the current character
		// is a semicolon or the end of the chp string. This is
		// the end of a block.
		if (depth[0] == 0 && depth[1] == 0 && depth[2] == 0 && ((*i == '|' && *(i+1) == '|') || i == chp.end()))
		{
			// Get the block string.
			raw_instr = chp.substr(j-chp.begin(), i-j);

			instr = NULL;

			// This sub block is a set of parallel sub sub blocks. s0 || s1 || ... || sn
			if (sequential)
				instr = new parallel(raw_instr, types, global, label, tab+"\t", verbosity);
			// This sub block has a specific order of operations. (s)
			else if (raw_instr[0] == '(' && raw_instr[raw_instr.length()-1] == ')')
				instr = new block(raw_instr.substr(1, raw_instr.length()-2), types, global, label, tab+"\t", verbosity);
			// This sub block is a loop. *[g0->s0[]g1->s1[]...[]gn->sn] or *[g0->s0|g1->s1|...|gn->sn]
			else if (raw_instr[0] == '*' && raw_instr[1] == '[' && raw_instr[raw_instr.length()-1] == ']')
				instr = new loop(raw_instr, types, global, label, tab+"\t", verbosity);
			// This sub block is a conditional. [g0->s0[]g1->s1[]...[]gn->sn] or [g0->s0|g1->s1|...|gn->sn]
			else if (raw_instr[0] == '[' && raw_instr[raw_instr.length()-1] == ']')
				instr = new conditional(raw_instr, types, global, label, tab+"\t", verbosity);
			// This sub block is either a variable definition or an assignment instruction.
			else
			{
				vdef = false;
				for (t = types.begin(); t != types.end(); t++)
					if (raw_instr.find(t->first) != raw_instr.npos)
					{
						vdef = true;
						break;
					}

				// This sub block is a variable definition. keyword<bitwidth> name
				if (vdef)
				{
					v = variable(raw_instr, global->size(), tab, verbosity);
					global->insert(pair<string, variable>(v.name, v));
				}
				// This sub block is an assignment instruction.
				else if (raw_instr.length() != 0)
					instr = new assignment(raw_instr, types, global, label, tab+"\t", verbosity);
			}

			if (instr != NULL)
				instrs.push_back(instr);
			j = i+2;
			sequential = false;
		}
		// We are in the current scope, and the current character
		// is a parallel bar or the end of the chp string. This is
		// the middle of a parallel sub block.
		else if (depth[0] == 0 && depth[1] == 0 && depth[2] == 0 && (*i == ';' || i == chp.end()))
			sequential = true;
	}
}

int parallel::generate_states(state_space *space, graph *trans, int init)
{
	cout << tab << "Parallel " << chp << endl;

	list<instruction*>::iterator instr_iter;
	instruction *instr;
	map<string, variable>::iterator vi;
	int state_catcher = -1;
	state s;
	for (vi = global->begin(); vi != global->end(); vi++)
		s.assign(vi->second.uid, value("_"));


	for (instr_iter = instrs.begin(); instr_iter != instrs.end(); instr_iter++)
	{
		instr = *instr_iter;
		instr->generate_states(space, trans, init);
		cout << "Unioning " << s << " and " << (*space)[state_catcher] << endl;
		s = s || (*space)[state_catcher];
	}
	uid = space->size();
	cout << "resulting merge of " << s;
	space->push_back(s);
	return uid;
}

void parallel::generate_prs()
{


}

