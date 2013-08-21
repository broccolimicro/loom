/*
 * bdd_package.cpp
 *
 *  Created on: Jul 31, 2013
 *      Author: nbingham
 */

#include "bdd_package.h"
#include "../utility.h"

bdd_package::bdd_package()
{
	T.push_back(triple(0, 0, 0));
	T.push_back(triple(0, 1, 1));
}
bdd_package::~bdd_package()
{
}

/**
 * \brief	Identifies all variables that are used to represent the expression whose index is u.
 * \param	u	An index into T that represents the expression to analyze.
 * \param	l	The resulting list of variable indices.
 */
void bdd_package::vars(uint32_t u, vector<int> *var_list)
{
	if (u > 1)
	{
		var_list->push_back(T[u].i);
		vars(T[u].l, var_list);
		vars(T[u].h, var_list);
	}
}

/**
 * \brief	Searches the table H for a triple (i,l,h) and returns the match if one exists. Otherwise, it creates a new triple which is then inserted into the table and then returned.
 * \param	i	The variable index of this triple. Can be obtained with variable_space::get_uid(), variable::uid, or bdd_package::var().
 * \param	l	An index into T identifying the lower outgoing edge of the new triple.
 * \param	h	An index into T identifying the higher outgoing edge of the new triple.
 * \return	An index into T that identifies the newly created node.
 * \see		build() and [An Introduction to Binary Decision Diagrams](https://www.itu.dk/courses/AVA/E2005/bdd_package-eap.pdf)
 */
uint32_t bdd_package::mk(int i, uint32_t l, uint32_t h)
{
	if (i >= T[0].i || i >= T[1].i)
	{
		T[0].i = i+1;
		T[1].i = i+1;
	}

	if (l == h)
		return l;

	triple t(i, l, h);
	unordered_map<triple, uint32_t>::iterator j = H.find(t);
	if (j != H.end())
		return j->second;

	T.push_back(t);
	H.insert(pair<triple, int>(t, T.size()-1));
	return T.size()-1;
}

/**
 * \brief	Parses the given string expression and constructs a bdd_package representation of it.
 * \pre		All variable names used in e must be previously defined in vars.
 * \param	e		A string representation of a boolean expression.
 * \param	vars	The database of variables used to parse e.
 * \param	i		The variable index at which to start. For internal use only.
 * \return	An index into T that identifies the top of the bdd_package that represents e.
 * \see		mk() and [An Introduction to Binary Decision Diagrams](https://www.itu.dk/courses/AVA/E2005/bdd_package-eap.pdf)
 */
uint32_t bdd_package::build(string exp, variable_space *V, int i)
{
	if (i == 0)
		exp = demorgan(exp, -1, false);

	uint32_t u0, u1;
	if (exp == "0")
	{
		if (i >= T[0].i || i >= T[1].i)
		{
			T[0].i = i;
			T[1].i = i;
		}
		return 0;
	}
	else if (exp == "1")
	{
		if (i >= T[0].i || i >= T[1].i)
		{
			T[0].i = i;
			T[1].i = i;
		}
		return 1;
	}
	else
	{
		u0 = build(restrict_exp(exp, V->get_name(i), 0), V, i+1);
		u1 = build(restrict_exp(exp, V->get_name(i), 1), V, i+1);
		return mk(i, u0, u1);
	}
}

/**
 * \brief	Constructs a bdd_package representation of t.
 * \param	t		A boolean minterm.
 * \param	i		The variable index at which to start. For internal use only.
 * \return	An index into T that identifies the top of the bdd_package that represents t.
 * \see		mk() and [An Introduction to Binary Decision Diagrams](https://www.itu.dk/courses/AVA/E2005/bdd_package-eap.pdf)
 */
uint32_t bdd_package::build(minterm t)
{
	uint32_t p = 1, v;
	if (t.size-1 >= T[0].i || t.size-1 >= T[1].i)
	{
		T[0].i = t.size-1;
		T[1].i = t.size-1;
	}

	for (int i = t.size-1; i >= 0; i--)
	{
		v = t.get(i);
		if (v != 0xFFFFFFFF && v != 0)
			p = mk(i, (v == 0x555555555)*p, (v == 0xAAAAAAAA)*p);
	}

	return p;
}

/**
 * \brief	Constructs a bdd_package representation of t.
 * \param	t	A list of (variable index, value) pairs that represent a minterm.
 * \return	An index into T that identifies the top of the bdd_package that represents t.
 * \see		mk() and [An Introduction to Binary Decision Diagrams](https://www.itu.dk/courses/AVA/E2005/bdd_package-eap.pdf)
 */
uint32_t bdd_package::build(map<int, uint32_t> t)
{
	map<int, uint32_t>::reverse_iterator it;
	uint32_t p = 1;
	for (it = t.rbegin(); it != t.rend(); it++)
	{
		if (it->first >= T[0].i || it->first >= T[1].i)
		{
			T[0].i = it->first;
			T[1].i = it->first;
		}
		p = mk(it->first, (1-it->second)*p, it->second*p);
	}

	return p;
}

/**
 * \brief	Constructs a bdd_package representation of t.
 * \param	t		A canonical boolean expression.
 * \param	i		The variable index at which to start. For internal use only.
 * \return	An index into T that identifies the top of the bdd_package that represents t.
 * \see		mk() and [An Introduction to Binary Decision Diagrams](https://www.itu.dk/courses/AVA/E2005/bdd_package-eap.pdf)
 */
uint32_t bdd_package::build(canonical t, int i)
{
	if (t == 0)
	{
		if (i >= T[0].i || i >= T[1].i)
		{
			T[0].i = i;
			T[1].i = i;
		}
		return 0;
	}
	else if (t == 1)
	{
		if (i >= T[0].i || i >= T[1].i)
		{
			T[0].i = i;
			T[1].i = i;
		}
		return 1;
	}
	else
		return mk(i, build(t(i, 0), i+1), build(t(i, 1), i+1));
}

/**
 * \brief	Applies the two input binary boolean operator given by op to the two bdd_packages given by u1 and u2 and returns the resulting bdd_package.
 * \param	op		A function pointer to a two input binary boolean operator.
 * \param	u1,u2	Indices into T that represent the two bdd_packages on which to apply the operator.
 * \param	G		A pointer to an empty map used for dynamic programming.
 * \return	An index into T that identifies the top of the bdd_package that represents the result.
 * \see		bitwise_or(), bitwise_and(), and [An Introduction to Binary Decision Diagrams](https://www.itu.dk/courses/AVA/E2005/bdd_package-eap.pdf)
 */
uint32_t bdd_package::apply(uint32_t (*op)(uint32_t, uint32_t), uint32_t u1, uint32_t u2, unordered_map<pair<uint32_t, uint32_t>, uint32_t> *G)
{
	unordered_map<pair<uint32_t, uint32_t>, uint32_t>::iterator g;
	uint32_t u = 0;

	g = G->find(pair<uint32_t, uint32_t>(u1, u2));
	if (g != G->end())
		return g->second;
	else if (u1 < 2 && u2 < 2)
		u = op(u1, u2);
	else if (T[u1].i == T[u2].i)
		u = mk(T[u1].i, apply(op, T[u1].l, T[u2].l, G), apply(op, T[u1].h, T[u2].h, G));
	else if (T[u1].i < T[u2].i)
		u = mk(T[u1].i, apply(op, T[u1].l, u2, G), apply(op, T[u1].h, u2, G));
	else
		u = mk(T[u2].i, apply(op, u1, T[u2].l, G), apply(op, u1, T[u2].h, G));
	G->insert(pair<pair<uint32_t, uint32_t>, uint32_t>(pair<uint32_t, uint32_t>(u1, u2), u));
	return u;
}

/**
 * \brief	Applies the one input binary boolean operator given by op to the bdd_package given by u1 and returns the resulting bdd_package.
 * \param	op		A function pointer to a one input binary boolean operator.
 * \param	u1		An Index into T that represent the bdd_package on which to apply the operator.
 * \param	G		A pointer to an empty map used for dynamic programming.
 * \return	An index into T that identifies the top of the bdd_package that represents the result.
 * \see		bitwise_not() and [An Introduction to Binary Decision Diagrams](https://www.itu.dk/courses/AVA/E2005/bdd_package-eap.pdf)
 */
uint32_t bdd_package::apply(uint32_t (*op)(uint32_t), uint32_t u1, unordered_map<uint32_t, uint32_t> *G)
{
	unordered_map<uint32_t, uint32_t>::iterator g;
	g = G->find(u1);
	if (g != G->end())
		return g->second;
	else if (u1 < 2)
		return op(u1);
	else
		return mk(T[u1].i, apply(op, T[u1].l, G), apply(op, T[u1].h, G));
}



/**
 * \brief	Inverts all of the inputs to the expression.
 * \details	This is not a binary boolean NOT. While this inverts the inputs, the binary boolean
 * 			NOT operation inverts the expression as a whole.
 * \param	u		An Index into T that represent the bdd_package on which to operate.
 * \return	An index into T that identifies the top of the bdd_package that represents the result.
 */
uint32_t bdd_package::invert(uint32_t u)
{
	return u < 2 ? u : mk(T[u].i, invert(T[u].h), invert(T[u].l));
}

/**
 * \brief	Tries to simplify a the bdd_package representation of an expression by removing and merging nodes.
 * \param	d		The domain to check for in u.
 * \param	u		An Index into T that represent the bdd_package on which to operate.
 * \return	An index into T that identifies the top of the bdd_package that represents the simplified version of u.
 * \see		[An Introduction to Binary Decision Diagrams](https://www.itu.dk/courses/AVA/E2005/bdd_package-eap.pdf)
 */
uint32_t bdd_package::simplify(uint32_t d, uint32_t u)
{
	if (d == 0)
		return 0;
	else if (u <= 1)
		return u;
	else if (d == 1)
		return mk(T[u].i, simplify(d, T[u].l), simplify(d, T[u].h));
	else if (T[d].i == T[u].i)
	{
		if (T[d].l == 0)
			return simplify(T[d].h, T[u].h);
		else if (T[d].h == 0)
			return simplify(T[d].l, T[u].l);
		else
			return mk(T[u].i, simplify(T[d].l, T[u].l), simplify(T[d].h, T[u].h));
	}
	else if (T[d].i < T[u].i)
		return mk(T[d].i, simplify(T[d].l, u), simplify(T[d].h, u));
	else
		return mk(T[u].i, simplify(d, T[u].l), simplify(d, T[u].h));
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
uint32_t bdd_package::restrict(uint32_t u, int j, uint32_t b)
{
	if (T[u].i > j)
		return u;
	else if (T[u].i < j)
		return mk(T[u].i, restrict(T[u].l, j, b), restrict(T[u].h, j, b));
	else if (b == 0)
		return restrict(T[u].l, j, b);
	else
		return restrict(T[u].h, j, b);
}




/**
 * \brief	A helper function for satcount().
 */
int bdd_package::count(uint32_t u)
{
	return (u < 2 ? u : powi(2, T[T[u].l].i - T[u].i - 1)*count(T[u].l)+
						powi(2, T[T[u].h].i - T[u].i - 1)*count(T[u].h));
}

/**
 * \brief	Calculates a satisfying state encoding for the expression represented by u.
 * \param	u		An index into T that represent the expression to analyze.
 * \return	A state encoding represented by a list of (variable index, value) pairs.
 * \see		allsat().
 */
map<int, uint32_t> bdd_package::anysat(uint32_t u)
{
	map<int, uint32_t> res;
	map<int, uint32_t> temp;
	if (u == 0)
	{
		cout << "Error: Put something here.\n";
		return res;
	}
	else if (u == 1)
		return res;
	else if (T[u].l == 0)
	{
		res.insert(pair<int, uint32_t>(T[u].i, 1));
		temp = anysat(T[u].h);
		res.insert(temp.begin(), temp.end());
		return res;
	}
	else
	{
		res.insert(pair<int, uint32_t>(T[u].i, 0));
		temp = anysat(T[u].l);
		res.insert(temp.begin(), temp.end());
		return res;
	}
}

/**
 * \brief	Calculates all satisfying state encodings for the expression represented by u.
 * \param	u		An index into T that represent the expression to analyze.
 * \return	A list of state encodings.
 * \see		anysat().
 */
vector<map<int, uint32_t> > bdd_package::allsat(uint32_t u)
{
	vector<map<int, uint32_t> > res;
	vector<map<int, uint32_t> > temp;
	vector<map<int, uint32_t> >::iterator i;
	if (u == 0)
		return res;
	else if (u == 1)
	{
		res.push_back(map<int, uint32_t>());
		return res;
	}
	else
	{
		temp = allsat(T[u].l);
		for (i = temp.begin(); i != temp.end(); i++)
			i->insert(pair<int, uint32_t>(T[u].i, 0));
		res.insert(res.end(), temp.begin(), temp.end());

		temp = allsat(T[u].h);
		for (i = temp.begin(); i != temp.end(); i++)
			i->insert(pair<int, uint32_t>(T[u].i, 1));
		res.insert(res.end(), temp.begin(), temp.end());
		return res;
	}
}

/**
 * \brief	Prints a structural representation of the bdd_package at a given index to stdout.
 * \param	u		An index into T that represent the bdd_package to print.
 * \param	tab		The starting level of indentation.
 */
void bdd_package::print(uint32_t u, string tab)
{
	cout << tab << u << " -> {" << T[u].i << ", " << T[u].l << ", " << T[u].h << "}" << endl;
	if (u > 1)
	{
		print(T[u].l, tab+"\t");
		print(T[u].h, tab+"\t");
	}
}

/**
 * \brief	Prints a binary representation of the bdd_package at a given index to a string.
 * \details	In a given minterm, the variable is either non-inverted (1), inverted (0), unknown (X), or impossible (_).
 * \param	u		An index into T that represent the bdd_package to print.
 * \param	vars	A vector of variable names indexed by variable index.
 */
/*string bdd_package::trace(uint32_t u, vector<string> vars)
{
	list<list<pair<int, int> > > sat = allsat(u);
	list<list<pair<int, int> > >::iterator i;
	list<pair<int, int> >::iterator j;
	string ret, temp;

	if (sat.size() == 0)
		return string(vars.size(), '_');

	for (i = sat.begin(); i != sat.end(); i++)
	{
		if (i != sat.begin())
			ret += "|";

		if (i->size() == 0)
			string(vars.size(), 'X');

		temp = string(vars.size(), 'X');
		for (j = i->begin(); j != i->end(); j++)
		{
			if ((int)temp.size() <= j->first)
				temp.resize(j->first+1, 'X');
			temp[j->first] = (j->second ? '1' : '0');
		}
		ret += temp;
	}

	return ret;
}*/
