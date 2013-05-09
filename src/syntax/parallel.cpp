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
parallel::parallel(instruction *parent, string chp, vspace *vars, string tab, int verbosity)
{
	clear();

	_kind = "parallel";
	this->chp = chp;
	this->tab = tab;
	this->verbosity = verbosity;
	this->vars = vars;
	this->parent = parent;

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
	this->vars		= p.vars;
	this->space		= p.space;
	this->tab		= p.tab;
	this->verbosity	= p.verbosity;
	this->parent	= p.parent;
	return *this;
}

/* This copies a guard to another process and replaces
 * all of the specified variables.
 */
instruction *parallel::duplicate(instruction *parent, vspace *vars, map<string, string> convert, string tab, int verbosity)
{
	parallel *instr;

	instr 				= new parallel();
	instr->chp			= this->chp;
	instr->vars			= vars;
	instr->tab			= tab;
	instr->verbosity	= verbosity;
	instr->parent		= parent;

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
		instr->instrs.push_back((*l)->duplicate(instr, vars, convert, tab+"\t", verbosity));

	return instr;
}

state parallel::variant()
{
	state result(value("_"), vars->global.size());

	list<instruction*>::iterator i;
	for (i = instrs.begin(); i != instrs.end(); i++)
		result = result || (*i)->variant();

	return result;
}

state parallel::active_variant()
{
	state result(value("_"), vars->global.size());

	list<instruction*>::iterator i;
	for (i = instrs.begin(); i != instrs.end(); i++)
		result = result || (*i)->active_variant();

	return result;
}

state parallel::passive_variant()
{
	state result(value("_"), vars->global.size());

	list<instruction*>::iterator i;
	for (i = instrs.begin(); i != instrs.end(); i++)
		result = result || (*i)->passive_variant();

	return result;
}

void parallel::expand_shortcuts()
{

}

void parallel::parse()
{
	if (verbosity & VERB_BASE_HSE && verbosity & VERB_DEBUG)
		cout << tab << "Parallel: " << chp << endl;

	string				raw_instr;	// chp of a sub sequential
	string::iterator	i, j;
	bool				semicolon = false;
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
		// the end of a sequential.
		if (depth[0] == 0 && depth[1] == 0 && depth[2] == 0 && ((*i == '|' && *(i+1) == '|') || i == chp.end()))
		{
			// Get the sequential string.
			raw_instr = chp.substr(j-chp.begin(), i-j);

			// This sub sequential is a set of parallel sub sub sequentials. s0 || s1 || ... || sn
			if (semicolon && raw_instr.length() > 0)
				push(new sequential(this, raw_instr, vars, tab+"\t", verbosity));
			// This sub sequential has a specific order of operations. (s)
			else if (raw_instr[0] == '(' && raw_instr[raw_instr.length()-1] == ')' && raw_instr.length() > 0)
				push(new sequential(this, raw_instr.substr(1, raw_instr.length()-2), vars, tab+"\t", verbosity));
			// This sub sequential is a loop. *[g0->s0[]g1->s1[]...[]gn->sn] or *[g0->s0|g1->s1|...|gn->sn]
			else if (raw_instr[0] == '*' && raw_instr[1] == '[' && raw_instr[raw_instr.length()-1] == ']' && raw_instr.length() > 0)
				push(new loop(this, raw_instr, vars, tab+"\t", verbosity));
			// This sub sequential is a conditional. [g0->s0[]g1->s1[]...[]gn->sn] or [g0->s0|g1->s1|...|gn->sn]
			else if (raw_instr[0] == '[' && raw_instr[raw_instr.length()-1] == ']' && raw_instr.length() > 0)
				push(new conditional(this, raw_instr, vars, tab+"\t", verbosity));
			// This sub sequential is a variable definition. keyword<bitwidth> name
			else if (vars->vdef(raw_instr) && raw_instr.length() > 0)
				push(expand_instantiation(this, raw_instr, vars, NULL, tab+"\t", verbosity, true));
			else if ((k = raw_instr.find_first_of("?!@")) != raw_instr.npos && raw_instr.find(":=") == raw_instr.npos && raw_instr.length() > 0)
				push(add_unique_variable(this, raw_instr.substr(0, k) + "._fn", "(" + (k+1 < raw_instr.length() ? raw_instr.substr(k+1) : "") + ")", vars->get_type(raw_instr.substr(0, k)) + ".operator" + raw_instr[k] + "()", vars, tab+"\t", verbosity).second);
			// This sub sequential is an assignment instruction.
			else if ((raw_instr.find(":=") != raw_instr.npos || raw_instr[raw_instr.length()-1] == '+' || raw_instr[raw_instr.length()-1] == '-') && raw_instr.length() > 0)
				push(expand_assignment(this, raw_instr, vars, tab+"\t", verbosity));
			else if (raw_instr.find("skip") == raw_instr.npos && raw_instr.length() > 0)
				push(new guard(this, raw_instr, vars, tab, verbosity));

			j = i+2;
			semicolon = false;
		}
		// We are in the current scope, and the current character
		// is a parallel bar or the end of the chp string. This is
		// the middle of a parallel sub sequential.
		else if (depth[0] == 0 && depth[1] == 0 && depth[2] == 0 && (*i == ';' || i == chp.end()))
			semicolon = true;
	}
}

void parallel::merge()
{
	list<instruction*>::iterator i;
	for (i = instrs.begin(); i != instrs.end(); i++)
		(*i)->merge();
}

int parallel::generate_states(graph *g, int init, state filter)
{
	list<instruction*>::iterator i, j;
	instruction *instr;
	map<string, variable>::iterator vi;
	vector<int> state_catcher;
	vector<string> chp_catcher;
	state s;
	bool first = true;
	state v;
	int k;

	if (verbosity & VERB_BASE_STATE_SPACE && verbosity & VERB_DEBUG)
		cout << tab << "Parallel " << chp << endl;

	space = g;
	from = init;
	for (i = instrs.begin(); i != instrs.end(); i++)
	{
		if (filter.size() > 0)
			v = filter;
		else
			v = state(value("_"), vars->global.size());

		for (j = instrs.begin(); j != instrs.end(); j++)
			if (i != j)
				v = v || (*j)->active_variant();

		instr = *i;
		state_catcher.push_back(instr->generate_states(g, init, v));
		if(CHP_EDGE)
			chp_catcher.push_back(instr->chp);
		else
			chp_catcher.push_back("Parallel");
		if (first)
		{
			s = g->states[state_catcher.back()];
			first = false;
		}
		else
			s = s || g->states[state_catcher.back()];
	}

	if (filter.size() > 0)
		s = s || filter;

	uid = g->states.size();

	for (k = 0; k < instrs.size(); k++)
		recursive_branch_set(g, g->front_edges[init][k], pair<int, int>(uid, k));

	g->append_state(s, state_catcher, chp_catcher);

	if (verbosity & VERB_BASE_STATE_SPACE && verbosity & VERB_DEBUG)
		cout << tab << s << endl;

	return uid;
}

state parallel::simulate_states(state init, state filter)
{
	list<instruction*>::iterator i, j;
	instruction *instr;
	state s;
	state v;

	if (filter.size() == 0)
		filter = null(vars->size());

	s = filter;
	for (i = instrs.begin(); i != instrs.end(); i++)
	{
		v = filter;
		for (j = instrs.begin(); j != instrs.end(); j++)
			if (i != j)
				v = v || (*j)->active_variant();

		instr = *i;
		s = s || instr->simulate_states(init, v);
	}

	return s;
}

void parallel::recursive_branch_set(graph *g, int from, pair<int, int> id)
{
	int i;

	if (g->states[from].branch.find(id.first) != g->states[from].branch.end())
		return;
	g->states[from].branch.insert(id);

	while (g->front_edges[from].size() == 1)
	{
		if (g->states[g->front_edges[from].front()].branch.find(id.first) != g->states[g->front_edges[from].front()].branch.end())
			return;
		g->states[g->front_edges[from].front()].branch.insert(id);
		from = g->front_edges[from].front();
	}

	for (i = 0; from < (int)g->front_edges.size() && i < (int)g->front_edges[from].size(); i++)
		if (g->states[g->front_edges[from][i]].branch.find(id.first) == g->states[g->front_edges[from][i]].branch.end())
			recursive_branch_set(g, g->front_edges[from][i], id);
}

void parallel::generate_scribes()
{
	list<instruction*>::iterator i;
	for (i = instrs.begin(); i != instrs.end(); i++)
		(*i)->generate_scribes();
}

void parallel::insert_instr(int uid, int nid, instruction *instr)
{
	instr->uid = nid;
	instr->from = uid;

	sequential *b;
	instruction* j;
	list<instruction*>::iterator i, k;
	for (i = instrs.begin(); i != instrs.end(); i++)
	{
		j = *i;
		if (j->uid == uid)
		{
			if (j->kind() != "sequential")
			{
				b = new sequential();
				b->from = j->from;
				b->instrs.push_back(j);
				b->chp = j->chp + ";" + instr->chp;
				b->parent = this;
				b->space = space;
				b->vars = vars;
				b->verbosity = verbosity;
				b->tab = tab;
				instrs.remove(*i);
				instrs.push_back(b);
			}
			else
				b = (sequential*)j;

			b->instrs.push_back(instr);
			b->uid = nid;
			return;
		}
	}

	for (i = instrs.begin(); i != instrs.end(); i++)
	{
		j = *i;
		if (j->kind() == "parallel")
			((parallel*)j)->insert_instr(uid, nid, instr);
		else if (j->kind() == "loop")
			((loop*)j)->insert_instr(uid, nid, instr);
		else if (j->kind() == "conditional")
			((conditional*)j)->insert_instr(uid, nid, instr);
		else if (j->kind() == "guard")
			((guard*)j)->insert_instr(uid, nid, instr);
		else if (j->kind() == "sequential")
			((sequential*)j)->insert_instr(uid, nid, instr);
		else if (j->kind() == "assignment")
			((assignment*)j)->insert_instr(uid, nid, instr);
	}
}

void parallel::print_hse(string t)
{
	if (instrs.size() > 1)
	{
		cout << "\n" << t << "(\n" << t + "\t";
		list<instruction*>::iterator i;
		for (i = instrs.begin(); i != instrs.end(); i++)
		{
			if (i != instrs.begin())
				cout << "||\n\t" << t;
			(*i)->print_hse(t + "\t");
		}
		cout << "\n" << t << ")";
	}
	else if (instrs.size() == 1)
	{
		cout << t;
		instrs.front()->print_hse(t);
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
			instrs.push_back(i);
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
		instrs.push_back(i);
}

