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
#include "record.h"

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

	string				raw_instr;	// chp of a sub block
	instruction			*instr; 	// instruction parser
	string::iterator	i, j;
	bool				sequential = false;
	int					depth[3] = {0};

	// Parse the instructions, making sure to stay in the current scope (outside of any bracket/parenthesis)
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
				instr = new block(raw_instr, types, global, label, tab+"\t", verbosity);
			// This sub block has a specific order of operations. (s)
			else if (raw_instr[0] == '(' && raw_instr[raw_instr.length()-1] == ')')
				instr = new block(raw_instr.substr(1, raw_instr.length()-2), types, global, label, tab+"\t", verbosity);
			// This sub block is a loop. *[g0->s0[]g1->s1[]...[]gn->sn] or *[g0->s0|g1->s1|...|gn->sn]
			else if (raw_instr[0] == '*' && raw_instr[1] == '[' && raw_instr[raw_instr.length()-1] == ']')
				instr = new loop(raw_instr, types, global, label, tab+"\t", verbosity);
			// This sub block is a conditional. [g0->s0[]g1->s1[]...[]gn->sn] or [g0->s0|g1->s1|...|gn->sn]
			else if (raw_instr[0] == '[' && raw_instr[raw_instr.length()-1] == ']')
				instr = new conditional(raw_instr, types, global, label, tab+"\t", verbosity);
			// This sub block is a variable definition. keyword<bitwidth> name
			else if (contains(raw_instr, types))
				expand(raw_instr, types, global, label, tab+"\t", verbosity);
			// This sub block is an assignment instruction.
			else if (raw_instr.length() != 0 && raw_instr.find("skip") == raw_instr.npos)
				instr = new assignment(raw_instr, types, global, label, tab+"\t", verbosity);

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
	vector<int> state_catcher;
	state s;


	for (vi = global->begin(); vi != global->end(); vi++)
		s.assign(vi->second.uid, value("_"));

	for (instr_iter = instrs.begin(); instr_iter != instrs.end(); instr_iter++)
	{
		instr = *instr_iter;
		state_catcher.push_back(instr->generate_states(space, trans, init));
		if (state_catcher.back() != -1)
			s = s || (*space)[state_catcher.back()];
		else
			state_catcher.pop_back();
	}
	uid = space->size();

	space->push_back(s);

	for (int i = 0; i < (int)state_catcher.size(); i++)
		trans->insert_edge(state_catcher[i], uid, chp);

	return uid;
}

void parallel::generate_prs()
{


}

