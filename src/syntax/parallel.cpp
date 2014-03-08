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
#include "condition.h"
#include "loop.h"
#include "debug.h"
#include "skip.h"

parallel::parallel()
{
	chp = "";
	_kind = "parallel";
}
parallel::parallel(instruction *parent, sstring chp, variable_space *vars, flag_space *flags)
{
	clear();

	_kind = "parallel";
	this->chp = chp;
	this->flags = flags;
	this->vars = vars;
	this->parent = parent;

	expand_shortcuts();
	parse();
}
parallel::~parallel()
{
	chp = "";
	_kind = "parallel";
}

/* This copies a guard to another process and replaces
 * all of the specified variables.
 */
instruction *parallel::duplicate(instruction *parent, variable_space *vars, smap<sstring, sstring> convert)
{
	parallel *instr;

	instr 				= new parallel();
	instr->chp			= this->chp;
	instr->vars			= vars;
	instr->flags		= flags;
	instr->parent		= parent;

	int idx;
	sstring rep;

	smap<sstring, sstring>::iterator i, j;
	int k = 0, min, curr;
	while (k != instr->chp.npos)
	{
		j = convert.end();
		min = instr->chp.length();
		curr = 0;
		for (i = convert.begin(); i != convert.end(); i++)
		{
			curr = find_name(instr->chp, i->first, k);
			if (curr < min && curr >= 0)
			{
				min = curr;
				j = i;
			}
		}

		if (j != convert.end())
		{
			rep = j->second;
			cout << "Replacing " << instr->chp << " " << min << " " << j->first << " " << rep << endl;
			instr->chp.replace(min, j->first.length(), rep);
			cout << " done" << endl;
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

	cout << "Recursing" << endl;
	list<instruction*>::iterator l;
	for (l = instrs.begin(); l != instrs.end(); l++)
		instr->instrs.push_back((*l)->duplicate(instr, vars, convert));

	return instr;
}

void parallel::expand_shortcuts()
{

}

void parallel::parse()
{
	sstring				raw_instr;	// chp of a sub sequential
	sstring::iterator	i, j;
	bool				semicolon = false;
	int					depth[3] = {0};
	int				k;

	flags->inc();
	if (flags->log_base_hse())
		(*flags->log_file) << flags->tab << "Parallel: " << chp << endl;

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
		// is a semicolon or the end of the chp sstring. This is
		// the end of a sequential.
		if (depth[0] == 0 && depth[1] == 0 && depth[2] == 0 && ((*i == '|' && *(i+1) == '|') || *i == ',' || i == chp.end()))
		{
			// Get the sequential sstring.
			raw_instr = chp.substr(j-chp.begin(), i-j);

			// This sub sequential is a set of parallel sub sub sequentials. s0 || s1 || ... || sn
			if (semicolon && raw_instr.length() > 0)
				push(new sequential(this, raw_instr, vars, flags));
			// This sub sequential has a specific order of operations. (s)
			else if (raw_instr[0] == '(' && raw_instr[raw_instr.length()-1] == ')' && raw_instr.length() > 0)
				push(new sequential(this, raw_instr.substr(1, raw_instr.length()-2), vars, flags));
			// This sub sequential is a loop. *[g0->s0[]g1->s1[]...[]gn->sn] or *[g0->s0|g1->s1|...|gn->sn]
			else if (raw_instr[0] == '*' && raw_instr[1] == '[' && raw_instr[raw_instr.length()-1] == ']' && raw_instr.length() > 0)
				push(expand_loop(raw_instr));
			// This sub sequential is a condition. [g0->s0[]g1->s1[]...[]gn->sn] or [g0->s0|g1->s1|...|gn->sn]
			else if (raw_instr[0] == '[' && raw_instr[raw_instr.length()-1] == ']' && raw_instr.length() > 0)
				push(expand_condition(raw_instr));
			// This sub sequential is a variable definition. keyword<bitwidth> name
			else if (vars->vdef(raw_instr) && raw_instr.length() > 0)
				push(expand_instantiation(this, raw_instr, vars, NULL, flags, true));
			else if ((k = raw_instr.find_first_of("?!#")) != raw_instr.npos && raw_instr.find(":=") == raw_instr.npos && raw_instr.length() > 0)
				push(add_unique_variable(this, raw_instr.substr(0, k) + "._fn", "(" + (k+1 < raw_instr.length() ? raw_instr.substr(k+1) : "") + ")", vars->get_type(raw_instr.substr(0, k)) + ".operator" + raw_instr[k] + "()", vars, flags).second);
			// This sub sequential is an assignment instruction.
			else if ((raw_instr.find(":=") != raw_instr.npos || raw_instr[raw_instr.length()-1] == '+' || raw_instr[raw_instr.length()-1] == '-') && raw_instr.length() > 0)
				push(expand_assignment(raw_instr));
			else if (raw_instr == "skip")
				push(new skip(this, raw_instr, vars, flags));
			else if (raw_instr.length() > 0)
				push(new guard(this, raw_instr, vars, flags));

			j = i+2;
			semicolon = false;
		}
		// We are in the current scope, and the current character
		// is a parallel bar or the end of the chp sstring. This is
		// the middle of a parallel sub sequential.
		else if (depth[0] == 0 && depth[1] == 0 && depth[2] == 0 && (*i == ';' || i == chp.end()))
			semicolon = true;
	}

	flags->dec();
}

void parallel::simulate()
{

}

void parallel::rewrite()
{
	list<instruction*>::iterator i;
	for (i = instrs.begin(); i != instrs.end(); i++)
		(*i)->rewrite();
}

void parallel::reorder()
{

}

svector<petri_index> parallel::generate_states(petri_net *n, rule_space *p, svector<petri_index> f, smap<int, int> pbranch, smap<int, int> cbranch)
{
	list<instruction*>::iterator i, j;
	svector<petri_index> next, end;
	svector<petri_index> allends;
	smap<int, int> npbranch;
	int npbranch_count;

	if (flags->log_base_state_space())
		(*flags->log_file) << flags->tab << "Parallel " << chp << endl;

	net  = n;
	prs = p;
	petri_index l = net->push_transition(f, pbranch, cbranch, this);
	from = svector<petri_index>(1, net->push_place(l, pbranch, cbranch, this));

	npbranch_count = net->pbranch_count;
	net->pbranch_count++;
	int k;
	for (i = instrs.begin(), k = instrs.size()-1; i != instrs.end(); i++, k--)
	{
		npbranch = pbranch;
		if (instrs.size() > 1)
			npbranch.insert(pair<int, int>(npbranch_count, (int)k));

		next.clear();
		next = (k == 0 ? from : net->duplicate(from));
		for (int l = 0; l < (int)next.size(); l++)
			net->at(next[l]).pbranch = npbranch;

		end = (*i)->generate_states(net, prs, next, npbranch, cbranch);
		allends.push_back(net->push_place(end, npbranch, cbranch, this));
	}
	uid.push_back(net->push_transition(allends, pbranch, cbranch, this));

	return uid;
}

void parallel::print_hse(sstring t, ostream *fout)
{
	if (instrs.size() > 1)
	{
		(*fout) << endl << t << "(" << endl << t + "\t";
		list<instruction*>::iterator i;
		for (i = instrs.begin(); i != instrs.end(); i++)
		{
			if (i != instrs.begin())
				(*fout) << "||" << endl << "\t" << t;
			(*i)->print_hse(t + "\t", fout);
		}
		(*fout) << endl << t << ")";
	}
	else if (instrs.size() == 1)
	{
		(*fout) << t;
		instrs.front()->print_hse(t, fout);
	}
}

void parallel::push(instruction *i)
{
	if (i == NULL)
		return;

	list<instruction*>::iterator j;
	if (i->kind() == "sequential")
	{
		if (((sequential*)i)->instrs.size() <= 1)
		{
			for (j = ((sequential*)i)->instrs.begin(); j != ((sequential*)i)->instrs.end(); j++)
			{
				(*j)->parent = this;
				push(*j);
			}
			((sequential*)i)->instrs.clear();
			delete (sequential*)i;
		}
		else
		{
			i->parent = this;
			instrs.push_back(i);
		}
	}
	else if (i->kind() == "parallel")
	{
		for (j = ((parallel*)i)->instrs.begin(); j != ((parallel*)i)->instrs.end(); j++)
		{
			(*j)->parent = this;
			push(*j);
		}
		((parallel*)i)->instrs.clear();
		delete (parallel*)i;
	}
	else
	{
		i->parent = this;
		instrs.push_back(i);
	}
}

