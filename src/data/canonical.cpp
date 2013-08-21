/*
 * canonical.cpp
 *
 *  Created on: May 11, 2013
 *      Author: nbingham
 */

#include "canonical.h"
#include "variable_space.h"
#include "../utility.h"

canonical::canonical()
{
	terms = vector<minterm>();
}

canonical::canonical(int s)
{
	terms.resize(s);
}

/**
 * \brief	Constructor that adds the minterm m to the canonical
 * \param	m	The one minterm to add.
 */
canonical::canonical(minterm m)
{
	terms.push_back(m);
}

/**
 * \brief	Constructor that adds all minterms in m to the canonical.
 * \param	m	All minterms to add.
 */
canonical::canonical(vector<minterm> m)
{
	terms = m;
	mccluskey();
}

/**
 * \brief	A constructor that parses a string and uses it to fill the canonical expression with values.
 * \param	s		A string that represents the expression.
 * \param	vars	The variable space used to parse the string.
 */
canonical::canonical(string s, variable_space *vars)
{
	if (s == "0")
		return;

	vector<string> t = distribute(demorgan(s, -1, false));

	for (int i = 0; i < (int)t.size(); i++)
	{
		minterm m(t[i], vars);
		if (m != 0)
			terms.push_back(m);
	}
	mccluskey();
}

canonical::canonical(int var, uint32_t val)
{
	terms.push_back(minterm(var, val));
}

canonical::canonical(map<int, uint32_t> vals)
{
	terms.push_back(minterm(vals));
}

canonical::~canonical()
{
	terms.clear();
}

/**
 * \brief	Returns an iterator that represents the beginning of the list of minterms.
 */
vector<minterm>::iterator canonical::begin()
{
	return terms.begin();
}

/**
 * \brief	Returns an iterator that represents the end of the list of minterms.
 */
vector<minterm>::iterator canonical::end()
{
	return terms.end();
}

/**
 * \brief	Returns the total number of minterms in this canonical expression.
 */
int canonical::size()
{
	return terms.size();
}

/**
 * \brief	Returns the total number of variables contained in this canonical expression.
 */
int canonical::width()
{
	if (terms.size() > 0)
		return terms[0].size;
	else
		return 0;
}

/**
 * \brief	Sets the term located at index i in this canonical expression equal to t.
 * \param	i	The index of the term to assign.
 * \param	t	The new minterm to use.
 */
void canonical::assign(int i, minterm t)
{
	if (i >= (int)terms.size())
		terms.resize(i+1, minterm());
	terms[i] = t;
}

/**
 * \brief	Adds another minterm to this canonical expression.
 * \details Specifically it represents the operation this = this | m.
 * \param	m	The minterm to add.
 */
void canonical::push_back(minterm m)
{
	terms.push_back(m);
}

/**
 * \brief	Adds a variable to every minterm in this canonical expression.
 * \param	m	The relation of the added variable to each minterm in the canonical.
 */
void canonical::push_up(minterm m)
{
	if ((int)terms.size() < m.size)
		terms.resize(m.size);

	for (int i = 0; i < (int)terms.size(); i++)
		terms[i].push_back(m.get(i));
}

/**
 * \brief	Removes a minterm from this canonical.
 * \param	i	The index of the minterm to remove.
 */
void canonical::remove(int i)
{
	terms.erase(terms.begin() + i);
}

/**
 * \brief	Remove all minterms from this canonical, effectively setting this expression to be 0.
 */
void canonical::clear()
{
	terms.clear();
}

/**
 * \brief	Executes the Quine-McCluskey algorithm, also known as the method of prime implicants.
 * \details	a method used for minimization of boolean functions which was developed by W.V. Quine
 * 			and Edward J. McCluskey in 1956. It is functionally identical to Karnaugh mapping, but
 * 			the tabular form makes it more efficient for use in computer algorithms, and it also
 * 			gives a deterministic way to check that the minimal form of a Boolean function has been
 * 			reached. It is sometimes referred to as the tabulation method. The method involves two steps:
 */
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

	for (i = 0; i < terms.size(); i++)
	{
		if (terms[i] != 0)
		{
			t[1].push_back(terms[i]);
			implicants.push_back(terms[i]);
		}
	}
	terms.clear();

	/**
	 * 1.	Find all prime implicants of the expression.
	 */
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
				if (t[1][i].diff_count(t[1][j]) <= 1)
				{
					implicant = t[1][i] | t[1][j];
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


	/**
	 * 2.	Use those prime implicants in a prime implicant chart to find the essential prime implicants of the expression, as well as other prime implicants that are necessary to cover the expression.
	 */
	cov.clear();
	for (j = 0; j < implicants.size(); j++)
		cov.insert(pair<size_t, vector<size_t> >(j, vector<size_t>()));
	for (j = 0; j < implicants.size(); j++)
	{
		for (i = 0; i < primes.size(); i++)
			if (primes[i].subset(implicants[j]))
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

minterm canonical::mask()
{
	minterm result;
	for (int i = 0; i < (int)terms.size(); i++)
		result |= terms[i].mask();
	return result;
}

void canonical::vars(vector<int> *var_list)
{
	for (int i = 0; i < (int)terms.size(); i++)
		terms[i].vars(var_list);
}

vector<int> canonical::vars()
{
	vector<int> result;
	for (int i = 0; i < (int)terms.size(); i++)
		terms[i].vars(&result);
	return result;
}

canonical canonical::refactor(vector<int> ids)
{
	canonical result;
	for (int i = 0; i < (int)terms.size(); i++)
		result.terms.push_back(terms[i].refactor(ids));
	return result;
}

canonical canonical::smooth(int var)
{
	canonical result;
	int i;
	for (i = 0; i < (int)terms.size(); i++)
		result.terms.push_back(terms[i].smooth(var));
	result.mccluskey();
	return result;
}

canonical canonical::smooth(vector<int> vars)
{
	canonical result;
	for (int i = 0; i < (int)terms.size(); i++)
		result.terms.push_back(terms[i].smooth(vars));
	result.mccluskey();
	return result;
}

void canonical::extract(map<int, canonical> *result)
{
	minterm m;
	vector<int> v = vars();
	unique(&v);

	for (int i = 0; i < (int)terms.size(); i++)
		m = (i == 0 ? terms[i] : m | terms[i]);

	for (int i = 0; i < (int)v.size(); i++)
		result->insert(pair<int, canonical>(v[i], canonical(m[v[i]])));
}

map<int, canonical> canonical::extract()
{
	map<int, canonical> result;
	minterm m;
	vector<int> v = vars();
	unique(&v);

	for (int i = 0; i < (int)terms.size(); i++)
		m = (i == 0 ? terms[i] : m | terms[i]);

	for (int i = 0; i < (int)v.size(); i++)
		result.insert(pair<int, canonical>(v[i], canonical(m[v[i]])));
	return result;
}

canonical canonical::pabs()
{
	canonical result;
	for (int i = 0; i < (int)terms.size(); i++)
		result.push_back(terms[i].pabs());
	result.mccluskey();
	return result;
}

canonical canonical::nabs()
{
	canonical result;
	for (int i = 0; i < (int)terms.size(); i++)
		result.push_back(terms[i].nabs());
	result.mccluskey();
	return result;
}

int canonical::satcount()
{
	return (int)terms.size();
}

map<int, uint32_t> canonical::anysat()
{
	return terms[0].anysat();
}

vector<map<int, uint32_t> > canonical::allsat()
{
	vector<map<int, uint32_t> > sats;
	for (int i = 0; i < (int)terms.size(); i++)
		sats.push_back(terms[i].anysat());
	return sats;
}

canonical &canonical::operator=(canonical c)
{
	this->terms = c.terms;
	return *this;
}

canonical &canonical::operator=(minterm t)
{
	this->terms.clear();
	this->terms.push_back(t);
	return *this;
}

canonical &canonical::operator=(uint32_t c)
{
	terms.clear();
	if (c == 1)
		terms.push_back(minterm());
	return *this;
}

canonical &canonical::operator|=(canonical c)
{
	*this = *this | c;
	return *this;
}
canonical &canonical::operator&=(canonical c)
{
	*this = *this & c;
	return *this;
}

canonical &canonical::operator|=(uint32_t c)
{
	if (c == 1)
	{
		terms.clear();
		terms.push_back(minterm());
	}
	return *this;
}
canonical &canonical::operator&=(uint32_t c)
{
	if (c == 0)
		terms.clear();
	return *this;
}

/**
 * \brief	Restricts the variable whose index is i to the value v.
 * \details	Given a binary boolean expression f, a variable x whose index is i,
 * 			and a value v, this calculates f(x = v). This does not modify the
 * 			canonical this was applied on. Instead, it makes a copy, does the
 * 			operation and returns the copy.
 * \param	i	The index of the variable to restrict.
 * \param	v	The value to restrict the variable to (v0, v1, vX, or v_).
 * \return	The canonical that results from this restriction.
 */
canonical canonical::operator()(int j, uint32_t b)
{
	canonical result;
	minterm temp;
	int i;
	uint32_t v;
	for (i = 0; i < (int)terms.size(); i++)
	{
		v = terms[i].val(j);
		if (v == 2 || v == b)
			result.push_back(terms[i](j, b));
	}
	result.mccluskey();
	return result;
}

canonical canonical::operator[](int i)
{
	canonical result;
	result.push_back(minterm());
	for (int j = 0; j < (int)terms.size(); j++)
		result.terms[0].sv_union(i, terms[j].get(i));
	return result;
}

canonical canonical::operator|(canonical c)
{
	canonical result;
	result.terms.insert(result.terms.end(), terms.begin(), terms.end());
	result.terms.insert(result.terms.end(), c.terms.begin(), c.terms.end());
	result.mccluskey();
	return result;
}

canonical canonical::operator&(canonical c)
{
	canonical result;
	int i, j;
	for (i = 0; i < (int)terms.size(); i++)
		for (j = 0; j < (int)c.terms.size(); j++)
			result.terms.push_back(terms[i] & c.terms[j]);
	result.mccluskey();
	return result;
}

canonical canonical::operator~()
{
	canonical result = 1;
	int i;
	for (i = 0; i < (int)terms.size(); i++)
		result = result & ~terms[i];
	result.mccluskey();
	return result;
}

canonical canonical::operator|(uint32_t c)
{
	canonical result;
	if (c == 1)
		result.terms.push_back(minterm());
	else
		result = *this;
	return result;
}

canonical canonical::operator&(uint32_t c)
{
	if (c == 1)
		return *this;
	else
		return canonical();
}

bool canonical::operator==(canonical c)
{
	bool result = (terms.size() == c.terms.size());
	for (int i = 0; i < (int)terms.size() && result; i++)
		result = result && (terms[i] == c.terms[i]);
	return result;
}

bool canonical::operator!=(canonical c)
{
	bool result = (terms.size() == c.terms.size());
	for (int i = 0; i < (int)terms.size() && result; i++)
		result = result && (terms[i] == c.terms[i]);
	return !result;
}

bool canonical::operator==(uint32_t c)
{
	if (c == 0)
	{
		bool zero = true;
		for (int i = 0; i < (int)terms.size() && zero; i++)
			zero = zero && (terms[i] == 0);
		return zero;
	}
	else if (c == 1)
	{
		bool one = false;
		for (int i = 0; i < (int)terms.size() && !one; i++)
			one = one || (terms[i] == 1);
		return one;
	}
	else
		return false;
}

bool canonical::operator!=(uint32_t c)
{
	if (c == 0)
	{
		bool zero = true;
		for (int i = 0; i < (int)terms.size() && zero; i++)
			zero = zero && (terms[i] == 0);
		return !zero;
	}
	else if (c == 1)
	{
		bool one = false;
		for (int i = 0; i < (int)terms.size() && !one; i++)
			one = one || (terms[i] == 1);
		return !one;
	}
	else
		return true;
}

bool canonical::constant()
{
	return ((terms.size() == 0) || (terms.size() == 1 && terms[0] == 1));
}

canonical canonical::operator>>(canonical c)
{
	canonical result;
	int i, j;

	for (i = 0; i < (int)terms.size(); i++)
		for (j = 0; j < (int)c.terms.size(); j++)
			result.terms.push_back(terms[i] >> c.terms[j]);
	result.mccluskey();

	return result;
}

/**
 * \brief	Prints the sum of minterms to stdout.
 * \param	vars	A list of variable names indexed by variable index.
 */
string canonical::print(variable_space *vars, string prefix)
{
	if (terms.size() == 0)
		return "0";
	else if (terms.size() == 1 && terms[0] == 1)
		return "1";

	string res;
	for (int i = 0; i < (int)terms.size(); i++)
	{
		if (i != 0)
			res += "|";
		res += terms[i].print(vars, prefix);
	}
	return res;
}

