/*
 * bdd.cpp
 *
 *  Created on: May 12, 2013
 *      Author: nbingham
 */

#include "bdd.h"
#include "bdd_package.h"

bdd_package pkg;

bdd::bdd()
{
	this->idx = 0;
}

bdd::bdd(uint32_t val)
{
	this->idx = val;
}

bdd::bdd(int var, uint32_t val)
{
	this->idx = pkg.mk(var, !val, val);
}

bdd::bdd(map<int, uint32_t> vals)
{
	this->idx = pkg.build(vals);
}

bdd::bdd(string exp, variable_space *v)
{
	this->idx = pkg.build(exp, v);
}

bdd::bdd(minterm t)
{
	this->idx = pkg.build(t);
}

bdd::bdd(canonical c)
{
	this->idx = pkg.build(c);
}

bdd::~bdd()
{
	this->idx = 0;
}

bdd bdd::low()
{
	return bdd(pkg.T[idx].l);
}

bdd bdd::high()
{
	return bdd(pkg.T[idx].h);
}

int bdd::var()
{
	return pkg.T[idx].i;
}

vector<int> bdd::vars()
{
	vector<int> ret;
	pkg.vars(idx, &ret);
	unique(&ret);
	return ret;
}

void bdd::vars(vector<int> *var_list)
{
	pkg.vars(idx, var_list);
	unique(var_list);
}

/**
 * \brief	Smoothes the variable whose index is j out of the expression represented by u.
 * \details	Given a binary boolean expression f whose index is u and a variable x whose index is j, this calculates f(x = 0) + f(x = 1).
 * \param	u		An index into T that represent the expression to smooth.
 * \param	j		A variable index that represents the variable being smoothed. Can be obtained with variable_space::get_uid(), variable::uid, or bdd_package::var().
 * \return	An index into T that identifies the top of the bdd_package that represents the resulting expression.
 * \see		restrict() and [Implicit State Enumeration of Finite State Machines using BDD's](http://pdf.aminer.org/000/283/307/implicit_state_enumeration_of_finite_state_machines_using_bdd_packages.pdf)
 */
bdd bdd::smooth(int var)
{
	unordered_map<pair<uint32_t, uint32_t>, uint32_t> G;
	return bdd(pkg.apply(&bitwise_or, pkg.restrict(idx, var, 0), pkg.restrict(idx, var, 1), &G));
}

/**
 * \brief	Smoothes the set of variables whose indices are in j out of the expression represented by u.
 * \param	u		An index into T that represent the expression to smooth.
 * \param	j		A set of variable indices that represent the variables being smoothed. Can be obtained with variable_space::get_uid(), variable::uid, or bdd_package::var().
 * \return	An index into T that identifies the top of the bdd_package that represents the resulting expression.
 * \see		smooth() and [Implicit State Enumeration of Finite State Machines using BDD's](http://pdf.aminer.org/000/283/307/implicit_state_enumeration_of_finite_state_machines_using_bdd_packages.pdf)
 */
bdd bdd::smooth(vector<int> vars)
{
	bdd ret = *this;
	for (int i = 0; i < (int)vars.size(); i++)
		ret = ret.smooth(vars[i]);

	return ret;
}


/**
 * \brief	extracts every variable from the expression represented by u into a map that maps variable indices to values.
 * \param	u		An index into T that represents the expression from which to extract all variable's values.
 * \param	result	The resulting map from variable indices to values.
 * \see		smooth() and extract().
 */
void bdd::extract(map<int, bdd> *result)
{
	vector<int> vl;
	int i;

	vars(&vl);
	for (i = 0; i < (int)vl.size(); i++)
		result->insert(pair<int, bdd>(vl[i], (*this)[vl[i]]));
}

/**
 * \brief	extracts every variable from the expression represented by u into a map that maps variable indices to values.
 * \param	u		An index into T that represents the expression from which to extract all variable's values.
 * \param	result	The resulting map from variable indices to values.
 * \see		smooth() and extract().
 */
map<int, bdd> bdd::extract()
{
	map<int, bdd> result;
	vector<int> vl;
	int i;

	vars(&vl);
	for (i = 0; i < (int)vl.size(); i++)
		result.insert(pair<int, bdd>(vl[i], (*this)[vl[i]]));

	return result;
}

/**
 * \brief	Smoothes out all inverted variables on a per-minterm basis.
 * \details	Given a binary boolean expression f whose index is u, for every variable x0, x1, ..., xn this calculates fn such that fi = (fi-1(xi = 0) + xi*fi-1(xi = 1)) and f-1 = f.
 * \param	u		An index into T that represent the expression to smooth.
 * \return	An index into T that identifies the top of the bdd_package that represents the resulting expression.
 * \see		get_neg(), restrict(), and smooth().
 */
bdd bdd::pabs()
{
	vector<int> vl;
	bdd ret = *this;

	vars(&vl);
	for (int i = 0; i < (int)vl.size(); i++)
		ret = ((ret(vl[i], 1) & bdd(vl[i], 1)) | ret(vl[i], 0));

	return ret;
}

/**
 * \brief	Smoothes out all non-inverted variables on a per-minterm basis.
 * \details	Given a binary boolean expression f whose index is u, for every variable x0, x1, ..., xn this calculates fn such that fi = (~xi*fi-1(xi = 0) + fi-1(xi = 1)) and f-1 = f.
 * \param	u		An index into T that represent the expression to smooth.
 * \return	An index into T that identifies the top of the bdd_package that represents the resulting expression.
 * \see		get_pos(), restrict(), and smooth().
 */
bdd bdd::nabs()
{
	vector<int> vl;
	bdd ret = *this;

	vars(&vl);
	for (int i = 0; i < (int)vl.size(); i++)
		ret = ((ret(vl[i], 0) & bdd(vl[i], 0)) | ret(vl[i], 1));

	return ret;
}

/**
 * \brief	Calculates the number of satisfying truth assignments.
 * \param	u		An index into T that represent the expression to analyze.
 * \return	The number of satisfying truth assignments.
 * \see		allsat().
 */
int bdd::satcount()
{
	return powi(2, pkg.T[idx].i)*pkg.count(idx);
}

map<int, uint32_t> bdd::anysat()
{
	return pkg.anysat(idx);
}

vector<map<int, uint32_t> > bdd::allsat()
{
	return pkg.allsat(idx);
}

bdd &bdd::operator=(bdd b)
{
	this->idx = b.idx;
	return *this;
}

bdd &bdd::operator=(uint32_t b)
{
	this->idx = b;
	return *this;
}

bdd &bdd::operator|=(bdd b)
{
	*this = *this | b;
	return *this;
}

bdd &bdd::operator&=(bdd b)
{
	*this = *this & b;
	return *this;
}

bdd &bdd::operator|=(uint32_t b)
{
	*this = *this | b;
	return *this;
}

bdd &bdd::operator&=(uint32_t b)
{
	*this = *this & b;
	return *this;
}

/**
 * \brief	Restricts the variable whose index is j to the value b in the expression represented by u.
 * \details Given a binary boolean expression f whose index is u, a variable x whose index is j, and a value b, this calculates f(x = b).
 * \param	u		An Index into T that represent the expression to restrict.
 * \param	j		A variable index that represents the variable being restricted. Can be obtained with variable_space::get_uid(), variable::uid, or bdd_package::var().
 * \param	b		A value that must be either 0 or 1.
 * \return	An index into T that identifies the top of the bdd_package that represents the result.
 * \see		[An Introduction to Binary Decision Diagrams](https://www.itu.dk/courses/AVA/E2005/bdd_package-eap.pdf)
 */
bdd bdd::operator()(int var, uint32_t val)
{
	return bdd(pkg.restrict(idx, var, val));
}

/**
 * \brief	Smoothes every variable whose index is not j out of the expression represented by u.
 * \param	u		An index into T that represents the expression to smooth.
 * \param	j		A variable index that represents the one variable not being smoothed. Can be obtained with variable_space::get_uid(), variable::uid, or bdd_package::var().
 * \return	An index into T that identifies the top of the bdd_package that represents the resulting expression.
 * \see		smooth().
 */
bdd bdd::operator[](int var)
{
	vector<int> vl;
	bdd ret = *this;

	pkg.vars(idx, &vl);
	for (int i = 0; i < (int)vl.size(); i++)
		if (vl[i] != var)
			ret = ret.smooth(vl[i]);

	return ret;
}

/**
 * \brief	Calculates the binary boolean OR of the two bdd_packages given by u1 and u2 and returns the resulting bdd_package.
 * \param	u1,u2	Indices into T that represent the two bdd_packages on which to operate.
 * \return	An index into T that identifies the top of the bdd_package that represents the result.
 * \see		apply()
 */
bdd bdd::operator|(bdd b)
{
	unordered_map<pair<uint32_t, uint32_t>, uint32_t> G;
	return bdd(pkg.apply(&bitwise_or, idx, b.idx, &G));
}

/**
 * \brief	Calculates the binary boolean AND of the two bdd_packages given by u1 and u2 and returns the resulting bdd_package.
 * \param	u1,u2	Indices into T that represent the two bdd_packages on which to operate.
 * \return	An index into T that identifies the top of the bdd_package that represents the result.
 * \see		apply()
 */
bdd bdd::operator&(bdd b)
{
	unordered_map<pair<uint32_t, uint32_t>, uint32_t> G;
	return bdd(pkg.apply(&bitwise_and, idx, b.idx, &G));
}

/**
 * \brief	Calculates the binary boolean NOT of the bdd_package given by u1 and returns the resulting bdd_package.
 * \param	u1		An Index into T that represent the bdd_package on which to operate.
 * \return	An index into T that identifies the top of the bdd_package that represents the result.
 * \see		apply()
 */
bdd bdd::operator~()
{
	unordered_map<uint32_t, uint32_t> G;
	return bdd(pkg.apply(&bitwise_not, idx, &G));
}

bdd bdd::operator|(uint32_t b)
{
	unordered_map<pair<uint32_t, uint32_t>, uint32_t> G;
	return bdd(pkg.apply(&bitwise_or, idx, b, &G));
}

bdd bdd::operator&(uint32_t b)
{
	unordered_map<pair<uint32_t, uint32_t>, uint32_t> G;
	return bdd(pkg.apply(&bitwise_and, idx, b, &G));
}

bool bdd::operator==(bdd b)
{
	return (idx == b.idx);
}

bool bdd::operator!=(bdd b)
{
	return (idx != b.idx);
}

bool bdd::operator==(uint32_t b)
{
	return (idx == b);
}

bool bdd::operator!=(uint32_t b)
{
	return (idx != b);
}

bool bdd::constant()
{
	return (idx < 2);
}

/**
 * \brief	Applies the transition whose index is u1 to the state whose index is u0 to generate the next state.
 * \param	u0		An index into T that represents the state.
 * \param	u1		An index into T that respresets the transition.
 * \return	The index of the resulting state.
 */
bdd bdd::operator>>(bdd b)
{
	unordered_map<pair<uint32_t, uint32_t>, uint32_t> G;
	return bdd(pkg.apply(&bitwise_and, smooth(b.vars()).idx, b.idx, &G));
}

/**
 * \brief	Prints a canonical boolean representation of the bdd_package at a given index to a stream.
 * \param	u		An index into T that represent the bdd_package to print.
 * \param	vars	A vector of variable names indexed by variable index.
 */
string bdd::print(variable_space *v)
{
	vector<map<int, uint32_t> > sat = allsat();
	vector<map<int, uint32_t> >::iterator i;
	map<int, uint32_t>::iterator j;
	string ret;

	if (sat.size() == 0)
		ret +="0";

	for (i = sat.begin(); i != sat.end(); i++)
	{
		if (i != sat.begin())
			ret +="|";

		if (i->size() == 0)
			ret +="1";

		for (j = i->begin(); j != i->end(); j++)
		{
			if (j != i->begin())
				ret +="&";
			ret +=(j->second ? string("") : string("~")) + v->get_name(j->first);
		}
	}

	return ret;
}
