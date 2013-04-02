/*
 * block.cpp
 *
 *  Created on: Oct 29, 2012
 *      Author: Ned Bingham and Nicholas Kramer
 *
 *  DO NOT DISTRIBUTE
 */

#include "../common.h"
#include "../utility.h"
#include "../type.h"
#include "../data.h"

#include "block.h"
#include "parallel.h"
#include "conditional.h"
#include "loop.h"

block::block()
{
	_kind = "block";
}

block::block(instruction *parent, string chp, vspace *vars, string tab, int verbosity)
{
	clear();

	this->_kind = "block";
	this->chp = chp;
	this->tab = tab;
	this->verbosity = verbosity;
	this->vars = vars;
	this->parent = parent;

	expand_shortcuts();
	parse();
}

block::~block()
{
	_kind = "block";

	list<instruction*>::iterator j;
	for (j = instrs.begin(); j != instrs.end(); j++)
	{
		if (*j != NULL)
			delete *j;
		*j = NULL;
	}

	instrs.clear();
}

block &block::operator=(block b)
{
	this->chp		= b.chp;
	this->instrs	= b.instrs;
	this->vars		= b.vars;
	this->space		= b.space;
	this->tab		= b.tab;
	this->verbosity	= b.verbosity;
	this->parent	= b.parent;
	return *this;
}

void block::init(string chp, vspace *vars, string tab, int verbosity)
{
	clear();

	this->_kind = "block";
	this->chp = chp;
	this->tab = tab;
	this->verbosity = verbosity;
	this->vars = vars;

	expand_shortcuts();
	parse();
}

/* This copies a guard to another process and replaces
 * all of the specified variables.
 */
instruction *block::duplicate(instruction *parent, vspace *vars, map<string, string> convert, string tab, int verbosity)
{
	block *instr;

	instr 				= new block();
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

state block::variant()
{
	state result(value("_"), vars->global.size());

	list<instruction*>::iterator i;
	for (i = instrs.begin(); i != instrs.end(); i++)
		result = result || (*i)->variant();

	return result;
}

state block::active_variant()
{
	state result(value("_"), vars->global.size());

	list<instruction*>::iterator i;
	for (i = instrs.begin(); i != instrs.end(); i++)
		result = result || (*i)->active_variant();

	return result;
}

state block::passive_variant()
{
	state result(value("_"), vars->global.size());

	list<instruction*>::iterator i;
	for (i = instrs.begin(); i != instrs.end(); i++)
		result = result || (*i)->passive_variant();

	return result;
}

void block::expand_shortcuts()
{
}

void block::parse()
{
	if (verbosity & VERB_BASE_HSE && verbosity & VERB_DEBUG)
		cout << tab << "Block: " << chp << endl;

	string				raw_instr;	// chp of a sub block
	string::iterator	i, j;
	bool				para = false;
	int					depth[3] = {0};
	size_t				k;

	// Parse the instructions, making sure to stay in the current scope (outside of any bracket/parenthesis)
	for (i = chp.begin(), j = chp.begin(); i != chp.end()+1; i++)
	{
		if (i != chp.end())
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
		}

		// We are in the current scope, and the current character
		// is a semicolon or the end of the chp string. This is
		// the end of an instruction.
		if (depth[0] == 0 && depth[1] == 0 && depth[2] == 0 && (*i == ';' || i == chp.end()))
		{
			// Get the instruction string.
			raw_instr = chp.substr(j-chp.begin(), i-j);

			// This sub block is a set of parallel sub sub blocks. s0 || s1 || ... || sn
			if (para && raw_instr.length() > 0)
				push(new parallel(this, raw_instr, vars, tab+"\t", verbosity));
			// This sub block has a specific order of operations. (s)
			else if (raw_instr[0] == '(' && raw_instr[raw_instr.length()-1] == ')' && raw_instr.length() > 0)
				push(new block(this, raw_instr.substr(1, raw_instr.length()-2), vars, tab+"\t", verbosity));
			// This sub block is a loop. *[g0->s0[]g1->s1[]...[]gn->sn] or *[g0->s0|g1->s1|...|gn->sn]
			else if (raw_instr[0] == '*' && raw_instr[1] == '[' && raw_instr[raw_instr.length()-1] == ']' && raw_instr.length() > 0)
				push(new loop(this, raw_instr, vars, tab+"\t", verbosity));
			// This sub block is a conditional. [g0->s0[]g1->s1[]...[]gn->sn] or [g0->s0|g1->s1|...|gn->sn]
			else if (raw_instr[0] == '[' && raw_instr[raw_instr.length()-1] == ']' && raw_instr.length() > 0)
				push(new conditional(this, raw_instr, vars, tab+"\t", verbosity));
			// This sub block is a variable instantiation.
			else if (vars->vdef(raw_instr) && raw_instr.length() > 0)
				push(expand_instantiation(this, raw_instr, vars, NULL, tab+"\t", verbosity, true));
			// This sub block is a communication instantiation.
			else if ((k = raw_instr.find_first_of("?!@")) != raw_instr.npos && raw_instr.find(":=") == raw_instr.npos && raw_instr.length() > 0)
				push(add_unique_variable(this, raw_instr.substr(0, k) + "._fn", "(" + (k+1 < raw_instr.length() ? raw_instr.substr(k+1) : "") + ")", vars->get_type(raw_instr.substr(0, k)) + ".operator" + raw_instr[k] + "()", vars, tab, verbosity).second);
			// This sub block is an assignment instruction.
			else if ((raw_instr.find(":=") != raw_instr.npos || raw_instr[raw_instr.length()-1] == '+' || raw_instr[raw_instr.length()-1] == '-') && raw_instr.length() > 0)
				push(expand_assignment(this, raw_instr, vars, tab+"\t", verbosity));
			else if (raw_instr.find("skip") == raw_instr.npos && raw_instr.length() > 0)
				push(new guard(this, raw_instr, vars, tab, verbosity));

			j = i+1;
			para = false;
		}
		// We are in the current scope, and the current character
		// is a parallel bar or the end of the chp string. This is
		// the middle of a parallel sub block.
		else if (depth[0] == 0 && depth[1] == 0 && depth[2] == 0 && ((*i == '|' && *(i+1) == '|') || i == chp.end()))
			para = true;
	}
}

void block::merge()
{
	list<instruction*>::iterator i, j;
	list<pair<block*, guard*> >::iterator k;
	list<pair<string, string> >::iterator m, n;
	i = instrs.begin();
	j = instrs.begin();
	conditional *ic, *jc;
	assignment *ia, *ja;
	int conflict_count;

	for (j++; j != instrs.end(); j++)
	{
		if ((*i)->kind() == "conditional" && (*j)->kind() == "conditional")
		{
			ic = (conditional*)*i;
			jc = (conditional*)*j;

			if (ic->instrs.size() == 1 && ic->instrs.front().first->instrs.size() == 0)
			{
				for (k = jc->instrs.begin(); k != jc->instrs.end(); k++)
					k->second->chp = expression("(" + ic->instrs.front().second->chp + ")&(" + k->second->chp + ")").simple;
				instrs.remove(*i);
				//delete (conditional*)(*i);
			}
		}
		else if ((*i)->kind() == "assignment" && (*j)->kind() == "assignment")
		{
			// TODO We need to check to see if one assignment reads a value of a channel that the other is modifying
			ia = (assignment*)*i;
			ja = (assignment*)*j;

			conflict_count = 0;
			for (m = ia->expr.begin(); m != ia->expr.end(); m++)
				for (n = ja->expr.begin(); n != ja->expr.end(); n++)
					if (m->second.find(n->first) != m->second.npos || n->second.find(m->first) != n->second.npos)
						conflict_count++;

			if (conflict_count == 0)
			{
				ja->expr.merge(ia->expr);
				ja->chp = "";
				for (m = ja->expr.begin(); m != ja->expr.end(); m++)
					ja->chp += (m != ja->expr.begin() ? "," : "") + m->first;
				ja->chp += ":=";
				for (m = ja->expr.begin(); m != ja->expr.end(); m++)
					ja->chp += (m != ja->expr.begin() ? "," : "") + m->second;
				instrs.remove(*i);
				//delete (assignment*)*i;
			}
		}
		i = j;
	}

	for (j = instrs.begin(); j != instrs.end(); j++)
		(*j)->merge();
}

int block::generate_states(graph *g, int init, state filter)
{
	list<instruction*>::iterator instr_iter;
	instruction *instr;

	if (verbosity & VERB_BASE_STATE_SPACE && verbosity & VERB_DEBUG)
		cout << tab << "Block " << chp << endl;

	space = g;
	from = init;
	for (instr_iter = instrs.begin(); instr_iter != instrs.end(); instr_iter++)
	{
		instr = *instr_iter;
		init = instr->generate_states(g, init, filter);
	}

	uid = init;

	return init;
}

state block::simulate_states(state init, state filter)
{
	list<instruction*>::iterator instr_iter;
	instruction *instr;

	for (instr_iter = instrs.begin(); instr_iter != instrs.end(); instr_iter++)
	{
		instr = *instr_iter;
		init = instr->simulate_states(init, filter);
	}

	return init;
}

void block::generate_scribes()
{
	list<instruction*>::iterator i;
	for (i = instrs.begin(); i != instrs.end(); i++)
		(*i)->generate_scribes();
}

/* This function cleans up all of the memory allocated
 * during parsing, and prepares for the next parsing.
 */
void block::clear()
{
	chp = "";
	_kind = "block";

	list<instruction*>::iterator j;
	for (j = instrs.begin(); j != instrs.end(); j++)
	{
		if (*j != NULL)
			delete *j;
		*j = NULL;
	}

	instrs.clear();
}

void block::insert_instr(int uid, int nid, instruction *instr)
{
	instr->uid = nid;
	instr->from = uid;

	instruction *j;
	list<instruction*>::iterator i;
	for (i = instrs.begin(); i != instrs.end(); i++)
	{
		j = *i;
		if (j->uid == uid)
		{
			i++;
			if (i == instrs.end())
				instrs.push_back(instr);
			else
				instrs.insert(i, instr);
			// TODO Set the from field here

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
		else if (j->kind() == "block")
			((block*)j)->insert_instr(uid, nid, instr);
		else if (j->kind() == "assignment")
			((assignment*)j)->insert_instr(uid, nid, instr);
	}
}

void block::print_hse(string t)
{
	list<instruction*>::iterator i;
	for (i = instrs.begin(); i != instrs.end(); i++)
	{
		if (i != instrs.begin())
			cout << ";";
		(*i)->print_hse(t);
	}
}

void block::push(instruction *i)
{
	if (i == NULL)
		return;

	list<instruction*>::iterator j;
	if (i->kind() == "parallel")
	{
		if (((parallel*)i)->instrs.size() <= 1)
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
	else if (i->kind() == "block")
	{
		for (j = ((block*)i)->instrs.begin(); j != ((block*)i)->instrs.end(); j++)
		{
			(*j)->parent = this;
			push(*j);
		}
		((block*)i)->instrs.clear();
		delete (block*)i;
	}
	else
		instrs.push_back(i);
}
