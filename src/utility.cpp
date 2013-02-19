/*
 * utility.cpp
 *
 *  Created on: Feb 8, 2013
 *      Author: nbingham
 */

#include "utility.h"
#include "common.h"

instruction *expand_instantiation(string chp, map<string, keyword*> types, vspace *vars, list<string> *input, string tab, int verbosity, bool allow_process)
{
	map<string, keyword*>::iterator var_type;
	map<string, variable>::iterator mem_var;
	variable v = variable(chp, !allow_process, tab, verbosity);
	variable v2;

	if (input != NULL)
		input->push_back(v.name);

	if ((var_type = types.find(v.type)) != types.end())
	{
		vars->insert(v);

		if (var_type->second->kind() == "record" || var_type->second->kind() == "channel")
			vars->instantiate(v.name, &(((record*)var_type->second)->vars));
		else if (var_type->second->kind() == "process" || var_type->second->kind() == "operate")
		{
			if (allow_process)
			{
				map<string, string> rename;
				map<string, string>::iterator ri;
				list<string>::iterator i, j;

				rename = vars->instantiate(v.name, &(((process*)var_type->second)->vars));

				if (v.type.find_first_of("!?@") != v.type.npos && v.name.find_first_of(".") != v.name.npos)
				{
					string chname = v.name.substr(0, v.name.find_first_of("."));
					string chtype = v.type.substr(0, v.type.find_first_of("."));
					map<string, keyword*>::iterator ch = types.find(chtype);
					if (ch != types.end())
					{
						for (mem_var = ((channel*)ch->second)->vars.global.begin(); mem_var != ((channel*)ch->second)->vars.global.end(); mem_var++)
						{
							rename.insert(pair<string, string>(mem_var->second.name, chname + "." + mem_var->second.name));
							v2 = mem_var->second;
							v2.name = chname + "." + v2.name;
							v2.uid = vars->global.size();
							v2.io = false;
							vars->global.insert(pair<string, variable>(v2.name, v2));
						}
						for (mem_var = ((channel*)ch->second)->vars.label.begin(); mem_var != ((channel*)ch->second)->vars.label.end(); mem_var++)
						{
							rename.insert(pair<string, string>(mem_var->second.name, chname + "." + mem_var->second.name));
							v2 = mem_var->second;
							v2.name = chname + "." + v2.name;
							v2.uid = vars->label.size();
							v2.io = false;
							vars->label.insert(pair<string, variable>(v2.name, v2));
						}
					}
				}

				for (i = v.inputs.begin(), j = ((process*)var_type->second)->input.begin(); i != v.inputs.end() && j != ((process*)var_type->second)->input.end(); i++, j++)
					rename.insert(pair<string, string>(*j, *i));

				return ((process*)var_type->second)->def.duplicate(vars, rename, tab, verbosity);
			}
			else
				cout << "Error: Invalid use of type " << var_type->second->kind() << " in record definition." << endl;
		}
	}
	else
		cout << "Error: Invalid typename: " << v.type << endl;

	return NULL;
}

pair<string, instruction*> add_unique_variable(string prefix, string postfix, string type, map<string, keyword*> types, vspace *vars, string tab, int verbosity)
{
	string name = vars->unique_name(prefix);

	return pair<string, instruction*>(name, expand_instantiation(type + " " + name + postfix, types, vars, NULL, tab, verbosity, true));
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

string get_kind(string name, vspace *vars, map<string, keyword*> types)
{
	string type = vars->get_type(name);
	map<string, keyword*>::iterator i;
	i = types.find(type);
	if (i == types.end())
		return "";

	return i->second->kind();
}

// Only | & ~ operators allowed
string demorgan(string exp, bool invert)
{
	list<string> ops, ex;

	string left, right, op = "";
	size_t p;

	if (op == "")
	{
		p = find_first_of_l0(exp, "|");
		if (p != exp.npos)
		{
			left = exp.substr(0, p);
			right = exp.substr(p+1);
			if (invert)
				return demorgan(left, true) + "&" + demorgan(right, true);
			else
				return "(" + demorgan(left, false) + "|" + demorgan(right, false) + ")";
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
				return "(" + demorgan(left, true) + "|" + demorgan(right, true) + ")";
			else
				return demorgan(left, false) + "&" + demorgan(right, false);
		}
	}

	if (op == "")
	{
		p = find_first_of_l0(exp, "~");
		if (p != exp.npos)
		{
			if (invert)
				return demorgan(exp.substr(p+1), false);
			else
				return demorgan(exp.substr(p+1), true);
		}
	}

	if (exp[0] == '(' && exp[exp.length()-1] == ')' && op == "")
		return demorgan(exp.substr(1, exp.length()-2), invert);

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
