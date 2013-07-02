/*
 * utility.cpp
 *
 *  Created on: Feb 8, 2013
 *      Author: nbingham
 */

#include "utility.h"
#include "common.h"

instruction *expand_instantiation(instruction *parent, string chp, vspace *vars, list<string> *input, string tab, int verbosity, bool allow_process)
{
	keyword* type;
	map<string, variable>::iterator mem_var;
	variable v = variable(chp, !allow_process, tab, verbosity);
	variable v2;

	map<string, string> rename;
	map<string, string> rename2;
	map<string, string>::iterator ri;
	list<string>::iterator i, j;

	if (input != NULL)
		input->push_back(v.name);

	if ((type = vars->find_type(v.type)) != NULL)
	{
		vars->insert(v);

		if (type->kind() == "record" || type->kind() == "channel")
			vars->instantiate(v.name, !allow_process, &(((record*)type)->vars), true);
		else if (type->kind() == "process" || type->kind() == "operate")
		{
			if (allow_process)
			{
				process *p = (process*)type;

				if (p->is_inline)
				{
					rename = vars->instantiate(v.name, !allow_process, &(p->vars), false);

					if (v.type.find_first_of("!?#") != v.type.npos && v.name.find_first_of(".") != v.name.npos)
					{
						string chname = v.name.substr(0, v.name.find_first_of("."));
						string chtype = v.type.substr(0, v.type.find_first_of("."));
						keyword* ch = vars->find_type(chtype);
						if (ch != NULL)
						{
							rename2 = vars->instantiate(chname, !allow_process, &(((channel*)ch)->vars), true);
							rename.insert(rename2.begin(), rename2.end());
						}
					}

					for (i = v.inputs.begin(), j = p->args.begin(); i != v.inputs.end() && j != p->args.end(); i++, j++)
						rename.insert(pair<string, string>(*j, *i));

					return p->def.duplicate(parent, vars, rename, tab, verbosity);
				}
				else
				{
					rename = vars->call(v.name, !allow_process, &(p->vars));
					return new sequential(parent, v.name + ".call.r+;[" + v.name + ".call.a];" + v.name + ".call.r-;[~" + v.name + ".call.a];", vars, tab, verbosity);
				}
			}
			else
				cout << "Error: Invalid use of type " << type->kind() << " in record definition." << endl;
		}
	}
	else
		cout << "Error: Invalid typename: " << v.type << endl;

	return NULL;
}

pair<string, instruction*> add_unique_variable(instruction *parent, string prefix, string postfix, string type, vspace *vars, string tab, int verbosity)
{
	string name = vars->unique_name(prefix);

	string dec = type;
	if (dec[dec.length()-1] != '>')
		dec += " ";
	dec += name;

	return pair<string, instruction*>(name, expand_instantiation(parent, dec + postfix, vars, NULL, tab, verbosity, true));
}

size_t find_name(string subject, string search, size_t pos)
{
	size_t ret = -1 + pos;
	bool alpha0, alpha1;

	do
	{
		ret = subject.find(search, ret + 1);
		alpha0 = ret > 0 && (nc(subject[ret-1]) || subject[ret-1] == '.');
		alpha1 = ret + search.length() < subject.length() && nc(subject[ret + search.length()]);
	} while (ret != subject.npos && (alpha0 || alpha1));

	return ret;
}

// Only | & ~ operators allowed
string demorgan(string exp, int depth, bool invert)
{
	list<string> ops, ex;

	string left, right, op = "";
	size_t p;

	if (depth != 0)
	{
		if (op == "")
		{
			p = find_first_of_l0(exp, "|");
			if (p != exp.npos)
			{
				left = exp.substr(0, p);
				right = exp.substr(p+1);
				if (invert)
					return demorgan(left, depth-1, true) + "&" + demorgan(right, depth-1, true);
				else
					return "(" + demorgan(left, depth-1, false) + "|" + demorgan(right, depth-1, false) + ")";
			}
		}

		if (op == "")
		{
			p = find_first_of_l0(exp, "&");
			if (p != exp.npos)
			{
				left = exp.substr(0, p);
				right = exp.substr(p+1);
				if (invert)
					return "(" + demorgan(left, depth-1, true) + "|" + demorgan(right, depth-1, true) + ")";
				else
					return demorgan(left, depth-1, false) + "&" + demorgan(right, depth-1, false);
			}
		}

		if (op == "")
		{
			p = find_first_of_l0(exp, "~");
			if (p != exp.npos)
			{
				if (invert)
					return demorgan(exp.substr(p+1), depth, false);
				else
					return demorgan(exp.substr(p+1), depth, true);
			}
		}

		if (exp[0] == '(' && exp[exp.length()-1] == ')' && op == "")
			return demorgan(exp.substr(1, exp.length()-2), depth, invert);
	}

	if (invert)
		return "~" + exp;
	else
		return exp;
}

string strip(string e)
{
	while (e.length() >= 2 && find_first_of_l0(e, "|&~") == e.npos && e.find_first_of("()") != e.npos)
		e = e.substr(1, e.length() - 2);
	return e;
}

string distribute(string exp, string sib)
{
	list<string> ops, ex;

	string left, right, op = "";
	size_t p;

	if (op == "")
	{
		p = find_first_of_l0(exp, "|");
		if (p != exp.npos)
		{
			op = "|";
			left = exp.substr(0, p);
			right = exp.substr(p+1);
			cout << left << " " << right << endl;

			if (sib == "")
				return distribute(left, "") + "|" + distribute(right, "");
			else
				return distribute(left + "&" + sib, "") + "|" + distribute(right + "&" + sib, "");
		}
	}

	if (op == "")
	{
		p = find_first_of_l0(exp, "&");
		if (p != exp.npos)
		{
			op = "&";
			left = exp.substr(0, p);
			right = exp.substr(p+1);
			cout << left << " " << right << endl;

			return distribute(left, right);
		}
	}

	if (op == "")
	{
		p = find_first_of_l0(exp, "~");
		if (p != exp.npos)
		{
			op = "~";
			left = "";
			right = exp.substr(p+1);
			cout << left << " " << right << endl;
		}
	}

	if (exp[0] == '(' && exp[exp.length()-1] == ')' && op == "")
		return distribute(exp.substr(1, exp.length()-2), sib);

	if (sib == "")
		return exp;
	else
		return exp + "&" + sib;
}

string flatten_slice(string slices)
{
	size_t a = slices.find_first_of("["),
		   b = slices.find_first_of("]"),
		   c = slices.find_last_of("["),
		   d = slices.find_last_of("]");

	string left = slices.substr(a+1, b - (a+1));
	string right = slices.substr(c+1, d - (c+1));

	size_t x = left.find(".."),
		   y = right.find("..");

	int ll, rl, rh;

	if (x != left.npos)
		ll = atoi(left.substr(0, x).c_str());
	else
		ll = atoi(left.c_str());

	if (y != right.npos)
	{
		rl = atoi(right.substr(0, x).c_str());
		rh = atoi(right.substr(x+2).c_str());
	}
	else
	{
		rl = atoi(right.c_str());
		rh = rl;
	}

	string ret = slices.substr(0, a) + "[" + to_string(ll + rl);

	if (rh > rl)
		ret += ".." + to_string(ll+rh);

	ret += "]";
	return ret;
}
