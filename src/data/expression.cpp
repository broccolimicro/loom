/*
 * expression.cpp
 */

#include "expression.h"
#include "../common.h"
#include "../utility.h"
#include "vspace.h"

expression::expression()
{
	simple = "";
	vars = NULL;
	internal_vars = false;
	expr.clear();
}

expression::expression(string e)
{
	simple = "";
	expr.clear();

	vars = new vspace();
	internal_vars = true;
	gen_variables(e);
	gen_minterms(e);
	expr.mccluskey();
	gen_output();
}

expression::expression(string e, vspace *v)
{
	simple = "";
	expr.clear();

	vars = v;
	internal_vars = false;
	gen_minterms(e);
	expr.mccluskey();
	gen_output();
}

expression::expression(vector<minterm> t, vspace *v)
{
	simple = "";
	expr.clear();

	expr.terms = t;
	vars = v;
	internal_vars = false;
	expr.mccluskey();
	gen_output();
}

expression::~expression()
{
	simple = "";
	if (vars != NULL && internal_vars)
		delete vars;
	vars = NULL;
	internal_vars = false;
	expr.clear();
}

void expression::gen_variables(string e)
{
	int id;
	size_t p;

	p = find_first_of_l0(e, "|");
	if (p != e.npos)
	{
		gen_variables(e.substr(0, p));
		gen_variables(e.substr(p+1));
		return;
	}

	p = find_first_of_l0(e, "&");
	if (p != e.npos)
	{
		gen_variables(e.substr(0, p));
		gen_variables(e.substr(p+1));
		return;
	}

	p = find_first_of_l0(e, "~");
	if (p != e.npos)
	{
		gen_variables(e.substr(p+1));
		return;
	}

	if (e[0] == '(' && e[e.length()-1] == ')')
	{
		gen_variables(e.substr(1, e.length()-2));
		return;
	}

	id = vars->get_uid(e);
	if (id < 0 && e.substr(0, 2) != "0x" && e.substr(0, 2) != "0b" && e.find_first_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_") != e.npos)
	{
		vars->insert(variable(e, "node", 1, false));
		//cout << e << endl;
	}
}

//Used by rule
void expression::gen_minterms(string e)
{
	int i, j, k, m, n;
	uint32_t v;
	vector<minterm> values(vars->global.size());
	vector<int> exists;

	for (i = 0; i < vars->global.size(); i++)
		if (find_name(e, vars->get_name(i)) != -1)
			exists.push_back(i);

	m = pow(2, exists.size())/2;
	n = 2;

	for (i = 0; i < (int)vars->global.size(); i++)
		if (find(exists.begin(), exists.end(), i) == exists.end())
			values[i] = minterm(pow(2, exists.size()), vX);

	for (i = 0; i < (int)exists.size(); i++)
	{
		v = v0;
		for (j = 0; j < n; j++)
		{
			for (k = 0; k < m; k++)
				values[exists[i]].push_back(v);
			if (v == v0)
				v = v1;
			else
				v = v0;
		}

		m /= 2;
		n *= 2;
	}

	minterm result = evaluate(e, vars, values);

	for (i = 0; i < result.size; i++)
		if (result[i] == v1)
		{
			minterm x;
			for (j = 0; j < (int)values.size(); j++)
				x.push_back(values[j][i]);
			expr.push_back(x);
		}

	expr.print();
}

void expression::mccluskey()
{
	expr.mccluskey();
	simple = expr.print(vars->get_names());
}

//Used by rule
void expression::gen_output()
{
	if (vars == NULL)
		simple = expr.print();
	else
		simple = expr.print(vars->get_names());
}

expression &expression::operator()(string e)
{
	simple = "";
	expr.clear();

	vars = new vspace();
	internal_vars = true;
	gen_variables(e);
	gen_minterms(e);
	expr.mccluskey();
	gen_output();

	return *this;
}

expression &expression::operator()(vector<minterm> t, vspace *v)
{
	simple = "";
	expr.clear();

	expr.terms = t;
	vars = v;
	internal_vars = false;
	expr.mccluskey();
	gen_output();

	return *this;
}

list<string> extract_vars(string exp)
{
	list<string> ret;
	size_t i = string::npos, j = 0;
	while ((i = exp.find_first_of("&|", i+1)) != string::npos)
	{
		ret.push_back(exp.substr(j, i-j));
		j = i+1;
	}

	ret.push_back(exp.substr(j));

	ret.sort();
	ret.unique();
	return ret;
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

bdd solve(string raw, vspace *vars, string tab, int verbosity)
{
	/*int id;
	state outcomes;
	string::iterator i, j;
	int depth;

	if (vars->global.size() != 0)
		outcomes.assign(vars->global.size()-1, value("?"), value("?"));

	//if (verbosity >= VERB_PARSE)
	//	cout << tab << "Solve: " << raw << endl;

	depth = 0;
	for (i = raw.begin(), j = raw.begin(); i != raw.end()+1; i++)
	{
		if (*i == '(')
			depth++;
		else if (*i == ')')
			depth--;

		if (depth == 0 && *i == '|')
		{
			outcomes = (solve(raw.substr(j-raw.begin(), i-j), vars, tab+"\t", verbosity) ||
						solve(raw.substr(i+1-raw.begin()), vars, tab+"\t", verbosity));

			//if (verbosity >= VERB_PARSE)
			//	cout << tab << outcomes << endl;

			return outcomes;
		}
	}

	depth = 0;
	for (i = raw.begin(), j = raw.begin(); i != raw.end()+1; i++)
	{
		if (*i == '(')
			depth++;
		else if (*i == ')')
			depth--;

		if (depth == 0 && *i == '&')
		{
			outcomes = (solve(raw.substr(j-raw.begin(), i-j), vars, tab+"\t", verbosity) &&
						solve(raw.substr(i+1-raw.begin()), vars, tab+"\t", verbosity));

			//if (verbosity >= VERB_PARSE)
			//	cout << tab << outcomes << endl;

			return outcomes;
		}
	}

	depth = 0;
	for (i = raw.begin(), j = raw.begin(); i != raw.end()+1; i++)
	{
		if (*i == '(')
			depth++;
		else if (*i == ')')
			depth--;

		if (depth == 0 && *i == '~')
		{
			outcomes = ~solve(raw.substr(i+1-raw.begin()), vars, tab+"\t", verbosity);

			//if (verbosity >= VERB_PARSE)
			//	cout << tab << outcomes << endl;

			return outcomes;
		}
	}

	unsigned long s = raw.find_first_of("(");
	unsigned long e = raw.find_last_of(")");
	if (s != raw.npos && e != raw.npos)
	{
		outcomes = solve(raw.substr(s+1, e-s-1), vars, tab+"\t", verbosity);

		//if (verbosity >= VERB_PARSE)
		//	cout << tab << outcomes << endl;

		return outcomes;
	}

	id = vars->get_uid(raw);
	if (id != -1)
		outcomes.assign(id, value("1"), value("?"));
	// TODO This is important for constants within guards such as a & 1 or a & 0, but breaks guards that are constants like 1 or 0
	else if (raw == "1")
		for (map<string, variable>::iterator vi = vars->global.begin(); vi != vars->global.end(); vi++)
			outcomes.assign(vi->second.uid, value("X"), value("?"));
	else if (raw == "0")
		for (map<string, variable>::iterator vi = vars->global.begin(); vi != vars->global.end(); vi++)
			outcomes.assign(vi->second.uid, value("_"), value("?"));
	else
		cout << "Error: Undefined variable " << raw << "." << endl;
	//if (verbosity >= VERB_PARSE)
	//	cout << tab << outcomes << endl;

	return outcomes;*/
}

minterm estimate(string e, vspace *vars)
{
	size_t p;

	p = find_first_of_l0(e, "|");
	if (p != e.npos)
		return estimate(e.substr(0, p), vars) || estimate(e.substr(p+1), vars);

	p = find_first_of_l0(e, "&");
	if (p != e.npos)
		return estimate(e.substr(0, p), vars) || estimate(e.substr(p+1), vars);

	p = find_first_of_l0(e, "~");
	if (p != e.npos)
		return estimate(e.substr(p+1), vars);

	if (e[0] == '(' && e[e.length()-1] == ')')
		return estimate(e.substr(1, e.length()-2), vars);

	minterm result(vars->global.size(), v_);
	size_t id = vars->get_uid(e);
	if (id >= 0 && id < vars->global.size())
		result.inelastic_set(id, vX);
	return result;
}
