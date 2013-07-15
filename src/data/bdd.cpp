/*
 * bdd.cpp
 *
 *  Created on: May 12, 2013
 *      Author: nbingham
 */

#include "bdd.h"
#include "../utility.h"

triple::triple()
{
}

triple::triple(int i, int l, int h)
{
	this->i = i;
	this->l = l;
	this->h = h;
}

triple::~triple()
{
}

triple &triple::operator=(triple t)
{
	this->i = t.i;
	this->l = t.l;
	this->h = t.h;
	return *this;
}

bool operator==(triple t1, triple t2)
{
	return (t1.i == t2.i && t1.l == t2.l && t1.h == t2.h);
}

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

int bdd::mk(int i, int l, int h)
{
	if (i >= T[0].i || i >= T[1].i)
	{
		T[0].i = i+1;
		T[1].i = i+1;
	}

	if (l == h)
		return l;

	unordered_map<triple, int>::iterator j = H.find(triple(i, l, h));
	if (j != H.end())
		return j->second;

	T.push_back(triple(i, l, h));
	return H.insert(pair<triple, int>(triple(i, l, h), T.size()-1)).first->second;
}

int bdd::build(string e, vspace *vars, int i)
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
		cout << "Descend " << t.print_expr() << endl;
		u0 = build(t(i, v0), i+1);
		u1 = build(t(i, v1), i+1);
		cout << "Merge" << endl;
		return mk(i, u0, u1);
	}
}

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

int bdd::smooth(int u, int j)
{
	unordered_map<pair<int, int>, int> G;
	return apply(&bitwise_or, restrict(u, j, 0) , restrict(u, j, 1), &G);
}

int bdd::smooth(int u, vector<int> j)
{
	for (int i = 0; i < (int)j.size(); i++)
		u = smooth(u, j[i]);

	return u;
}

int bdd::smart_smooth(int u, int j, int u1)
{
	if (apply_and(u, u1) != u)
		u = smooth(u, j);

	return u;
}

int bdd::smart_smooth(int u, map<int, int> j)
{
	map<int, int>::iterator ji;
	for (ji = j.begin(); ji != j.end(); ji++)
		u = smart_smooth(u, ji->first, ji->second);

	return u;
}

int bdd::extract(int u, int j)
{
	vector<int> vl;
	variable_list(u, &vl);
	for (int i = 0; i < (int)vl.size(); i++)
		if (vl[i] != j)
			u = smooth(u, vl[i]);
	return u;
}

void bdd::extract(int u, map<int, int> *result)
{
	vector<int> vl;
	int i, j;
	int t;
	map<int, int>::iterator ri;

	variable_list(u, &vl);
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

void bdd::variable_list(int u, vector<int> *l)
{
	if (u > 1)
	{
		l->push_back(var(u));
		variable_list(low(u), l);
		variable_list(high(u), l);
	}
}

int bdd::transition(int u0, int u1)
{
	vector<int> vars;
	unordered_map<pair<int, int>, int> G;

	variable_list(u1, &vars);
	unique(&vars);

	int ret = u0;
	for (int i = 0; i < (int)vars.size(); i++)
		ret = smooth(ret, vars[i]);

	return apply(&bitwise_and, ret, u1, &G);
}

int bdd::count(int u)
{
	return (u < 2 ? u : powi(2, var(low(u)) - var(u) - 1)*count(low(u))+
						powi(2, var(high(u)) - var(u) - 1)*count(high(u)));
}

int bdd::satcount(int u)
{
	return powi(2, var(u))*count(u);
}

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

void bdd::print(int u, string tab)
{
	cout << tab << u << " -> {" << var(u) << ", " << low(u) << ", " << high(u) << "}" << endl;
	if (u > 1)
	{
		print(low(u), tab+"\t");
		print(high(u), tab+"\t");
	}
}

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

int bdd::invert(int u)
{
	return u < 2 ? u : mk(var(u), invert(high(u)), invert(low(u)));
}

int bdd::apply_or(int u0, int u1)
{
	unordered_map<pair<int, int>, int> G;
	return apply(&bitwise_or, u0, u1, &G);
}

int bdd::apply_and(int u0, int u1)
{
	unordered_map<pair<int, int>, int> G;
	return apply(&bitwise_and, u0, u1, &G);
}

int bdd::apply_not(int u1)
{
	unordered_map<int, int> G;
	return apply(&bitwise_not, u1, &G);
}

int bdd::apply_union(int u0, int u1)
{
	return 0;
}

int bdd::apply_intersect(int u0, int u1)
{
	return 0;
}

int bdd::apply_inverse(int u1)
{
	return 0;
}
