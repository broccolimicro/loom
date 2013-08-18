/*
 * sequential.cpp
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

#include "instruction.h"
#include "sequential.h"
#include "parallel.h"
#include "condition.h"
#include "loop.h"
#include "debug.h"

sequential::sequential()
{
	_kind = "sequential";
}

sequential::sequential(instruction *parent, string chp, variable_space *vars, flag_space *flags)
{
	clear();

	this->_kind = "sequential";
	this->chp = chp;
	this->flags = flags;
	this->vars = vars;
	this->parent = parent;

	expand_shortcuts();
	parse();
}

sequential::~sequential()
{
	_kind = "sequential";
}

/* This copies a guard to another process and replaces
 * all of the specified variables.
 */
instruction *sequential::duplicate(instruction *parent, variable_space *vars, map<string, string> convert)
{
	sequential *instr;

	instr 				= new sequential();
	instr->chp			= this->chp;
	instr->vars			= vars;
	instr->flags		= flags;
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
		instr->instrs.push_back((*l)->duplicate(instr, vars, convert));

	return instr;
}

void sequential::expand_shortcuts()
{
}

void sequential::parse()
{
	string				raw_instr;	// chp of a sub sequential
	string::iterator	i, j;
	bool				para = false;
	int					depth[3] = {0};
	size_t				k;

	flags->inc();
	if (flags->log_base_hse())
		(*flags->log_file) << flags->tab << "Sequential: " << chp << endl;

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

			// This sub sequential is a set of parallel sub sub sequentials. s0 || s1 || ... || sn
			if (para && raw_instr.length() > 0)
				push(new parallel(this, raw_instr, vars, flags));
			// This sub sequential has a specific order of operations. (s)
			else if (raw_instr[0] == '(' && raw_instr[raw_instr.length()-1] == ')' && raw_instr.length() > 0)
				push(new sequential(this, raw_instr.substr(1, raw_instr.length()-2), vars, flags));
			// This sub sequential is a loop. *[g0->s0[]g1->s1[]...[]gn->sn] or *[g0->s0|g1->s1|...|gn->sn]
			else if (raw_instr[0] == '*' && raw_instr[1] == '[' && raw_instr[raw_instr.length()-1] == ']' && raw_instr.length() > 0)
				push(expand_loop(raw_instr));
			// This sub sequential is a condition. [g0->s0[]g1->s1[]...[]gn->sn] or [g0->s0|g1->s1|...|gn->sn]
			else if (raw_instr[0] == '[' && raw_instr[raw_instr.length()-1] == ']' && raw_instr.length() > 0)
				push(expand_condition(raw_instr));
			else if (raw_instr.find_first_of("{}") != raw_instr.npos)
				push(new debug(this, raw_instr, vars, flags));
			// This sub sequential is a variable instantiation.
			else if (vars->vdef(raw_instr) && raw_instr.length() > 0)
				push(expand_instantiation(this, raw_instr, vars, NULL, flags, true));
			// This sub sequential is a communication instantiation.
			else if ((k = raw_instr.find_first_of("?!#")) != raw_instr.npos && raw_instr.find(":=") == raw_instr.npos && raw_instr.length() > 0)
				push(add_unique_variable(this, raw_instr.substr(0, k) + "._fn", "(" + (k+1 < raw_instr.length() ? raw_instr.substr(k+1) : "") + ")", vars->get_type(raw_instr.substr(0, k)) + ".operator" + raw_instr[k] + "()", vars, flags).second);
			// This sub sequential is an assignment instruction.
			else if ((raw_instr.find(":=") != raw_instr.npos || raw_instr[raw_instr.length()-1] == '+' || raw_instr[raw_instr.length()-1] == '-') && raw_instr.length() > 0)
				push(expand_assignment(raw_instr));
			else if (raw_instr.find("skip") == raw_instr.npos && raw_instr.length() > 0)
				push(new guard(this, raw_instr, vars, flags));

			j = i+1;
			para = false;
		}
		// We are in the current scope, and the current character
		// is a parallel bar or the end of the chp string. This is
		// the middle of a parallel sub sequential.
		else if (depth[0] == 0 && depth[1] == 0 && depth[2] == 0 && ((*i == '|' && *(i+1) == '|') || i == chp.end()))
			para = true;
	}

	flags->dec();
}

void sequential::merge()
{
	list<instruction*>::iterator i, j;
	list<pair<sequential*, guard*> >::iterator k;
	list<pair<string, string> >::iterator m, n;
	i = instrs.begin();
	j = instrs.begin();
	condition *ic, *jc;
	assignment *ia, *ja;
	int conflict_count;

	for (j++; j != instrs.end(); j++)
	{
		if ((*i)->kind() == "condition" && (*j)->kind() == "condition")
		{
			ic = (condition*)*i;
			jc = (condition*)*j;

			if (ic->instrs.size() == 1 && ic->instrs.front().first->instrs.size() == 0)
			{
				for (k = jc->instrs.begin(); k != jc->instrs.end(); k++)
					k->second->chp = canonical("(" + ic->instrs.front().second->chp + ")&(" + k->second->chp + ")", vars).print(vars);
				instrs.remove(*i);
				//delete (condition*)(*i);
			}
		}
		else if ((*i)->kind() == "assignment" && (*j)->kind() == "assignment")
		{
			// TODO We need to check to see if one assignment reads a value of a channel that the other is modifying
			/*ia = (assignment*)*i;
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
			}*/
		}
		i = j;
	}

	for (j = instrs.begin(); j != instrs.end(); j++)
		(*j)->merge();
}

vector<int> sequential::generate_states(petri *n, vector<int> f, map<int, int> pbranch, map<int, int> cbranch)
{
	list<instruction*>::iterator instr_iter;
	instruction *instr;
	vector<int> next;
	map<map<int, int>, vector<int> > groups;
	map<map<int, int>, vector<int> >::iterator gi;
	map<int, int>::iterator mi, mj;
	bool trans = true;

	flags->inc();
	if (flags->log_base_state_space())
		(*flags->log_file) << flags->tab << "Sequential " << chp << endl;

	net  = n;
	from = f;
	if (instrs.size() == 0)
		uid.push_back(net->insert_dummy(from, pbranch, cbranch, this));
	else
	{
		uid  = f;
		for (instr_iter = instrs.begin(); instr_iter != instrs.end(); instr_iter++)
		{
			next.clear();
			instr = *instr_iter;

			if (instr_iter != instrs.begin() && trans)
				next.push_back(net->insert_place(uid, pbranch, cbranch, this));
			else
				next = uid;

			uid	= instr->generate_states(net, next, pbranch, cbranch);
			trans = (next != uid);
		}
	}
	flags->dec();
	return uid;
}

void sequential::insert_instr(int uid, int nid, instruction *instr)
{
	/*instr->uid = nid;
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
		else if (j->kind() == "condition")
			((condition*)j)->insert_instr(uid, nid, instr);
		else if (j->kind() == "guard")
			((guard*)j)->insert_instr(uid, nid, instr);
		else if (j->kind() == "sequential")
			((sequential*)j)->insert_instr(uid, nid, instr);
		else if (j->kind() == "assignment")
			((assignment*)j)->insert_instr(uid, nid, instr);
	}*/
}

void sequential::print_hse(string t, ostream *fout)
{
	list<instruction*>::iterator i;
	for (i = instrs.begin(); i != instrs.end(); i++)
	{
		if (i != instrs.begin())
			(*fout) << ";";
		(*i)->print_hse(t, fout);
	}
}

void sequential::push(instruction *i)
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
		{
			i->parent = this;
			instrs.push_back(i);
		}
	}
	else if (i->kind() == "sequential")
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
