/*
 * condition.cpp
 *
 * DND
 */

#include "../common.h"
#include "../utility.h"
#include "../type.h"
#include "../data.h"

#include "condition.h"
#include "parallel.h"

condition::condition()
{
	_kind = "condition";
	type = unknown;
}

condition::condition(instruction *parent, string chp, variable_space *vars, flag_space *flags)
{
	clear();

	_kind = "condition";
	type = unknown;
	this->chp = chp.substr(1, chp.length()-2);
	this->flags = flags;
	this->vars = vars;
	this->parent = parent;

	expand_shortcuts();
	parse();
}

condition::~condition()
{
	_kind = "condition";
	type = unknown;
}

/* This copies a guard to another process and replaces
 * all of the specified variables.
 */
instruction *condition::duplicate(instruction *parent, variable_space *vars, map<string, string> convert)
{
	condition *instr;

	instr 				= new condition();
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

void condition::expand_shortcuts()
{
	//Check for the shorthand [var] and replace it with [var -> skip]
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

	chp += "->skip";
}
// [G -> S]
void condition::parse()
{
	string::iterator i, j, k;
	string guardstr, sequentialstr;
	bool guarded = true;

	flags->inc();
	if (flags->log_base_hse())
		(*flags->log_file) << flags->tab << "condition:\t" << chp << endl;

	//Parse instructions!
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
				cout << "Error: A condition can either be mutually exclusive or choice, but not both." << endl;

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
				cout << "Error: A condition can either be mutually exclusive or choice, but not both." << endl;

			sequentialstr = chp.substr(j-chp.begin(), i-j);
			k = sequentialstr.find("->") + sequentialstr.begin();
			guardstr = sequentialstr.substr(0, k-sequentialstr.begin());
			sequentialstr = sequentialstr.substr(k-sequentialstr.begin()+2);

			instrs.push_back(pair<sequential*, guard*>(new sequential(this, sequentialstr, vars, flags), new guard(this, guardstr, vars, flags)));
			j = i+2;
			guarded = true;
		}
		else if (depth[0] == 0 && depth[1] <= 1 && depth[2] == 0 && ((*i == '-' && *(i+1) == '>') || i == chp.end()))
			guarded = false;
	}
	flags->dec();
}

void condition::simulate()
{
	list<pair<sequential*, guard*> >::iterator i;
	for (i = instrs.begin(); i != instrs.end(); i++)
	{

	}
}

// TODO Bug in merge code somewhere
void condition::rewrite()
{
	list<pair<sequential*, guard*> >::iterator i, j;
	list<instruction*>::iterator ii;
	sequential *copy;
	sequential *nsequential;
	guard *nguard;
	condition *front;
	list<pair<sequential*, guard*> > add;

	for (i = instrs.begin(); i != instrs.end(); i++)
	{
		if (i->first->instrs.size() > 0 && i->first->instrs.front()->kind() == "condition" && (((condition*)i->first->instrs.front())->type == type || instrs.size() == 1 || ((condition*)i->first->instrs.front())->instrs.size() == 1))
		{
			front = (condition*)i->first->instrs.front();
			i->first->instrs.pop_front();

			j = front->instrs.begin();
			for (j++; j != front->instrs.end(); j++)
			{
				nsequential = j->first;
				copy = (sequential*)i->first->duplicate(this, vars, map<string, string>());
				for (ii = copy->instrs.begin(); ii != copy->instrs.end(); ii++)
					nsequential->instrs.push_back(*ii);
				copy->instrs.clear();
				delete copy;
				nguard = j->second;

				nguard->chp = canonical("(" + i->second->chp + ")&(" + nguard->chp + ")", vars).print(vars);
				add.push_back(pair<sequential*, guard*>(nsequential, nguard));
			}
			j = front->instrs.begin();

			i->second->chp = canonical("(" + i->second->chp + ")&(" + j->second->chp + ")", vars).print(vars);
			for (ii = j->first->instrs.begin(); ii != j->first->instrs.end(); ii++)
				i->first->instrs.push_front(*ii);
			j->first->instrs.clear();
			delete j->first;
			delete j->second;
			front->instrs.clear();
			delete front;
		}
	}

	for (i = add.begin(); i != add.end(); i++)
		instrs.push_back(*i);
	add.clear();

	vector<string> vl;
	vector<int> stable;
	vector<int> monotonic;
	vector<int> unstable;
	variable *v;
	int k;
	canonical ugexp, dgexp, gexp;
	string g, ng;

	if (type == choice)
	{
		for (i = instrs.begin(); i != instrs.end(); i++)
		{
			stable.clear();
			monotonic.clear();
			unstable.clear();

			vl = extract_names(i->second->chp);
			for (k = 0; k < (int)vl.size(); k++)
			{
				v = vars->find(vl[k]);
				if (v != NULL && v->driven)
					stable.push_back(v->uid);
				else if (v != NULL && !v->driven && vars->part_of_channel(v->uid))
					monotonic.push_back(v->uid);
				else if (v != NULL && !v->driven)
					unstable.push_back(v->uid);
				else
					cout << "Error: Internal failure at line " << __LINE__ << " in file " << __FILE__ << ". Cannot find variable " << vl[k] << "." << endl;
			}

			if (stable.size() > 0)
			{
				gexp = canonical(i->second->chp, vars).smooth(monotonic).smooth(unstable);
				ugexp = canonical(i->second->chp, vars).restrict(gexp);
				dgexp = canonical(i->second->chp, vars).restrict(~gexp);
				cout << "Stable " << gexp.print(vars) << "\t" << ugexp.print(vars) << "\t" << dgexp.print(vars) << endl;
			}
			else if (monotonic.size() > 0)
			{
				gexp = canonical(i->second->chp, vars).smooth(unstable);
				ugexp = canonical(i->second->chp, vars).restrict(gexp);
				dgexp = canonical(i->second->chp, vars).restrict(~gexp);
				cout << "Monotonic " << gexp.print(vars) << "\t" << ugexp.print(vars) << "\t" << dgexp.print(vars) << endl;
			}
			else if (unstable.size() > 0)
			{
				gexp = canonical(i->second->chp, vars);
				cout << "Unstable " << gexp.print(vars) << endl;
			}
			else
				cout << "Error: Internal failure at line " << __LINE__ << " in file " << __FILE__ << "." << endl;
		}

		for (i = instrs.begin(); i != instrs.end(); i++)
		{
			j = i;
			for (j++; j != instrs.end(); j++)
				if ((logic(i->second->chp, vars) & logic(j->second->chp, vars)) != 0)
					cout << "Error: Conflicting guards in a thin bar conditional {" << i->second->chp << ", " << j->second->chp << "}." << endl;
		}
	}
	else if (type == mutex)
		for (i = instrs.begin(); i != instrs.end(); i++)
		{
			j = i;
			for (j++; j != instrs.end(); j++)
				if ((logic(i->second->chp, vars) & logic(j->second->chp, vars)) != 0)
					cout << "Error: Conflicting guards in a thick bar conditional {" << i->second->chp << ", " << j->second->chp << "}." << endl;
		}

	for (i = instrs.begin(); i != instrs.end(); i++)
	{
		i->second->rewrite();
		i->first->rewrite();
	}
}

void condition::reorder()
{

}

vector<int> condition::generate_states(petri *n, rule_space *p, vector<int> f, map<int, int> pbranch, map<int, int> cbranch)
{
	list<pair<sequential*, guard*> >::iterator instr_iter;
	vector<int> start, end;
	map<int, int> ncbranch;
	int ncbranch_count;
	string bvname;
	vector<string> bvnames;
	vector<int> bvuids;
	int i, k;
	bool first;

	if (flags->log_base_state_space())
		(*flags->log_file) << flags->tab << "Conditional " << chp << endl;

	net = n;
	prs = p;
	from = f;

	ncbranch_count = net->cbranch_count;
	net->cbranch_count++;
	for (instr_iter = instrs.begin(), k = instrs.size()-1; instr_iter != instrs.end(); instr_iter++, k--)
	{
		ncbranch = cbranch;
		if (instrs.size() > 1)
			ncbranch.insert(pair<int, int>(ncbranch_count, (int)k));

		/*if (type == choice)
		{
			bvname = "_bv" + to_string(ncbranch_count) + "_" + to_string(k);
			bvnames.push_back(bvname);
			bvuids.push_back(vars->insert(variable(bvname, "node", 1, false, flags)));
			prs->insert(rule(instr_iter->second->chp, "~(" + instr_iter->second->chp + ")", bvname, vars, net, flags));
			instr_iter->second->chp = "(" + instr_iter->second->chp + ")&" + bvname;
		}*/

		end.clear();
		start.clear();
		start = instr_iter->second->generate_states(net, prs, from, pbranch, ncbranch);
		end.push_back(net->insert_place(start, pbranch, ncbranch, this));
		start = instr_iter->first->generate_states(net, prs, end, pbranch, ncbranch);
		uid.insert(uid.end(), start.begin(), start.end());
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
	}*/

	return uid;
}

void condition::print_hse(string t, ostream *fout)
{
	if (instrs.size() > 1)
		(*fout) << "\n" << t;
	(*fout) << "[";
	if (instrs.size() > 1)
		(*fout) << "\t";

	list<pair<sequential*, guard*> >::iterator i;
	for (i = instrs.begin(); i != instrs.end(); i++)
	{
		if (i != instrs.begin() && type == mutex)
		{
			if (instrs.size() > 1)
				(*fout) << "\n" << t;
			(*fout) << "[]";
			if (instrs.size() > 1)
				(*fout) << "\t";
		}
		else if (i != instrs.begin() && type == choice)
		{
			if (instrs.size() > 1)
				(*fout) << "\n" << t;
			(*fout) << "|";
			if (instrs.size() > 1)
				(*fout) << "\t";
		}
		i->second->print_hse(t+"\t", fout);
		if (i->first->instrs.size() > 0)
		{
			(*fout) << " -> ";
			i->first->print_hse(t+"\t", fout);
		}
	}
	if (instrs.size() > 1)
		(*fout) << "\n" << t;
	(*fout) << "]";
}
