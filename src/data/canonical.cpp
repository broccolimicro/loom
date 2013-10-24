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
	terms = svector<minterm>();
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
canonical::canonical(svector<minterm> m)
{
	terms = m;
	mccluskey();
}

/**
 * \brief	A constructor that parses a sstring and uses it to fill the canonical expression with values.
 * \param	s		A sstring that represents the expression.
 * \param	vars	The variable space used to parse the sstring.
 */
canonical::canonical(sstring s, variable_space *vars)
{
	if (s == "0")
		return;

	svector<sstring> t = distribute(demorgan(s, -1, false));

	for (int i = 0; i < t.size(); i++)
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

canonical::canonical(smap<int, uint32_t> vals)
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
svector<minterm>::iterator canonical::begin()
{
	return terms.begin();
}

/**
 * \brief	Returns an iterator that represents the end of the list of minterms.
 */
svector<minterm>::iterator canonical::end()
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
 * \brief	Sets the term located at index i in this canonical expression equal to t.
 * \param	i	The index of the term to assign.
 * \param	t	The new minterm to use.
 */
void canonical::assign(int i, minterm t)
{
	if (i >= terms.size())
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
	if (terms.size() < m.size)
		terms.resize(m.size);

	for (int i = 0; i < terms.size(); i++)
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
	canonical temp;
	svector<minterm> implicants;
	svector<minterm> primes;
	svector<int> essentials;
	svector<minterm> t[2] = {svector<minterm>(), svector<minterm>()};
	minterm implicant;

	int i, j, k;

	svector<int> count;
	int count_sum;
	svector<int> vl;

	smap<int, svector<int> > cov, Tcov;
	svector<int>::iterator ci;

	int max_count = implicants.size();
	int choice;
	pair<int, int> xdiff;
	int diff;

	for (i = 0; i < terms.size(); i++)
		if (terms[i] != 0)
			implicants.push_back(terms[i]);

	t[1] = implicants;
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
				xdiff = t[1][i].xdiff_count(t[1][j]);
				diff = t[1][i].diff_count(t[1][j]);
				if ((diff <= 1 && xdiff.first + xdiff.second <= 1) ||
					(xdiff.first == 0 && diff - xdiff.second == 0) ||
					(xdiff.second == 0 && diff - xdiff.first == 0))
				{
					implicant = t[1][i] | t[1][j];
					count[i]++;
					count[j]++;
					if (t[0].find(implicant) == t[0].end())
						t[0].push_back(implicant);
				}
				else if (xdiff.first == 0 && diff - xdiff.second == 1)
				{
					implicant = (t[1][i] & t[1][j]).xoutnulls();
					count[i]++;
					if (t[0].find(implicant) == t[0].end())
						t[0].push_back(implicant);
				}
				else if (xdiff.second == 0 && diff - xdiff.first == 1)
				{
					implicant = (t[1][i] & t[1][j]).xoutnulls();
					count[j]++;
					if (t[0].find(implicant) == t[0].end())
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
		cov.insert(pair<int, svector<int> >(j, svector<int>()));
	for (j = 0; j < implicants.size(); j++)
	{
		for (i = 0; i < primes.size(); i++)
			if (primes[i].subset(implicants[j]))
				cov[j].push_back(i);

		if (cov[j].size() == 1 && essentials.find(cov[j].front()) == essentials.end())
		{
			essentials.push_back(cov[j].front());
			terms.push_back(primes[cov[j].front()]);
		}
	}

	Tcov.clear();
	for (j = 0; j < primes.size(); j++)
		Tcov.insert(pair<int, svector<int> >(j, svector<int>()));
	for (j = 0; j < cov.size(); j++)
	{
		for (i = 0; i < essentials.size(); i++)
			if (cov[j].find(essentials[i]) != cov[j].end())
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
					ci = Tcov[i].find(Tcov[choice][j]);
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
	for (int i = 0; i < terms.size(); i++)
		result |= terms[i].mask();
	return result;
}

void canonical::vars(svector<int> *var_list)
{
	for (int i = 0; i < terms.size(); i++)
		terms[i].vars(var_list);
}

svector<int> canonical::vars()
{
	svector<int> result;
	for (int i = 0; i < terms.size(); i++)
		terms[i].vars(&result);
	return result;
}

canonical canonical::refactor(svector<int> ids)
{
	canonical result;
	for (int i = 0; i < terms.size(); i++)
		result.terms.push_back(terms[i].refactor(ids));
	return result;
}

canonical canonical::refactor(svector<pair<int, int> > ids)
{
	canonical result;
	for (int i = 0; i < terms.size(); i++)
		result.terms.push_back(terms[i].refactor(ids));
	return result;
}


canonical canonical::hide(int var)
{
	canonical result;
	int i;
	for (i = 0; i < terms.size(); i++)
		result.terms.push_back(terms[i].hide(var));
	result.mccluskey();
	return result;
}

canonical canonical::hide(svector<int> vars)
{
	canonical result;
	for (int i = 0; i < terms.size(); i++)
		result.terms.push_back(terms[i].hide(vars));
	result.mccluskey();
	return result;
}

canonical canonical::restrict(canonical r)
{
	canonical result;
	for (int i = 0; i < terms.size(); i++)
		for (int j = 0; j < r.terms.size(); j++)
			if ((terms[i] & r.terms[j]) != 0)
				result.terms.push_back(terms[i].hide(r.terms[j].vars()));
	result.mccluskey();
	return result;
}

void canonical::extract(smap<int, canonical> *result)
{
	minterm m;
	svector<int> v = vars();
	v.unique();

	for (int i = 0; i < terms.size(); i++)
		m = (i == 0 ? terms[i] : m | terms[i]);

	for (int i = 0; i < v.size(); i++)
		result->insert(pair<int, canonical>(v[i], canonical(m[v[i]])));
}

smap<int, canonical> canonical::extract()
{
	smap<int, canonical> result;
	minterm m;
	svector<int> v = vars();
	v.unique();

	for (int i = 0; i < terms.size(); i++)
		m = (i == 0 ? terms[i] : m | terms[i]);

	for (int i = 0; i < v.size(); i++)
		result.insert(pair<int, canonical>(v[i], canonical(m[v[i]])));
	return result;
}

uint32_t canonical::val(int uid)
{
	uint32_t v = 0;
	for (int i = 0; i < terms.size(); i++)
	{
		if (uid >= terms[i].size)
			terms[i].resize(uid+1, 0xFFFFFFFF);

		v |= terms[i].values[uid>>4];
	}

	return mtoi(v >> vidx(uid));
}

canonical canonical::pabs()
{
	canonical result;
	for (int i = 0; i < terms.size(); i++)
		result.push_back(terms[i].pabs());
	result.mccluskey();
	return result;
}

canonical canonical::nabs()
{
	canonical result;
	for (int i = 0; i < terms.size(); i++)
		result.push_back(terms[i].nabs());
	result.mccluskey();
	return result;
}

int canonical::satcount()
{
	return terms.size();
}

smap<int, uint32_t> canonical::anysat()
{
	return terms[0].anysat();
}

svector<smap<int, uint32_t> > canonical::allsat()
{
	svector<smap<int, uint32_t> > sats;
	for (int i = 0; i < terms.size(); i++)
		sats.push_back(terms[i].anysat());
	return sats;
}

canonical &canonical::operator=(canonical c)
{
	this->terms.clear();
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

canonical &canonical::operator^=(canonical c)
{
	*this = *this ^ c;
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
	for (i = 0; i < terms.size(); i++)
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
	minterm result;
	for (int j = 0; j < terms.size(); j++)
		result.sv_union(i, terms[j].get(i));
	return canonical(result);
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
	for (i = 0; i < terms.size(); i++)
		for (j = 0; j < c.terms.size(); j++)
		{
			result.terms.push_back(terms[i] & c.terms[j]);
			if (result.terms.size() >= 128)
				result.mccluskey();
		}
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

canonical canonical::operator^(canonical c)
{
	return ((*this & ~c) | (~*this & c));
}

canonical canonical::operator&&(canonical c)
{
	canonical result;
	int i, j;
	for (i = 0; i < terms.size(); i++)
		for (j = 0; j < c.terms.size(); j++)
		{
			result.terms.push_back((terms[i] & c.terms[j]).xoutnulls());
			if (result.terms.size() >= 128)
				result.mccluskey();
		}
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
	terms.sort();
	c.terms.sort();
	bool result = (terms.size() == c.terms.size());
	for (int i = 0; i < (int)terms.size() && result; i++)
		result = result && (terms[i] == c.terms[i]);
	return result;
}

bool canonical::operator!=(canonical c)
{
	terms.sort();
	c.terms.sort();
	bool result = (terms.size() == c.terms.size());
	for (int i = 0; i < (int)terms.size() && result; i++)
		result = result && (terms[i] == c.terms[i]);
	return !result;
}

bool canonical::operator==(minterm c)
{
	return (terms.size() == 1 && terms[0] == c);
}

bool canonical::operator!=(minterm c)
{
	return (terms.size() != 1 || terms[0] != c);
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
sstring canonical::print(variable_space *vars, sstring prefix)
{
	if (terms.size() == 0)
		return "0";
	else if (terms.size() == 1 && terms[0] == 1)
		return "1";

	sstring res;
	for (int i = 0; i < (int)terms.size(); i++)
	{
		if (i != 0)
			res += "|";
		res += terms[i].print(vars, prefix);
	}
	return res;
}

sstring canonical::print_assign(variable_space *v, sstring prefix)
{
	string result = "";
	for (int i = 0; i < terms.size(); i++)
	{
		if (i != 0)
			result += ", ";
		result += terms[i].print_assign(v, prefix);
	}
	return result == "" ? "skip" : result;
}

sstring canonical::print_with_quotes(variable_space *vars, sstring prefix)
{
	if (terms.size() == 0)
                return "0";
        else if (terms.size() == 1 && terms[0] == 1)
                return "1";

        sstring res;
        for (int i = 0; i < (int)terms.size(); i++)
        {
                if (i != 0)
                        res += "|";
                res += terms[i].print_with_quotes(vars, prefix);
        }
        return res;
}

bool is_mutex(canonical *c0, canonical *c1)
{
	int i, j;
	for (i = 0; i < (int)c0->terms.size(); i++)
		for (j = 0; j < (int)c1->terms.size(); j++)
			if ((c0->terms[i] & c1->terms[j]) != 0)
				return false;

	return true;
}

bool is_mutex(canonical *c0, canonical *c1, canonical *c2)
{
	int i, j, k;
	for (i = 0; i < (int)c0->terms.size(); i++)
		for (j = 0; j < (int)c1->terms.size(); j++)
			for (k = 0; k < (int)c2->terms.size(); k++)
			if ((c0->terms[i] & c1->terms[j] & c2->terms[k]) != 0)
				return false;

	return true;
}

bool mergible(canonical *c0, canonical *c1)
{
	for (int i = 0; i < c0->terms.size(); i++)
	{
		for (int j = 0; j < c1->terms.size(); j++)
		{
			pair<int, int> xdiff = c0->terms[i].xdiff_count(c1->terms[j]);
			int diff = c0->terms[i].diff_count(c1->terms[j]);
			if ((diff <= 1 && xdiff.first + xdiff.second <= 1) ||
				(xdiff.first == 0 && diff - xdiff.second <= 1) ||
				(xdiff.second == 0 && diff - xdiff.first <= 1))
				return true;
		}
	}
	return false;
}
