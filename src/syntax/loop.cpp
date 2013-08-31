/*
 * loop.cpp
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

#include "loop.h"

loop::loop()
{
	_kind = "loop";
}

loop::loop(instruction *parent, string chp, variable_space *vars, flag_space *flags)
{
	clear();

	_kind = "loop";
	this->chp = chp.substr(2, chp.length()-3);
	this->flags = flags;
	this->type = unknown;
	this->vars = vars;
	this->parent = parent;

	expand_shortcuts();
	parse();
}

loop::~loop()
{
	_kind = "loop";
}

/* This copies a guard to another process and replaces
 * all of the specified variables.
 */
instruction *loop::duplicate(instruction *parent, variable_space *vars, map<string, string> convert)
{
	loop *instr;

	instr 				= new loop();
	instr->chp			= this->chp;
	instr->vars			= vars;
	instr->flags		= flags;
	instr->type			= this->type;
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

	list<pair<sequential*, guard*> >::iterator l;
	for (l = instrs.begin(); l != instrs.end(); l++)
		instr->instrs.push_back(pair<sequential*, guard*>((sequential*)l->first->duplicate(instr, vars, convert), (guard*)l->second->duplicate(instr, vars, convert)));

	return instr;
}

void loop::expand_shortcuts()
{
	//Check for the shorthand *[S] and replace it with *[1 -> S]
	string::iterator i, j;
	int depth[3] = {0};
	for (i = chp.begin(), j = chp.begin(); i != chp.end()-1; i++)
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

		if (depth[0] == 0 && depth[1] == 0 && depth[2] == 0 && (*i == '-' && *(i+1) == '>'))
			return;
	}

	chp = "1->" + chp;
}

void loop::parse()
{
	string guardstr, sequentialstr;

	string::iterator i, j, k;

	flags->inc();

	//Parse instructions!
	bool guarded = true;
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

		if (!guarded && depth[0] == 0 && depth[1] == 0 && depth[2] == 0 && ((*i == '|' && *(i+1) != '|' && *(i-1) != '|') || (i == chp.end() && type == choice)))
		{
			if (flags->log_base_hse())
				(*flags->log_file) << flags->tab << "Choice\n";
			if (type == unknown)
				type = choice;
			else if (type == mutex)
				cout << "Error: A loop can either be mutually exclusive or choice, but not both." << endl;

			sequentialstr = chp.substr(j-chp.begin(), i-j);
			k = sequentialstr.find("->") + sequentialstr.begin();
			guardstr = sequentialstr.substr(0, k-sequentialstr.begin());
			sequentialstr = sequentialstr.substr(k-sequentialstr.begin()+2);

			instrs.push_back(pair<sequential*, guard*>(new sequential(this, sequentialstr, vars, flags), new guard(this, guardstr, vars, flags)));
			j = i+1;
			guarded = true;
		}
		else if (!guarded && depth[0] == 0 && depth[1] <= 1 && depth[2] == 0 && ((*i == '[' && *(i+1) == ']') || i == chp.end()))
		{
			if (flags->log_base_hse())
				(*flags->log_file) << flags->tab << "Mutex\n";
			if (type == unknown)
				type = mutex;
			else if (type == choice)
				cout << "Error: A loop can either be mutually exclusive or choice, but not both." << endl;

			sequentialstr = chp.substr(j-chp.begin(), i-j);
			k = sequentialstr.find("->") + sequentialstr.begin();
			guardstr = sequentialstr.substr(0, k-sequentialstr.begin());
			sequentialstr = sequentialstr.substr(k-sequentialstr.begin()+2);

			instrs.push_back(pair<sequential*, guard*>(new sequential(this, sequentialstr, vars, flags), new guard(this, guardstr, vars, flags)));
			j = i+2;
			guarded = true;
		}
		else if (depth[0] == 0 && depth[1] == 0 && depth[2] == 0 && ((*i == '-' && *(i+1) == '>') || i == chp.end()))
			guarded = false;
	}

	flags->dec();
}

void loop::simulate()
{

}

void loop::rewrite()
{
	list<pair<sequential*, guard*> >::iterator i;
	for (i = instrs.begin(); i != instrs.end(); i++)
	{
		i->second->rewrite();
		i->first->rewrite();
	}
}

void loop::reorder()
{

}

vector<int> loop::generate_states(petri *n, rule_space *p, vector<int> f, map<int, int> pbranch, map<int, int> cbranch)
{
	list<pair<sequential*, guard*> >::iterator instr_iter;
	vector<int> start, end;
	string antiguard = "";
	string bvname;
	vector<string> bvnames;
	vector<int> bvuids;
	int ncbranch_count;
	int i, k;
	bool first;

	flags->inc();
	if (flags->log_base_state_space())
		(*flags->log_file) << flags->tab << "Loop " << chp << endl;

	net = n;
	prs = p;
	from = f;

	ncbranch_count = net->cbranch_count;
	net->cbranch_count++;
	for (instr_iter = instrs.begin(), k = instrs.size()-1; instr_iter != instrs.end(); instr_iter++, k--)
	{
		/*if (type == choice)
		{
			bvname = "_bv" + to_string(ncbranch_count) + "_" + to_string(k);
			bvnames.push_back(bvname);
			bvuids.push_back(vars->insert(variable(bvname, "node", 1, false, flags)));
			prs->rules.push_back(rule(instr_iter->second->chp, "~(" + instr_iter->second->chp + ")", bvname, vars, net, flags));
			instr_iter->second->chp = "(" + instr_iter->second->chp + ")&" + bvname;
		}*/

		start.clear();
		end.clear();
		start = instr_iter->second->generate_states(net, prs, from, pbranch, cbranch);
		end.push_back(net->insert_place(start, pbranch, cbranch, this));
		end = instr_iter->first->generate_states(net, prs, end, pbranch, cbranch);
		net->connect(end, from);
		antiguard += string(antiguard != "" ? "&" : "") + "~(" + instr_iter->second->chp + ")";
	}

	/*if (type == choice)
	{
		bvname = "";
		for (i = 0; i < (int)bvnames.size(); i++)
		{
			if (i != 0)
				bvname += "|";

			first = true;
			for (k = 0; k < (int)bvnames.size(); k++)
			{
				if (!first && i != k)
					bvname += "&";

				if (i != k)
				{
					bvname += "~" + bvnames[k];
					first = false;
				}
			}
		}

		vars->enforcements = vars->enforcements >> logic(bvname, vars);
		prs->excl.push_back(pair<vector<int>, int>(bvuids, 1));
		for (k = 0; k < (int)bvuids.size(); k++)
			prs->excl.push_back(pair<vector<int>, int>(vector<int>(1, bvuids[k]), 0));

		antiguard = "(" + antiguard + ")";
		for (i = 0; i < (int)bvnames.size(); i++)
			antiguard += "&~" + bvnames[i];
	}*/

	uid.push_back(net->insert_transition(f, logic(antiguard, vars), pbranch, cbranch, this));

	flags->dec();

	return uid;
}

void loop::print_hse(string t, ostream *fout)
{
	(*fout) << "\n" << t << "*[";
	list<pair<sequential*, guard*> >::iterator i;
	for (i = instrs.begin(); i != instrs.end(); i++)
	{
		if (i != instrs.begin() && type == mutex)
			(*fout) << "\n" << t << "[]";
		else if (i != instrs.begin() && type == choice)
			(*fout) << "\n" << t << "|";
		if (instrs.size() > 1 || i->second->chp != "1")
		{
			i->second->print_hse(t + "\t", fout);
			(*fout) << " ->\t";
		}
		else
			(*fout) << "\t";
		i->first->print_hse(t + "\t", fout);
	}
	(*fout) << endl << t << "]";
}
