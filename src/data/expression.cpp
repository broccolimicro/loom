/*
 * expression.cpp
 *
 *  Created on: Mar 15, 2013
 *      Author: nbingham
 */

#include "expression.h"
#include "../common.h"
#include "state.h"
#include "vspace.h"
#include "tspace.h"

expression::expression()
{
	simple = "";
	vars = NULL;
	internal_vars = false;
	implicants.clear();
	primes.clear();
	essentials.clear();
}

expression::expression(string e)
{
	simple = "";
	implicants.clear();
	primes.clear();
	essentials.clear();

	vars = new vspace();
	internal_vars = true;
	gen_variables(e);
	gen_minterms(e);
	gen_primes();
	gen_essentials();
	gen_output();
}

expression::expression(vector<state> t, vspace *v)
{
	simple = "";
	implicants.clear();
	primes.clear();
	essentials.clear();

	implicants = t;
	vars = v;
	internal_vars = false;
	gen_primes();
	gen_essentials();
	gen_output();
}

expression::~expression()
{
	simple = "";
	if (vars != NULL && internal_vars)
		delete vars;
	vars = NULL;
	internal_vars = false;
	implicants.clear();
	primes.clear();
	essentials.clear();
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
		vars->insert(variable(e, "int", value("X"), 1, false));
		cout << e << endl;
	}
}

void expression::gen_minterms(string e)
{
	int i, j, k, m, n;
	string v;
	trace_space values(vars->global.size());

	cout << "Minterms " << vars->global.size() << endl;

	m = pow(2, values.size())/2;
	n = 2;
	for (i = 0; i < (int)vars->global.size(); i++)
	{
		v = "0";
		for (j = 0; j < n; j++)
		{
			for (k = 0; k < m; k++)
				values[i].values.push_back(value(v));
			if (v == "0")
				v = "1";
			else
				v = "0";
		}

		m /= 2;
		n *= 2;

		cout << values[i] << endl;
	}

	trace result = evaluate(e, vars, values.traces);
	cout << "Result " << result << endl;

	for (i = 0; i < result.size(); i++)
		if (result[i].data == "1")
			implicants.push_back(values(i));
}

void expression::gen_primes()
{
	if (vars == NULL || implicants.size() == 0)
		return;

	vector<state> t[2];
	state implicant;
	size_t i, j;

	vector<int> count;
	int count_sum;

	cout << "Minterms\t";
	for (i = 0; i < implicants.size(); i++)
		cout << "[" << implicants[i] << "] ";
	cout << endl;

	t[1] = implicants;

	count_sum = t[1].size();
	while (count_sum > 0)
	{
		t[0].clear();
		count.clear();
		count.resize(t[1].size(), 0);
		for (i = 0; i < t[1].size(); i++)
		{
			for (j = i+1; j < t[1].size(); j++)
			{
				if (diff_count(t[1][i], t[1][j]) <= 1)
				{
					implicant = t[1][i] || t[1][j];
					count[i]++;
					count[j]++;
					if (find(t[0].begin(), t[0].end(), implicant) == t[0].end())
						t[0].push_back(implicant);
				}
			}
		}
		count_sum = 0;
		for (i = 0; i < t[1].size(); i++)
		{
			count_sum += count[i];
			if (count[i] == 0)
				primes.push_back(t[1][i]);
		}

		t[1] = t[0];
	}

	cout << "Primes\t";
	for (i = 0; i < primes.size(); i++)
		cout << "[" << primes[i] << "] ";
	cout << endl;
}

void expression::gen_essentials()
{
	if (vars == NULL || primes.size() == 0)
		return;

	map<size_t, vector<size_t> > cov, Tcov;
	vector<size_t>::iterator ci;
	size_t i, j, k;
	size_t max_count = implicants.size();
	size_t choice;

	cout << "Prime Implicant Chart" << endl;
	cov.clear();
	for (j = 0; j < implicants.size(); j++)
		cov.insert(pair<size_t, vector<size_t> >(j, vector<size_t>()));
	for (j = 0; j < implicants.size(); j++)
	{
		cout << implicants[j] << " is covered by ";
		for (i = 0; i < primes.size(); i++)
			if (subset(primes[i], implicants[j]))
			{
				cout << "[" << primes[i] << "] ";
				cov[j].push_back(i);
			}

		if (cov[j].size() == 1 && find(essentials.begin(), essentials.end(), cov[j].front()) == essentials.end())
			essentials.push_back(cov[j].front());

		cout << endl;
	}

	cout << endl;

	cout << "Essential Prime Implicants" << endl;
	for (j = 0; j < essentials.size(); j++)
		cout << "[" << primes[essentials[j]] << "]" << endl;
	cout << endl;

	Tcov.clear();
	for (j = 0; j < primes.size(); j++)
		Tcov.insert(pair<size_t, vector<size_t> >(j, vector<size_t>()));
	for (j = 0; j < cov.size(); j++)
	{
		for (i = 0; i < essentials.size(); i++)
			if (find(cov[j].begin(), cov[j].end(), essentials[i]) != cov[j].end())
				break;

		for (k = 0; i == essentials.size() && k < cov[j].size(); k++)
			Tcov[cov[j][k]].push_back(j);
	}

	cout << "Leftover Non-Essential Prime Implicants" << endl;
	for (i = 0; i < primes.size(); i++)
	{
		if (Tcov[i].size() > 0)
		{
			cout << primes[i] << " covers ";
			for (j = 0; j < Tcov[i].size(); j++)
				cout << "[" << implicants[Tcov[i][j]] << "] ";
			cout << endl;
		}
	}
	cout << endl;

	max_count = implicants.size();
	while (max_count > 0)
	{
		max_count = 0;
		for (i = 0; i < primes.size(); i++)
		{
			if (Tcov[i].size() > max_count)
			{
				max_count = Tcov.size();
				choice = i;
			}
		}

		if (max_count > 0)
		{
			essentials.push_back(choice);

			for (i = 0; i < primes.size(); i++)
				for (j = 0; i != choice && j < Tcov[choice].size(); j++)
				{
					ci = find(Tcov[i].begin(), Tcov[i].end(), Tcov[choice][j]);
					if (ci != Tcov[i].end())
						Tcov[i].erase(ci);
				}

			Tcov[choice].clear();
		}
	}

	cout << "Best Essential Prime Implicants" << endl;
	for (j = 0; j < essentials.size(); j++)
		cout << "[" << primes[essentials[j]] << "]" << endl;
	cout << endl;
}

void expression::gen_output()
{
	if (vars == NULL)
		return;

	vector<size_t>::iterator i;
	int j;
	bool first;

	simple = "";
	for (i = essentials.begin(); i != essentials.end(); i++)
	{
		if (i != essentials.begin())
			simple += "|";

		first = true;
		for (j = 0; j < primes[*i].size(); j++)
		{
			if (primes[*i].values[j].data == "0")
			{
				if (!first)
					simple += "&";
				simple += "~" + vars->get_name(j);
				first = false;
			}
			else if (primes[*i].values[j].data == "1")
			{
				if (!first)
					simple += "&";
				simple += vars->get_name(j);
				first = false;
			}
		}
		if (first)
			simple += "1";
	}
	if (simple == "")
		simple += "0";

	cout << "Final Result " << simple << endl;
}

expression &expression::operator()(string e)
{
	simple = "";
	implicants.clear();
	primes.clear();
	essentials.clear();

	vars = new vspace();
	internal_vars = true;
	gen_variables(e);
	gen_minterms(e);
	gen_primes();
	gen_essentials();
	gen_output();

	return *this;
}

expression &expression::operator()(vector<state> t, vspace *v)
{
	simple = "";
	implicants.clear();
	primes.clear();
	essentials.clear();

	implicants = t;
	vars = v;
	internal_vars = false;
	gen_primes();
	gen_essentials();
	gen_output();

	return *this;
}
