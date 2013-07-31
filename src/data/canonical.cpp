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
	vector<string> t = distribute(demorgan(s, -1, false));

	for (int i = 0; i < (int)t.size(); i++)
	{
		minterm m(t[i], vars);
		if (!has_(m))
			terms.push_back(m);
	}
	mccluskey();
}

/**
 * \brief	A constructor that parses a string and uses it to fill the canonical expression with values.
 * \param	s		A string that represents the expression.
 * \param	vars	A list of all the of variable names indexed by variable index.
 */
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
		terms[i].push_back(m[i]);
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
 * \brief	Checks to see if this expression is always false.
 */
bool canonical::always_0()
{
	return (terms.size() == 0);
}

/**
 * \brief	Checks to see if this expression is always true.
 */
bool canonical::always_1()
{
	return (terms.size() == 1 && allX(terms[0]));
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

	t[1] = terms;
	implicants = terms;
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


	/**
	 * 2.	Use those prime implicants in a prime implicant chart to find the essential prime implicants of the expression, as well as other prime implicants that are necessary to cover the expression.
	 */
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

canonical canonical::restrict(int j, uint32_t b)
{
	canonical result;
	int i;
	for (i = 0; i < (int)terms.size(); i++)
		if ((terms[i][j] & b) != 0)
		{
			result.push_back(terms[i]);
			result.terms[result.terms.size()-1].sv_union(j, vX);
		}
	result.mccluskey();
	return result;
}

canonical canonical::smooth(int j)
{
	canonical result;
	int i;
	for (i = 0; i < (int)terms.size(); i++)
	{
		result.push_back(terms[i]);
		result.terms[result.terms.size()-1].sv_union(j, vX);
	}
	result.mccluskey();
	return result;
}

canonical canonical::smooth(vector<int> j)
{
	canonical result;
	int i, k;
	for (i = 0; i < (int)terms.size(); i++)
		for (k = 0; k < (int)j.size(); k++)
		{
			result.push_back(terms[i]);
			result.terms[result.terms.size()-1].sv_union(j[k], vX);
		}
	result.mccluskey();
	return result;
}

void canonical::extract(map<int, uint32_t> *result)
{
	minterm m(terms[0].size, v_);
	for (int i = 0; i < (int)terms.size(); i++)
		m = m || terms[i];
	m.extract(result);
}

canonical canonical::get_pos()
{
	canonical result;
	for (int i = 0; i < (int)terms.size(); i++)
		result.push_back(terms[i].get_pos());
	result.mccluskey();
	return result;
}

canonical canonical::get_neg()
{
	canonical result;
	for (int i = 0; i < (int)terms.size(); i++)
		result.push_back(terms[i].get_neg());
	result.mccluskey();
	return result;
}

/**
 * \brief	Prints the sum of minterms to stdout where each variable is given the name xi where i is the variable index.
 */
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

/**
 * \brief	Prints the sum of minterms to stdout.
 * \param	vars	A list of variable names indexed by variable index.
 */
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

canonical &canonical::operator=(canonical c)
{
	this->terms = c.terms;
	return *this;
}

/**
 * \brief	Restricts all variables to the values contained in the minterm m.
 * \details	This does not modify the canonical this was applied on. Instead, it
 * 			makes a copy, does the operation and returns the copy.
 * \param	m	The values to restrict the variables to (v0, v1, vX, or v_).
 * \return	The canonical that results from this restriction.
 */
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

/**
 * \brief	Returns the minterm located at index i.
 * \param	i	The index of the minterm to lookup.
 */
minterm canonical::operator[](int i)
{
	return terms[i];
}

/**
 * \brief	Returns the value of the variable whose index is i in each minterm.
 * \param	i	The index of the variable to lookup.
 */
minterm canonical::operator()(int i)
{
	minterm ret;
	vector<minterm>::iterator t;

	for (t = terms.begin(); t != terms.end(); t++)
		ret.push_back((*t)[i]);

	return ret;
}

canonical operator|(canonical c1, canonical c2)
{
	canonical result;
	result.terms.insert(result.terms.end(), c1.terms.begin(), c1.terms.end());
	result.terms.insert(result.terms.end(), c2.terms.begin(), c2.terms.end());
	result.mccluskey();
	return result;
}

canonical operator&(canonical c1, canonical c2)
{
	canonical result;
	int i, j;
	for (i = 0; i < c1.terms.size(); i++)
		for (j = 0; j < c2.terms.size(); j++)
			result.terms.push_back(c1.terms[i] & c2.terms[j]);
	result.mccluskey();
	return result;
}

canonical operator~(canonical c)
{
	canonical result;
	int i;
	for (i = 0; i < c.terms.size(); i++)
		result = result & ~c.terms[i];
	result.mccluskey();
	return result;
}

canonical operator+(canonical s, minterm t)
{
	canonical result;
	minterm m = t.mask();
	int i;
	for (i = 0; i < (int)s.terms.size(); i++)
		result.terms.push_back((s.terms[i] || m) && t);
	result.mccluskey();
	return result;
}
