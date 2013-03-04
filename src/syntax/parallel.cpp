/*
 * parallel.cpp
 *
 *  Created on: Nov 1, 2012
 *      Author: Ned Bingham and Nicholas Kramer
 *
 *  DO NOT DISTRIBUTE
 */

#include "../common.h"
#include "../utility.h"
#include "../type.h"
#include "../data.h"

#include "parallel.h"
#include "conditional.h"
#include "loop.h"

parallel::parallel()
{
	chp = "";
	_kind = "parallel";
}
parallel::parallel(string chp, vspace *vars, string tab, int verbosity)
{
	clear();

	_kind = "parallel";
	this->chp = chp;
	this->tab = tab;
	this->verbosity = verbosity;
	this->vars = vars;

	expand_shortcuts();
	parse();
}
parallel::~parallel()
{
	chp = "";
	_kind = "parallel";

	instrs.clear();
}

parallel &parallel::operator=(parallel p)
{
	this->uid		= p.uid;
	this->chp		= p.chp;
	this->instrs	= p.instrs;
	this->rules		= p.rules;
	this->vars		= p.vars;
	this->tab		= p.tab;
	this->verbosity	= p.verbosity;
	return *this;
}

/* This copies a guard to another process and replaces
 * all of the specified variables.
 */
instruction *parallel::duplicate(vspace *vars, map<string, string> convert, string tab, int verbosity)
{
	parallel *instr;

	instr 				= new parallel();
	instr->chp			= this->chp;
	instr->vars			= vars;
	instr->tab			= tab;
	instr->verbosity	= verbosity;

	size_t idx;
	string rep;

	map<string, string>::iterator i, j;
	size_t k = 0, min, curr;
	while (k != instr->chp.npos)
	{
		j = convert.end();
		min = instr->chp.length();
		curr = 0;
		for (i = convert.begin(); i != convert.end(); i++)
		{
			curr = find_name(instr->chp, i->first, k);
			if (curr < min)
			{
				min = curr;
				j = i;
			}
		}

		if (j != convert.end())
		{
			rep = j->second;
			instr->chp.replace(min, j->first.length(), rep);
			if (instr->chp[min + rep.length()] == '[' && instr->chp[min + rep.length()-1] == ']')
			{
				idx = instr->chp.find_first_of("]", min + rep.length()) + 1;
				rep = flatten_slice(instr->chp.substr(min, idx - min));
				instr->chp.replace(min, idx - min, rep);
			}

			k = min + rep.length();
		}
		else
			k = instr->chp.npos;
	}

	list<instruction*>::iterator l;
	for (l = instrs.begin(); l != instrs.end(); l++)
		instr->instrs.push_back((*l)->duplicate(vars, convert, tab+"\t", verbosity));

	return instr;
}

void parallel::expand_shortcuts()
{

}

void parallel::parse()
{
	if (verbosity >= VERB_PARSE)
		cout << tab << "Parallel: " << chp << endl;

	string				raw_instr;	// chp of a sub block
	string::iterator	i, j;
	bool				sequential = false;
	int					depth[3] = {0};
	size_t				k;

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

			// This sub block is a set of parallel sub sub blocks. s0 || s1 || ... || sn
			if (sequential && raw_instr.length() > 0)
				push(new block(raw_instr, vars, tab+"\t", verbosity));
			// This sub block has a specific order of operations. (s)
			else if (raw_instr[0] == '(' && raw_instr[raw_instr.length()-1] == ')' && raw_instr.length() > 0)
				push(new block(raw_instr.substr(1, raw_instr.length()-2), vars, tab+"\t", verbosity));
			// This sub block is a loop. *[g0->s0[]g1->s1[]...[]gn->sn] or *[g0->s0|g1->s1|...|gn->sn]
			else if (raw_instr[0] == '*' && raw_instr[1] == '[' && raw_instr[raw_instr.length()-1] == ']' && raw_instr.length() > 0)
				push(new loop(raw_instr, vars, tab+"\t", verbosity));
			// This sub block is a conditional. [g0->s0[]g1->s1[]...[]gn->sn] or [g0->s0|g1->s1|...|gn->sn]
			else if (raw_instr[0] == '[' && raw_instr[raw_instr.length()-1] == ']' && raw_instr.length() > 0)
				push(new conditional(raw_instr, vars, tab+"\t", verbosity));
			// This sub block is a variable definition. keyword<bitwidth> name
			else if (vars->vdef(raw_instr) && raw_instr.length() > 0)
				push(expand_instantiation(raw_instr, vars, NULL, tab+"\t", verbosity, true));
			else if ((k = raw_instr.find_first_of("?!@")) != raw_instr.npos && raw_instr.find(":=") == raw_instr.npos && raw_instr.length() > 0)
				push(add_unique_variable("_fn", "(" + (k+1 < raw_instr.length() ? raw_instr.substr(k+1) : "") + ")", vars->get_type(raw_instr.substr(0, k)) + ".operator" + raw_instr[k] + "()", vars, tab+"\t", verbosity).second);
			// This sub block is an assignment instruction.
			else if ((raw_instr.find(":=") != raw_instr.npos || raw_instr[raw_instr.length()-1] == '+' || raw_instr[raw_instr.length()-1] == '-') && raw_instr.length() > 0)
				push(expand_assignment(raw_instr, vars, tab+"\t", verbosity));
			else if (raw_instr.find("skip") == raw_instr.npos && raw_instr.length() > 0)
				push(new guard(raw_instr, vars, tab, verbosity));

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

int parallel::generate_states(graph *trans, int init)
{
	cout << tab << "Parallel " << chp << endl;

	list<instruction*>::iterator instr_iter;
	instruction *instr;
	map<string, variable>::iterator vi;
	vector<int> state_catcher;
	state s;
	bool first = true;

	// TODO X out variables modified in other branches of the parallel
	for (instr_iter = instrs.begin(); instr_iter != instrs.end(); instr_iter++)
	{
		instr = *instr_iter;
		state_catcher.push_back(instr->generate_states(trans, init));
		if (first)
		{
			s = trans->states[state_catcher.back()];
			first = false;
		}
		else
			s = s || trans->states[state_catcher.back()];
	}
	uid = trans->states.size();

	trans->push_back(s);

	int i = 0;
	for (i = 0, instr_iter = instrs.begin(); i < (int)state_catcher.size() && instr_iter != instrs.end(); i++, instr_iter++)
	{
		if (CHP_EDGE)
			trans->insert_edge(state_catcher[i], uid, (*instr_iter)->chp);
		else
			trans->insert_edge(state_catcher[i], uid, "Parallel merge");
	}

	return uid;
}

void parallel::generate_prs()
{


}

void parallel::print_hse()
{
	cout << "\n" << tab << "(\n";
	list<instruction*>::iterator i;
	for (i = instrs.begin(); i != instrs.end(); i++)
	{
		if (i != instrs.begin())
			cout << "||";
		(*i)->print_hse();
	}
	cout << "\n" << tab << ")";
}

void parallel::push(instruction *i)
{
	if (i == NULL)
		return;

	list<instruction*>::iterator j;
	if (i->kind() == "block")
	{
		if (((block*)i)->instrs.size() <= 1)
		{
			for (j = ((block*)i)->instrs.begin(); j != ((block*)i)->instrs.end(); j++)
				push(*j);
			((block*)i)->instrs.clear();
			delete (block*)i;
		}
		else
			instrs.push_back(i);
	}
	else if (i->kind() == "parallel")
	{
		for (j = ((parallel*)i)->instrs.begin(); j != ((parallel*)i)->instrs.end(); j++)
			push(*j);
		((parallel*)i)->instrs.clear();
		delete (parallel*)i;
	}
	else
		instrs.push_back(i);
}

