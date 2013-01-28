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
parallel::parallel( string chp, map<string, keyword*> *types, map<string, variable*> globals, string tab, int verbosity)
{
	clear();

	_kind = "parallel";
	this->chp = chp;
	this->tab = tab;
	this->verbosity = verbosity;
	this->global = globals;

	expand_shortcuts();
	parse(types);
}
parallel::~parallel()
{
	chp = "";
	_kind = "parallel";

	map<string, variable*>::iterator i;
	for (i = local.begin(); i != local.end(); i++)
	{
		if (i->second != NULL)
			delete i->second;
		i->second = NULL;
	}

	local.clear();

	list<instruction*>::iterator j;
	for (j = instrs.begin(); j != instrs.end(); j++)
	{
		if (*j != NULL)
			delete *j;
		*j = NULL;
	}

	instrs.clear();
}

void parallel::expand_shortcuts()
{

}

void parallel::parse(map<string, keyword*> *types)
{
	if (verbosity >= VERB_PARSE)
		cout << tab << "Parallel: " << chp << endl;


	string		raw_instr;	// chp of a sub block

	instruction *instr; 	// instruction parser
	variable	*v;			// variable instantiation parser

	map<string, state> current_state;

	list<instruction*>		::iterator	ii, ij;
	map<string, variable*>	::iterator	vi;
	//map<string, space>		::iterator	si, sj, sk;
	map<string, state>		::iterator	l, m;
	list<state>				::iterator	a, b;
	map<string, keyword*>	::iterator	t;
	list<bool>				::iterator	di;
	string					::iterator	i, j;

	map<string, variable*>				affected;
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
				instr = new parallel( raw_instr, types, global, tab+"\t", verbosity);
			// This sub block has a specific order of operations. (s)
			else if (raw_instr[0] == '(' && raw_instr[raw_instr.length()-1] == ')')
				instr = new block( raw_instr.substr(1, raw_instr.length()-2), types, global, tab+"\t", verbosity);
			// This sub block is a loop. *[g0->s0[]g1->s1[]...[]gn->sn] or *[g0->s0|g1->s1|...|gn->sn]
			else if (raw_instr[0] == '*' && raw_instr[1] == '[' && raw_instr[raw_instr.length()-1] == ']')
				instr = new loop( raw_instr, types, global, tab+"\t", verbosity);
			// This sub block is a conditional. [g0->s0[]g1->s1[]...[]gn->sn] or [g0->s0|g1->s1|...|gn->sn]
			else if (raw_instr[0] == '[' && raw_instr[raw_instr.length()-1] == ']')
				instr = new conditional( raw_instr, types, global, tab+"\t", verbosity);
			// This sub block is either a variable definition or an assignment instruction.
			else
			{
				vdef = false;
				for (t = types->begin(); t != types->end(); t++)
					if (raw_instr.find(t->first) != raw_instr.npos)
					{
						vdef = true;
						break;
					}

				// This sub block is a variable definition. keyword<bitwidth> name
				if (vdef)
				{
					v = new variable(raw_instr, tab, verbosity);
					local.insert(pair<string, variable*>(v->name, v));
					global.insert(pair<string, variable*>(v->name, v));
				}
				// This sub block is an assignment instruction.
				else if (raw_instr.length() != 0)
					instr = new assignment(raw_instr, types, global, tab+"\t", verbosity);
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

void parallel::generate_states(state init)
{


	list<instruction*>::iterator instr_iter;
	instruction *instr;

	for (instr_iter = instrs.begin(); instr_iter != instrs.end(); instr_iter++)
	{
		instr = *instr_iter;
		instr->generate_states(state());
	}
}

void parallel::generate_prs(map<string, variable*> globals)
{


}

/*
	for (l = init.begin(); l != init.end(); l++)
		affected.insert(pair<string, variable*>(l->first, vars[l->first]));
 */


	/*
				// Now that we have parsed the sub block, we need to
				// check the resulting state space deltas of that sub block.
				// Loop through all of the affected variables.
				delta = false;
				for (l = instr->result.begin(); l != instr->result.end(); l++)
				{
					// If this variable exists, then we check the resultant value against
					// its current bitwidth and adjust the bitwidth to fit the resultant value.
					// We also need to mark whether or not we need to generate a production rule
					// for this instruction.
					vi = global.find(l->first);
					if (vi == global.end() && l->first != "Unhandled")
						cout<< "Error: you are trying to call an instruction that operates on a variable not in this block's scope: " + l->first << endl;
					else if (vi != global.end())
					{
						delta |= ((l->second.prs) && (l->second.data != vi->second->last.data));
						vi->second->last = l->second;
						if (affected.find(vi->first) == affected.end())
							affected.insert(pair<string, variable*>(vi->first, vi->second));
					}
				}
				delta_out.push_back(delta);

				// Fill in the state space based upon the recorded delta values from instruction parsing above.
				// Right now, we X out the input variables when an instruction changes an output value. This will
				// have to be modified in the future so that we only X out the input variables depending upon the
				// communication protocol.
				for(vi = affected.begin(); vi != affected.end(); vi++)
				{
					si = states.find(vi->first);
					if (si == states.end())
					{
						states.insert(pair<string, space>(vi->first, space(vi->first, list<state>())));
						si = states.find(vi->first);
						// The first state for every variable is always all X
						if ((l = init.find(vi->first)) != init.end())
							((space&)si->second).states.push_back(l->second);
						else
							((space&)si->second).states.push_back(state(string(vi->second->width, 'X'), false));
					}
				}
			}*/


/*if (verbosity >= VERB_PARSE)
		cout << endl;

	for (ii = instrs.begin(); ii != instrs.end(); ii++)
	{
		// TODO We need to check to see that every instruction's variable space is mutually exclusive here
		// If it isn't mutually exclusive, then it violates the no shared variables and non-interference rules
		/ *ij = ii;
		ij++;

		for (; ij != instrs.end(); ij++)
			for (l = (*ij)->result.begin(); l != (*ij)->result.end(); l++)
				if ((*ii)->result.find(l->first) != (*ii)->result.end())
					cout << "Error: Shared variable " << l->first << " violates non-interference: " << chp << endl;* /

		//Loop through all variables affected by these instructions
		for (l = (*ii)->result.begin(); l != (*ii)->result.end(); l++)
		{
			m = result.find(l->first);
			//If this variable hasn't been seen yet...
			if (m == result.end())
				result.insert(pair<string, state>(l->first, l->second));
			else
				m->second = m->second || l->second;
		}
	}

	if (verbosity >= VERB_PARSE)
	{
		cout << tab << "Result:\t";
		for (l = result.begin(); l != result.end(); l++)
			cout << "{" << l->first << " = " << l->second << "} ";
		cout << endl;
	}*/
