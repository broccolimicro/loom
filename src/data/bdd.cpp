/*
 * bdd.cpp
 *
 *  Created on: May 12, 2013
 *      Author: nbingham
 */

#include "bdd.h"
#include "../utility.h"

bdd::bdd()
{
	T.push_back(triple(0, 0, 0));
	T.push_back(triple(0, 1, 1));
}
bdd::~bdd()
{
}

int bdd::var(int u)
{
	return T[u].i;
}

int bdd::low(int u)
{
	return T[u].l;
}

int bdd::high(int u)
{
	return T[u].h;
}

/**
 * \brief	Identifies all variables that are used to represent the expression whose index is u.
 * \param	u	An index into T that represents the expression to analyze.
 * \param	l	The resulting list of variable indices.
 */
void bdd::allvars(int u, vector<int> *l)
{
	if (u > 1)
	{
		l->push_back(var(u));
		allvars(low(u), l);
		allvars(high(u), l);
	}
}

/**
 * \brief	Searches the table H for a triple (i,l,h) and returns the match if one exists. Otherwise, it creates a new triple which is then inserted into the table and then returned.
 * \param	i	The variable index of this triple. Can be obtained with variable_space::get_uid(), variable::uid, or bdd::var().
 * \param	l	An index into T identifying the lower outgoing edge of the new triple.
 * \param	h	An index into T identifying the higher outgoing edge of the new triple.
 * \return	An index into T that identifies the newly created node.
 * \see		build() and [An Introduction to Binary Decision Diagrams](https://www.itu.dk/courses/AVA/E2005/bdd-eap.pdf)
 */
int bdd::mk(int i, int l, int h)
{
	if (i >= T[0].i || i >= T[1].i)
	{
		T[0].i = i+1;
		T[1].i = i+1;
	}

	if (l == h)
		return l;

	triple t(i, l, h);
	unordered_map<triple, int>::iterator j = H.find(t);
	if (j != H.end())
		return j->second;

	if (T.size() == 0xFFFFFFFF)
		for (int x = 0; x < 200; x++)
			cout << "OVERFLOW" << endl;

	T.push_back(t);
	H.insert(pair<triple, int>(t, T.size()-1));
	return T.size()-1;
}

/**
 * \brief	Parses the given string expression and constructs a bdd representation of it.
 * \pre		All variable names used in e must be previously defined in vars.
 * \param	e		A string representation of a boolean expression.
 * \param	vars	The database of variables used to parse e.
 * \param	i		The variable index at which to start. For internal use only.
 * \return	An index into T that identifies the top of the bdd that represents e.
 * \see		mk() and [An Introduction to Binary Decision Diagrams](https://www.itu.dk/courses/AVA/E2005/bdd-eap.pdf)
 */
int bdd::build(string e, variable_space *vars, int i)
{
	if (i == 0)
		e = demorgan(e, -1, false);

	int u0, u1;
	if (e == "0")
	{
		if (i >= T[0].i || i >= T[1].i)
		{
			T[0].i = i;
			T[1].i = i;
		}
		return 0;
	}
	else if (e == "1")
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
		u0 = build(restrict_exp(e, vars->get_name(i), 0), vars, i+1);
		u1 = build(restrict_exp(e, vars->get_name(i), 1), vars, i+1);
		return mk(i, u0, u1);
	}
}

/**
 * \brief	Constructs a bdd representation of t.
 * \param	t		A boolean minterm.
 * \param	i		The variable index at which to start. For internal use only.
 * \return	An index into T that identifies the top of the bdd that represents t.
 * \see		mk() and [An Introduction to Binary Decision Diagrams](https://www.itu.dk/courses/AVA/E2005/bdd-eap.pdf)
 */
int bdd::build(minterm t, int i)
{
	for (; i < t.size && t[i] == vX; i++);

	int u0, u1;
	if (t.always_0())
	{
		if (i >= T[0].i || i >= T[1].i)
		{
			T[0].i = i;
			T[1].i = i;
		}
		return 0;
	}
	else if (t.always_1())
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
		u0 = build(t(i, v0), i+1);
		u1 = build(t(i, v1), i+1);
		return mk(i, u0, u1);
	}
}

/**
 * \brief	Constructs a bdd representation of t.
 * \param	t	A list of (variable index, value) pairs that represent a minterm.
 * \return	An index into T that identifies the top of the bdd that represents t.
 * \see		mk() and [An Introduction to Binary Decision Diagrams](https://www.itu.dk/courses/AVA/E2005/bdd-eap.pdf)
 */
int bdd::build(list<pair<int, int> > t)
{
	t.sort();
	t.reverse();
	list<pair<int, int> >::iterator it;
	int p = 1;
	for (it = t.begin(); it != t.end(); it++)
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
 * \brief	Constructs a bdd representation of t.
 * \param	t		A canonical boolean expression.
 * \param	i		The variable index at which to start. For internal use only.
 * \return	An index into T that identifies the top of the bdd that represents t.
 * \see		mk() and [An Introduction to Binary Decision Diagrams](https://www.itu.dk/courses/AVA/E2005/bdd-eap.pdf)
 */
vector<int> bdd::build(canonical t, int i)
{
	vector<int> u0, u1;
	if (t.always_0())
	{
		if (i >= T[0].i || i >= T[1].i)
		{
			T[0].i = i;
			T[1].i = i;
		}
		return vector<int>(1, 0);
	}
	else if (t.always_1())
	{
		if (i >= T[0].i || i >= T[1].i)
		{
			T[0].i = i;
			T[1].i = i;
		}
		return vector<int>(1, 1);
	}
	else
	{
		u0 = build(t(i, v0), i+1);
		u1 = build(t(i, v1), i+1);
		return vector<int>(1, mk(i, u0.front(), u1.front()));
	}
}

/**
 * \brief	Applies the two input binary boolean operator given by op to the two bdds given by u1 and u2 and returns the resulting bdd.
 * \param	op		A function pointer to a two input binary boolean operator.
 * \param	u1,u2	Indices into T that represent the two bdds on which to apply the operator.
 * \param	G		A pointer to an empty map used for dynamic programming.
 * \return	An index into T that identifies the top of the bdd that represents the result.
 * \see		bitwise_or(), bitwise_and(), and [An Introduction to Binary Decision Diagrams](https://www.itu.dk/courses/AVA/E2005/bdd-eap.pdf)
 */
int bdd::apply(int (*op)(int, int), int u1, int u2, unordered_map<pair<int, int>, int> *G)
{
	unordered_map<pair<int, int>, int>::iterator g;
	int u = 0;

	g = G->find(pair<int, int>(u1, u2));
	if (g != G->end())
		return g->second;
	else if (u1 < 2 && u2 < 2)
		u = op(u1, u2);
	else if (var(u1) == var(u2))
		u = mk(var(u1), apply(op, low(u1), low(u2), G), apply(op, high(u1), high(u2), G));
	else if (var(u1) < var(u2))
		u = mk(var(u1), apply(op, low(u1), u2, G), apply(op, high(u1), u2, G));
	else
		u = mk(var(u2), apply(op, u1, low(u2), G), apply(op, u1, high(u2), G));
	G->insert(pair<pair<int, int>, int>(pair<int, int>(u1, u2), u));
	return u;
}

/**
 * \brief	Applies the one input binary boolean operator given by op to the bdd given by u1 and returns the resulting bdd.
 * \param	op		A function pointer to a one input binary boolean operator.
 * \param	u1		An Index into T that represent the bdd on which to apply the operator.
 * \param	G		A pointer to an empty map used for dynamic programming.
 * \return	An index into T that identifies the top of the bdd that represents the result.
 * \see		bitwise_not() and [An Introduction to Binary Decision Diagrams](https://www.itu.dk/courses/AVA/E2005/bdd-eap.pdf)
 */
int bdd::apply(int (*op)(int), int u1, unordered_map<int, int> *G)
{
	unordered_map<int, int>::iterator g;
	g = G->find(u1);
	if (g != G->end())
		return g->second;
	else if (u1 < 2)
		return op(u1);
	else
		return mk(var(u1), apply(op, low(u1), G), apply(op, high(u1), G));
}

/**
 * \brief	Calculates the binary boolean OR of the two bdds given by u1 and u2 and returns the resulting bdd.
 * \param	u1,u2	Indices into T that represent the two bdds on which to operate.
 * \return	An index into T that identifies the top of the bdd that represents the result.
 * \see		apply()
 */
int bdd::apply_or(int u0, int u1)
{
	unordered_map<pair<int, int>, int> G;
	return apply(&bitwise_or, u0, u1, &G);
}

/**
 * \brief	Calculates the binary boolean AND of the two bdds given by u1 and u2 and returns the resulting bdd.
 * \param	u1,u2	Indices into T that represent the two bdds on which to operate.
 * \return	An index into T that identifies the top of the bdd that represents the result.
 * \see		apply()
 */
int bdd::apply_and(int u0, int u1)
{
	unordered_map<pair<int, int>, int> G;
	return apply(&bitwise_and, u0, u1, &G);
}

/**
 * \brief	Calculates the binary boolean NOT of the bdd given by u1 and returns the resulting bdd.
 * \param	u1		An Index into T that represent the bdd on which to operate.
 * \return	An index into T that identifies the top of the bdd that represents the result.
 * \see		apply()
 */
int bdd::apply_not(int u1)
{
	unordered_map<int, int> G;
	return apply(&bitwise_not, u1, &G);
}

/**
 * \brief	Inverts all of the inputs to the expression.
 * \details	This is not a binary boolean NOT. While this inverts the inputs, the binary boolean
 * 			NOT operation inverts the expression as a whole.
 * \param	u		An Index into T that represent the bdd on which to operate.
 * \return	An index into T that identifies the top of the bdd that represents the result.
 */
int bdd::invert(int u)
{
	return u < 2 ? u : mk(var(u), invert(high(u)), invert(low(u)));
}

/**
 * \brief	Tries to simplify a the bdd representation of an expression by removing and merging nodes.
 * \param	d		The domain to check for in u.
 * \param	u		An Index into T that represent the bdd on which to operate.
 * \return	An index into T that identifies the top of the bdd that represents the simplified version of u.
 * \see		[An Introduction to Binary Decision Diagrams](https://www.itu.dk/courses/AVA/E2005/bdd-eap.pdf)
 */
int bdd::simplify(int d, int u)
{
	if (d == 0)
		return 0;
	else if (u <= 1)
		return u;
	else if (d == 1)
		return mk(var(u), simplify(d, low(u)), simplify(d, high(u)));
	else if (var(d) == var(u))
	{
		if (low(d) == 0)
			return simplify(high(d), high(u));
		else if (high(d) == 0)
			return simplify(low(d), low(u));
		else
			return mk(var(u), simplify(low(d), low(u)), simplify(high(d), high(u)));
	}
	else if (var(d) < var(u))
		return mk(var(d), simplify(low(d), u), simplify(high(d), u));
	else
		return mk(var(u), simplify(d, low(u)), simplify(d, high(u)));
}

/**
 * \brief	Applies the transition whose index is u1 to the state whose index is u0 to generate the next state.
 * \param	u0		An index into T that represents the state.
 * \param	u1		An index into T that respresets the transition.
 * \return	The index of the resulting state.
 */
int bdd::transition(int u0, int u1)
{
	vector<int> vars;
	unordered_map<pair<int, int>, int> G;

	allvars(u1, &vars);
	unique(&vars);

	int ret = u0;
	for (int i = 0; i < (int)vars.size(); i++)
		ret = smooth(ret, vars[i]);

	return apply(&bitwise_and, ret, u1, &G);
}

/**
 * \brief	Smoothes out all inverted variables on a per-minterm basis.
 * \details	Given a binary boolean expression f whose index is u, for every variable x0, x1, ..., xn this calculates fn such that fi = (fi-1(xi = 0) + xi*fi-1(xi = 1)) and f-1 = f.
 * \param	u		An index into T that represent the expression to smooth.
 * \return	An index into T that identifies the top of the bdd that represents the resulting expression.
 * \see		get_neg(), restrict(), and smooth().
 */
int bdd::get_pos(int u)
{
	vector<int> vl;
	int i;

	allvars(u, &vl);
	unique(&vl);
	for (i = 0; i < (int)vl.size(); i++)
		u = apply_or(apply_and(restrict(u, vl[i], 1), mk(vl[i], 0, 1)), restrict(u, vl[i], 0));
	return u;
}

/**
 * \brief	Smoothes out all non-inverted variables on a per-minterm basis.
 * \details	Given a binary boolean expression f whose index is u, for every variable x0, x1, ..., xn this calculates fn such that fi = (~xi*fi-1(xi = 0) + fi-1(xi = 1)) and f-1 = f.
 * \param	u		An index into T that represent the expression to smooth.
 * \return	An index into T that identifies the top of the bdd that represents the resulting expression.
 * \see		get_pos(), restrict(), and smooth().
 */
int bdd::get_neg(int u)
{
	vector<int> vl;
	int i;

	allvars(u, &vl);
	unique(&vl);
	for (i = 0; i < (int)vl.size(); i++)
		u = apply_or(apply_and(restrict(u, vl[i], 0), mk(vl[i], 1, 0)), restrict(u, vl[i], 1));

	return u;
}

/**
 * \brief	Restricts the variable whose index is j to the value b in the expression represented by u.
 * \details Given a binary boolean expression f whose index is u, a variable x whose index is j, and a value b, this calculates f(x = b).
 * \param	u		An Index into T that represent the expression to restrict.
 * \param	j		A variable index that represents the variable being restricted. Can be obtained with variable_space::get_uid(), variable::uid, or bdd::var().
 * \param	b		A value that must be either 0 or 1.
 * \return	An index into T that identifies the top of the bdd that represents the result.
 * \see		[An Introduction to Binary Decision Diagrams](https://www.itu.dk/courses/AVA/E2005/bdd-eap.pdf)
 */
int bdd::restrict(int u, int j, int b)
{
	if (var(u) > j)
		return u;
	else if (var(u) < j)
		return mk(var(u), restrict(low(u), j, b), restrict(high(u), j, b));
	else if (b == 0)
		return restrict(low(u), j, b);
	else
		return restrict(high(u), j, b);
}

/**
 * \brief	Smoothes the variable whose index is j out of the expression represented by u.
 * \details	Given a binary boolean expression f whose index is u and a variable x whose index is j, this calculates f(x = 0) + f(x = 1).
 * \param	u		An index into T that represent the expression to smooth.
 * \param	j		A variable index that represents the variable being smoothed. Can be obtained with variable_space::get_uid(), variable::uid, or bdd::var().
 * \return	An index into T that identifies the top of the bdd that represents the resulting expression.
 * \see		restrict() and [Implicit State Enumeration of Finite State Machines using BDD's](http://pdf.aminer.org/000/283/307/implicit_state_enumeration_of_finite_state_machines_using_bdds.pdf)
 */
int bdd::smooth(int u, int j)
{
	unordered_map<pair<int, int>, int> G;
	return apply(&bitwise_or, restrict(u, j, 0) , restrict(u, j, 1), &G);
}

/**
 * \brief	Smoothes the set of variables whose indices are in j out of the expression represented by u.
 * \param	u		An index into T that represent the expression to smooth.
 * \param	j		A set of variable indices that represent the variables being smoothed. Can be obtained with variable_space::get_uid(), variable::uid, or bdd::var().
 * \return	An index into T that identifies the top of the bdd that represents the resulting expression.
 * \see		smooth() and [Implicit State Enumeration of Finite State Machines using BDD's](http://pdf.aminer.org/000/283/307/implicit_state_enumeration_of_finite_state_machines_using_bdds.pdf)
 */
int bdd::smooth(int u, vector<int> j)
{
	for (int i = 0; i < (int)j.size(); i++)
		u = smooth(u, j[i]);

	return u;
}

/**
 * \brief	Smoothes every variable whose index is not j out of the expression represented by u.
 * \param	u		An index into T that represents the expression to smooth.
 * \param	j		A variable index that represents the one variable not being smoothed. Can be obtained with variable_space::get_uid(), variable::uid, or bdd::var().
 * \return	An index into T that identifies the top of the bdd that represents the resulting expression.
 * \see		smooth().
 */
int bdd::extract(int u, int j)
{
	vector<int> vl;
	allvars(u, &vl);
	for (int i = 0; i < (int)vl.size(); i++)
		if (vl[i] != j)
			u = smooth(u, vl[i]);
	return u;
}

/**
 * \brief	extracts every variable from the expression represented by u into a map that maps variable indices to values.
 * \param	u		An index into T that represents the expression from which to extract all variable's values.
 * \param	result	The resulting map from variable indices to values.
 * \see		smooth() and extract().
 */
void bdd::extract(int u, map<int, int> *result)
{
	vector<int> vl;
	int i, j;
	int t;
	map<int, int>::iterator ri;

	allvars(u, &vl);
	for (i = 0; i < (int)vl.size(); i++)
	{
		t = u;
		for (j = 0; j < (int)vl.size(); j++)
			if (vl[j] != vl[i])
				t = smooth(t, vl[j]);

		ri = result->find(vl[i]);
		if (ri == result->end())
			result->insert(pair<int, int>(vl[i], t));
		else
			ri->second = apply_and(ri->second, t);
	}
}

/**
 * \brief	A helper function for satcount().
 */
int bdd::count(int u)
{
	return (u < 2 ? u : powi(2, var(low(u)) - var(u) - 1)*count(low(u))+
						powi(2, var(high(u)) - var(u) - 1)*count(high(u)));
}

/**
 * \brief	Calculates the number of satisfying truth assignments.
 * \param	u		An index into T that represent the expression to analyze.
 * \return	The number of satisfying truth assignments.
 * \see		allsat().
 */
int bdd::satcount(int u)
{
	return powi(2, var(u))*count(u);
}

/**
 * \brief	Calculates a satisfying state encoding for the expression represented by u.
 * \param	u		An index into T that represent the expression to analyze.
 * \return	A state encoding represented by a list of (variable index, value) pairs.
 * \see		allsat().
 */
list<pair<int, int> > bdd::anysat(int u)
{
	list<pair<int, int> > res;
	if (u == 0)
	{
		cout << "Error: Put something here.\n";
		return res;
	}
	else if (u == 1)
		return res;
	else if (low(u) == 0)
	{
		res.push_back(pair<int, int>(var(u), 1));
		res.splice(res.end(), anysat(high(u)));
		return res;
	}
	else
	{
		res.push_back(pair<int, int>(var(u), 0));
		res.splice(res.end(), anysat(low(u)));
		return res;
	}
}

/**
 * \brief	Calculates all satisfying state encodings for the expression represented by u.
 * \param	u		An index into T that represent the expression to analyze.
 * \return	A list of state encodings.
 * \see		anysat().
 */
list<list<pair<int, int> > > bdd::allsat(int u)
{
	list<list<pair<int, int> > > res;
	list<list<pair<int, int> > > temp;
	list<list<pair<int, int> > >::iterator i;
	if (u == 0)
		return res;
	else if (u == 1)
	{
		res.push_back(list<pair<int, int> >());
		return res;
	}
	else
	{
		temp = allsat(low(u));
		for (i = temp.begin(); i != temp.end(); i++)
			i->push_front(pair<int, int>(var(u), 0));
		res.splice(res.end(), temp);

		temp = allsat(high(u));
		for (i = temp.begin(); i != temp.end(); i++)
			i->push_front(pair<int, int>(var(u), 1));
		res.splice(res.end(), temp);
		return res;
	}
}

/**
 * \brief	Prints a structural representation of the bdd at a given index to stdout.
 * \param	u		An index into T that represent the bdd to print.
 * \param	tab		The starting level of indentation.
 */
void bdd::print(int u, string tab)
{
	cout << tab << u << " -> {" << var(u) << ", " << low(u) << ", " << high(u) << "}" << endl;
	if (u > 1)
	{
		print(low(u), tab+"\t");
		print(high(u), tab+"\t");
	}
}

/**
 * \brief	Prints a canonical boolean representation of the bdd at a given index to a string.
 * \param	u		An index into T that represent the bdd to print.
 * \param	vars	A vector of variable names indexed by variable index.
 */
string bdd::expr(int u, vector<string> vars)
{
	list<list<pair<int, int> > > sat = allsat(u);
	list<list<pair<int, int> > >::iterator i;
	list<pair<int, int> >::iterator j;
	string ret = "";

	if (sat.size() == 0)
		return "0";

	for (i = sat.begin(); i != sat.end(); i++)
	{
		if (i != sat.begin())
			ret += "|";

		if (i->size() == 0)
			return "1";

		for (j = i->begin(); j != i->end(); j++)
		{
			if (j != i->begin())
				ret += "&";
			ret += (j->second ? string("") : string("~")) + vars[j->first];
		}
	}

	return ret;
}

/**
 * \brief	Prints a binary representation of the bdd at a given index to a string.
 * \details	In a given minterm, the variable is either non-inverted (1), inverted (0), unknown (X), or impossible (_).
 * \param	u		An index into T that represent the bdd to print.
 * \param	vars	A vector of variable names indexed by variable index.
 */
string bdd::trace(int u, vector<string> vars)
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
}
