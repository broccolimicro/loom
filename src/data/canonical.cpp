/*
 * canonical.cpp
 *
 *  Created on: May 11, 2013
 *      Author: nbingham
 */

#include "canonical.h"
#include "../utility.h"

canonical::canonical()
{
	terms = vector<minterm>();
}

canonical::canonical(int s)
{
	terms.resize(s);
}

canonical::canonical(minterm m)
{
	terms.push_back(m);
}

canonical::canonical(vector<minterm> m)
{
	terms = m;
	mccluskey();
}

canonical::canonical(string s, vspace *vars)
{
	vector<string> t = distribute(demorgan(s, -1, false));

	for (int i = 0; i < (int)t.size(); i++)
	{
		minterm m(t[i], vars);
		if (!has_(m))
			terms.push_back(m);
	}
	mccluskey();
}

canonical::canonical(string s, vector<string> vars)
{
	vector<string> t = distribute(demorgan(s, -1, false));

	for (int i = 0; i < (int)t.size(); i++)
	{
		minterm m(t[i], vars);
		if (!has_(m))
			terms.push_back(m);
	}
	mccluskey();
}

canonical::~canonical()
{
	terms.clear();
}

int canonical::size()
{
	return terms.size();
}

int canonical::width()
{
	if (terms.size() > 0)
		return terms[0].size;
	else
		return 0;
}

void canonical::assign(int i, minterm t)
{
	if (i >= (int)terms.size())
		terms.resize(i+1, minterm());
	terms[i] = t;
}

void canonical::push_back(minterm m)
{
	terms.push_back(m);
}

void canonical::push_up(minterm m)
{
	if ((int)terms.size() < m.size)
		terms.resize(m.size);

	for (int i = 0; i < (int)terms.size(); i++)
		terms[i].push_back(m[i]);
}

void canonical::remove(int i)
{
	vector<minterm>::iterator k = terms.begin();
	for (int j = 0; j < i; j++)
		k++;

	terms.erase(k);
}

void canonical::clear()
{
	terms.clear();
}

bool canonical::always_0()
{
	return (terms.size() == 0);
}

bool canonical::always_1()
{
	return (terms.size() == 1 && allX(terms[0]));
}

void canonical::mccluskey()
{
	vector<minterm> implicants;
	vector<minterm> primes;
	vector<size_t> essentials;
	vector<minterm> t[2] = {vector<minterm>(), vector<minterm>()};
	minterm implicant;

	size_t i, j, k;

	vector<int> count;
	int count_sum;

	map<size_t, vector<size_t> > cov, Tcov;
	vector<size_t>::iterator ci;

	size_t max_count = implicants.size();
	size_t choice;

	t[1] = terms;
	implicants = terms;
	terms.clear();

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


	// Essential Prime Implicants
	cov.clear();
	for (j = 0; j < implicants.size(); j++)
		cov.insert(pair<size_t, vector<size_t> >(j, vector<size_t>()));
	for (j = 0; j < implicants.size(); j++)
	{
		for (i = 0; i < primes.size(); i++)
			if (subset(primes[i], implicants[j]))
				cov[j].push_back(i);

		if (cov[j].size() == 1 && find(essentials.begin(), essentials.end(), cov[j].front()) == essentials.end())
		{
			essentials.push_back(cov[j].front());
			terms.push_back(primes[cov[j].front()]);
		}
	}

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
			terms.push_back(primes[choice]);

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
}


string canonical::print()
{
	if (terms.size() == 0)
		return "0";
	else if (terms.size() == 1 && allX(terms[0]))
		return "1";

	ostringstream res;
	for (int i = 0; i < (int)terms.size(); i++)
	{
		if (i != 0)
			res << "|";
		res << terms[i].print_expr();
	}
	return res.str();
}

string canonical::print(vector<string> vars)
{
	if (terms.size() == 0)
		return "0";
	else if (terms.size() == 1 && allX(terms[0]))
		return "1";

	ostringstream res;
	for (int i = 0; i < (int)terms.size(); i++)
	{
		if (i != 0)
			res << "|";
		res << terms[i].print_expr(vars);
	}
	return res.str();
}

vector<minterm>::iterator canonical::begin()
{
	return terms.begin();
}

vector<minterm>::iterator canonical::end()
{
	return terms.end();
}

canonical &canonical::operator=(canonical c)
{
	this->terms = c.terms;
	return *this;
}

canonical canonical::operator()(minterm m)
{
	canonical result;
	minterm x;
	for (size_t i = 0; i < terms.size(); i++)
	{
		x = terms[i] && m;
		if (!has_(x))
			result.push_back(terms[i] || (!m));
	}

	result.mccluskey();
	return result;
}

canonical canonical::operator()(int i, uint32_t v)
{
	canonical result;
	minterm x;
	for (size_t j = 0; j < terms.size(); j++)
	{
		x = terms[j];
		x.sv_intersect(i, v);
		if (!has_(x))
		{
			x.sv_union(i, vX);
			result.push_back(x);
		}
	}

	result.mccluskey();
	return result;
}

minterm canonical::operator[](int i)
{
	return terms[i];
}

minterm canonical::operator()(int i)
{
	minterm ret;
	vector<minterm>::iterator t;

	for (t = terms.begin(); t != terms.end(); t++)
		ret.push_back((*t)[i]);

	return ret;
}
