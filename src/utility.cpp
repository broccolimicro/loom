/*
 * utility.cpp
 *
 *  Created on: Feb 8, 2013
 *      Author: nbingham
 */

#include "utility.h"
#include "common.h"

instruction *expand_instantiation(instruction *parent, string chp, variable_space *vars, list<string> *input, flag_space *flags, bool allow_process)
{
	keyword* type;
	map<string, variable>::iterator mem_var;
	variable v = variable(chp, !allow_process, flags);
	variable v2;

	map<string, string> rename;
	map<string, string> rename2;
	map<string, string>::iterator ri;
	list<string>::iterator i, j;
	string name,val;
	int k;
	vector<int> reset;
	bool first;

	if (input != NULL)
		input->push_back(v.name);

	if ((type = vars->find_type(v.type)) != NULL)
	{
		if (allow_process)
		{
			reset = v.reset;
			v.reset = vector<int>();
		}
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

					return p->def.duplicate(parent, vars, rename);
				}
				else
				{
					rename = vars->call(v.name, !allow_process, &(p->vars));
					return new sequential(parent, v.name + ".call.r+;[" + v.name + ".call.a];" + v.name + ".call.r-;[~" + v.name + ".call.a];", vars, flags);
				}
			}
			else
				cout << "Error: Invalid use of type " << type->kind() << " in record definition." << endl;
		}
		else if (allow_process)
		{
			first = true;
			for (k = 0; k < (int)reset.size(); k++)
			{
				if (reset[k] == 0 || reset[k] == 1)
				{
					if (!first)
					{
						name += ",";
						val += ",";
					}

					name += v.name + "[" + to_string(k) + "]";
					val += to_string(reset[k]);

					first = false;
				}
			}

			if (name != "" && val != "")
				return new assignment(parent, name + ":=" + val, vars, flags);
		}
	}
	else
		cout << "Error: Invalid typename: " << v.type << endl;

	return NULL;
}

pair<string, instruction*> add_unique_variable(instruction *parent, string prefix, string postfix, string type, variable_space *vars, flag_space *flags)
{
	string name = vars->unique_name(prefix);

	string dec = type;
	if (dec[dec.length()-1] != '>')
		dec += " ";
	dec += name;

	return pair<string, instruction*>(name, expand_instantiation(parent, dec + postfix, vars, NULL, flags, true));
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

	string left, right;
	size_t p;

	if (depth != 0)
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

		p = find_first_of_l0(exp, "~");
		if (p != exp.npos)
		{
			if (invert)
				return demorgan(exp.substr(p+1), depth, false);
			else
				return demorgan(exp.substr(p+1), depth, true);
		}

		if (exp[0] == '(' && exp[exp.length()-1] == ')')
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

vector<string> distribute(string exp)
{
	vector<string> result;
	vector<string> terms;
	vector<string> temp0, temp1;
	string term, tempstr;
	size_t i, j, k, l, m, n;
	int count, count1;
	for (i = 0, j = 0, count = 0; i <= exp.length(); i++)
	{
		if (i < exp.length() && exp[i] == '(')
			count++;
		else if (i < exp.length() && exp[i] == ')')
			count--;

		if (i == exp.length() || (count == 0 && exp[i] == '|'))
		{
			terms.clear();
			term = exp.substr(j, i-j);
			for (k = 0, l = 0, count1 = 0; k <= term.size(); k++)
			{
				if (k < term.size() && term[k] == '(')
					count1++;
				else if (k < term.size() && term[k] == ')')
					count1--;

				if (k == term.size() || (count1 == 0 && term[k] == '&'))
				{
					temp0 = terms;
					temp1.clear();
					terms.clear();
					tempstr = term.substr(l, k-l);
					if (tempstr.find_first_of("|&()") == tempstr.npos)
						temp1.push_back(tempstr);
					else
						temp1 = distribute(strip(tempstr));

					if (temp0.size() == 0)
						terms = temp1;
					else if (temp1.size() == 0)
						terms = temp0;
					else
						for (m = 0; m < temp0.size(); m++)
							for (n = 0; n < temp1.size(); n++)
								terms.push_back(temp0[m] + "&" + temp1[n]);
					l = k+1;
				}
			}

			result.insert(result.end(), terms.begin(), terms.end());

			j = i+1;
		}
	}

	return result;
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

string restrict_exp(string exp, string var, int val)
{
	string left, right;
	string leftres, rightres;
	size_t p;

	p = find_first_of_l0(exp, "|");
	if (p != exp.npos)
	{
		left = exp.substr(0, p);
		right = exp.substr(p+1);
		leftres = restrict_exp(left, var, val);
		rightres = restrict_exp(right, var, val);

		if (leftres == "1" || rightres == "1")
			return "1";
		else if (leftres == "0")
			return rightres;
		else if (rightres == "0")
			return leftres;
		else
			return leftres + "|" + rightres;
	}

	p = find_first_of_l0(exp, "&");
	if (p != exp.npos)
	{
		left = exp.substr(0, p);
		right = exp.substr(p+1);
		leftres = restrict_exp(left, var, val);
		rightres = restrict_exp(right, var, val);

		if (leftres == "0" || rightres == "0")
			return "0";
		else if (leftres == "1")
			return rightres;
		else if (rightres == "1")
			return leftres;
		else
			return leftres + "&" + rightres;
	}

	p = find_first_of_l0(exp, "~");
	if (p != exp.npos)
	{
		leftres = restrict_exp(exp.substr(p+1), var, val);

		if (leftres == "1")
			return "0";
		else if (leftres == "0")
			return "1";
		else
			return "~" + leftres;
	}

	if (exp[0] == '(' && exp[exp.length()-1] == ')')
		return restrict_exp(exp.substr(1, exp.length()-2), var, val);

	if (exp == var)
		return to_string(val);
	else
		return exp;
}

void gen_variables(string exp, variable_space *vars, flag_space *flags)
{
	string name;
	int id;
	size_t i, j;
	for (i = exp.find_first_of("&|~()"), j = 0; i != exp.npos; j = i+1, i = exp.find_first_of("&|~()", j))
	{
		name = exp.substr(j, i-j);
		if (name.size() != 0)
		{
			id = vars->get_uid(name);
			if (id < 0 && name.substr(0, 2) != "0x" && name.substr(0, 2) != "0b" && name.find_first_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_") != name.npos)
				vars->insert(variable(name, "node", 1, false, flags));
		}
	}

	name = exp.substr(j);
	if (name.size() != 0)
	{
		id = vars->get_uid(name);
		if (id < 0 && name.substr(0, 2) != "0x" && name.substr(0, 2) != "0b" && name.find_first_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_") != name.npos)
			vars->insert(variable(name, "node", 1, false, flags));
	}
}

vector<string> extract_names(string exp)
{
	vector<string> ret;
	size_t i = string::npos, j = 0;
	while ((i = exp.find_first_of("~&|()", i+1)) != string::npos)
	{
		if (i-j > 1)
			ret.push_back(exp.substr(j, i-j));
		j = i+1;
	}

	if (exp.size()-j > 1)
		ret.push_back(exp.substr(j));

	return unique(&ret);
}

vector<int> extract_ids(string exp, variable_space *vars)
{
	vector<int> ret;
	size_t i = string::npos, j = 0;
	while ((i = exp.find_first_of("~&|()", i+1)) != string::npos)
	{
		if (i-j > 1)
			ret.push_back(vars->get_uid(exp.substr(j, i-j)));
		j = i+1;
	}

	if (exp.size()-j > 1)
		ret.push_back(vars->get_uid(exp.substr(j)));

	return unique(&ret);
}

string remove_var(string exp, string var)
{
	string ret = "", test;
	size_t i = string::npos, j = 0, k;
	while ((i = exp.find_first_of("|", i+1)) != string::npos)
	{
		test = exp.substr(j, i-j);
		k = test.find(var);
		if (k != string::npos)
		{
			test = test.substr(0, k) + test.substr(k + var.length());
			if (test[0] == '&')
				test = test.substr(1);
			else if (test[test.length() - 1] == '&')
				test = test.substr(0, test.length()-1);
			else if ((k = test.find("&&")) != string::npos)
				test = test.substr(0, k) + test.substr(k+2);

			ret += (ret == "" ? test : "|" + test);
		}
		j = i+1;
	}

	test = exp.substr(j);
	k = test.find(var);
	if (k != string::npos)
	{
		test = test.substr(0, k) + test.substr(k + var.length());
		if (test[0] == '&')
			test = test.substr(1);
		else if (test[test.length() - 1] == '&')
			test = test.substr(0, test.length()-1);
		else if ((k = test.find("&&")) != string::npos)
			test = test.substr(0, k) + test.substr(k+2);
		ret += (ret == "" ? test : "|" + test);
	}

	return ret;
}

string remove_comments(string str)
{
	// Remove line comments:
	size_t comment_begin = str.find("//");
	size_t comment_end = str.find("\n", comment_begin);
	while (comment_begin != str.npos && comment_end != str.npos){
		str = str.substr(0,comment_begin) + str.substr(comment_end);
		comment_begin = str.find("//");
		comment_end = str.find("\n", comment_begin);
	}

	// Remove block comments:
	comment_begin = str.find("/*");
	comment_end = str.find("*/");
	while (comment_begin != str.npos && comment_end != str.npos){
		str = str.substr(0,comment_begin) + str.substr(comment_end+2);
		comment_begin = str.find("/*");
		comment_end = str.find("*/");
	}
	return str;
}

string remove_whitespace(string str)
{
	string cleaned_str;
	// Remove extraneous whitespace
	for (string::iterator i = str.begin(); i != str.end(); i++)
	{
		if (!sc(*i))
			cleaned_str += *i;
		else if (nc(*(i-1)) && (i == str.end()-1 || nc(*(i+1))))
			cleaned_str += ' ';
	}

	return cleaned_str;
}
