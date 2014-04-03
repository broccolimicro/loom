/*
 * petri.cpp
 *
 *  Created on: May 12, 2013
 *      Author: nbingham
 */

#include "petri.h"
#include "variable_space.h"
#include "../syntax/instruction.h"
#include "../syntax/assignment.h"
#include "../syntax/sequential.h"
#include "../syntax/parallel.h"
#include "../syntax/rule_space.h"

petri_index::petri_index()
{
	data = -1;
}

petri_index::petri_index(int idx, bool place)
{
	data = place ? idx : (idx | 0x80000000);
}

petri_index::~petri_index()
{

}

bool petri_index::is_place() const
{
	return data >= 0;
}

bool petri_index::is_trans() const
{
	return data < 0;
}

sstring petri_index::name() const
{
	return (data >= 0 ? "S" : "T") + sstring(data & 0x7FFFFFFF);
}

int petri_index::idx() const
{
	return data & 0x7FFFFFFF;
}

/*petri_index::operator int() const
{
	return (data & 0x7FFFFFFF);
}*/

petri_index &petri_index::operator=(petri_index i)
{
	data = i.data;
	return *this;
}

petri_index &petri_index::operator--()
{
	--data;
	return *this;
}

petri_index &petri_index::operator++()
{
	++data;
	return *this;
}

petri_index &petri_index::operator--(int)
{
	data--;
	return *this;
}

petri_index &petri_index::operator++(int)
{
	data++;
	return *this;
}

bool operator==(petri_index i, petri_index j)
{
	return i.data == j.data;
}

bool operator!=(petri_index i, petri_index j)
{
	return i.data != j.data;
}

bool operator<(petri_index i, int j)
{
	return ((i.data & 0x7FFFFFFF) < j);
}

ostream &operator<<(ostream &os, petri_index i)
{
	os << i.name();
	return os;
}

bool operator>(petri_index i, petri_index j)
{
	return i.data > j.data;
}

bool operator<(petri_index i, petri_index j)
{
	return i.data < j.data;
}

bool operator>=(petri_index i, petri_index j)
{
	return i.data >= j.data;
}

bool operator<=(petri_index i, petri_index j)
{
	return i.data <= j.data;
}

petri_index operator+(petri_index i, int j)
{
	i.data += j;
	return i;
}

petri_index operator-(petri_index i, int j)
{
	i.data -= j;
	return i;
}

petri_node::petri_node()
{
	owner = NULL;
	assumptions = 1;
}

petri_node::petri_node(canonical index, bool active, smap<int, int> pbranch, smap<int, int> cbranch, instruction *owner)
{
	this->index = index;
	this->active = active;
	this->definitely_invacuous = false;
	this->possibly_vacuous = true;
	this->definitely_vacuous = false;
	this->pbranch = pbranch;
	this->cbranch = cbranch;
	this->owner = owner;
	assumptions = 1;
}

petri_node::~petri_node()
{
	owner = NULL;
}

pair<int, int> petri_node::sense_count()
{
	pair<int, int> result(0, 0);
	for (int i = 0; i < index.terms.size(); i++)
	{
		for (int j = 0; j < index.terms[i].size; j++)
		{
			if (index.terms[i].val(j) == 0)
				result.first++;
			else if (index.terms[i].val(j) == 1)
				result.second++;
		}
	}

	return result;
}

petri_net::petri_net()
{
	pbranch_count = 0;
	cbranch_count = 0;
	prs = NULL;
}

petri_net::~petri_net()
{
	prs = NULL;
}

petri_index petri_net::put_place(canonical root, smap<int, int> pbranch, smap<int, int> cbranch, instruction *owner)
{
	S.push_back(petri_node(root, false, pbranch, cbranch, owner));
	return petri_index(S.size()-1, true);
}

petri_index petri_net::put_transition(canonical root, bool active, smap<int, int> pbranch, smap<int, int> cbranch, instruction *owner)
{
	T.push_back(petri_node(root, active, pbranch, cbranch, owner));
	return petri_index(T.size()-1, false);
}

svector<petri_index> petri_net::put_places(svector<canonical> root, smap<int, int> pbranch, smap<int, int> cbranch, instruction *owner)
{
	svector<petri_index> result;
	for (int i = 0; i < root.size(); i++)
		result.push_back(put_place(root[i], pbranch, cbranch, owner));
	return result;
}

svector<petri_index> petri_net::put_transitions(svector<canonical> root, bool active, smap<int, int> pbranch, smap<int, int> cbranch, instruction *owner)
{
	svector<petri_index> result;
	for (int i = 0; i < root.size(); i++)
		result.push_back(put_transition(root[i], active, pbranch, cbranch, owner));
	return result;
}

void petri_net::cut(petri_index node)
{
	for (int m = 0; m < M0.size(); )
	{
		if (M0[m] == node)
			M0.erase(M0.begin() + m);
		else
		{
			if (M0[m].is_place() == node.is_place() && M0[m] > node)
				M0[m]--;
			m++;
		}
	}

	for (svector<petri_arc>::iterator arc = arcs.begin(); arc != arcs.end(); )
	{
		if (arc->first == node || arc->second == node)
			arc = arcs.erase(arc);
		else
		{
			if (arc->first.is_place() == node.is_place() && arc->first > node)
				arc->first--;
			if (arc->second.is_place() == node.is_place() && arc->second > node)
				arc->second--;
			arc++;
		}
	}

	if (node.is_place())
		S.erase(S.begin() + node.data);
	else
		T.erase(T.begin() + (node.data & 0x7FFFFFFF));
}

void petri_net::cut(svector<petri_index> nodes)
{
	for (int i = 0; i < nodes.size(); i++)
	{
		cut(nodes[i]);
		for (int j = i+1; j < nodes.size(); j++)
			if (nodes[j].is_place() == nodes[i].is_place() && nodes[j] > nodes[i])
				nodes[j]--;
	}
}

svector<petri_index> petri_net::connect(svector<petri_index> from, svector<petri_index> to)
{
	for (int i = 0; i < from.size(); i++)
		for (int j = 0; j < to.size(); j++)
			connect(from[i], to[j]);

	return to;
}

petri_index petri_net::connect(svector<petri_index> from, petri_index to)
{
	for (int i = 0; i < from.size(); i++)
		connect(from[i], to);

	return to;
}

svector<petri_index> petri_net::connect(petri_index from, svector<petri_index> to)
{
	for (int j = 0; j < to.size(); j++)
		connect(from, to[j]);

	return to;
}

petri_index petri_net::connect(petri_index from, petri_index to)
{
	if (from.is_place() && to.is_trans())
	{
		arcs.push_back(petri_arc(from, to));
		at(from).active = at(from).active || at(to).active;
	}
	else if (from.is_trans() && to.is_place())
		arcs.push_back(petri_arc(from, to));
	else if (from.is_place() && to.is_place())
		cerr << "Error: Illegal arc {" << from.name() << ", " << to.name() << "}." << endl;
	else if (from.is_trans() && to.is_trans())
		cerr << "Error: Illegal arc {" << from.name() << ", " << to.name() << "}." << endl;

	return to;
}

petri_index petri_net::push_transition(petri_index from, canonical root, bool active, smap<int, int> pbranch, smap<int, int> cbranch, instruction *owner)
{
	return connect(from, put_transition(root, active, pbranch, cbranch, owner));
}

petri_index petri_net::push_transition(svector<petri_index> from, canonical root, bool active, smap<int, int> pbranch, smap<int, int> cbranch, instruction *owner)
{
	return connect(from, put_transition(root, active, pbranch, cbranch, owner));
}

petri_index petri_net::push_transition(petri_index from, smap<int, int> pbranch, smap<int, int> cbranch, instruction *owner)
{
	return connect(from, put_transition(canonical(1), false, pbranch, cbranch, owner));
}

petri_index petri_net::push_transition(svector<petri_index> from, smap<int, int> pbranch, smap<int, int> cbranch, instruction *owner)
{
	return connect(from, put_transition(canonical(1), false, pbranch, cbranch, owner));
}

svector<petri_index> petri_net::push_transitions(petri_index from, svector<canonical> root, bool active, smap<int, int> pbranch, smap<int, int> cbranch, instruction *owner)
{
	return connect(from, put_transitions(root, active, pbranch, cbranch, owner));
}

svector<petri_index> petri_net::push_transitions(svector<petri_index> from, svector<canonical> root, bool active, smap<int, int> pbranch, smap<int, int> cbranch, instruction *owner)
{
	return connect(from, put_transitions(root, active, pbranch, cbranch, owner));
}

petri_index petri_net::push_place(petri_index from, smap<int, int> pbranch, smap<int, int> cbranch, instruction *owner)
{
	return connect(from, put_place(canonical(0), pbranch, cbranch, owner));
}

petri_index petri_net::push_place(svector<petri_index> from, smap<int, int> pbranch, smap<int, int> cbranch, instruction *owner)
{
	return connect(from, put_place(canonical(0), pbranch, cbranch, owner));
}

svector<petri_index> petri_net::push_places(svector<petri_index> from, smap<int, int> pbranch, smap<int, int> cbranch, instruction *owner)
{
	svector<petri_index> result;
	for (int i = 0; i < from.size(); i++)
		result.push_back(connect(from[i], put_place(canonical(0), pbranch, cbranch, owner)));
	return result;
}

void petri_net::pinch_forward(petri_index n)
{
	svector<petri_index> n1 = next(n);
	svector<petri_index> n1p = prev(n1);
	svector<petri_index> from = prev(n), to = next(n1);
	n1.push_back(n);
	n1.runique();

	if (M0.set_intersection(n1).size() > 0)
		M0.merge(from);

	connect(n1p, from);
	connect(from, to);

	cut(n1);
}

void petri_net::pinch_backward(petri_index n)
{
	svector<petri_index> n1 = prev(n);
	svector<petri_index> n1n = next(n1);
	svector<petri_index> from = prev(n1), to = next(n);
	n1.push_back(n);
	n1.runique();

	bool done = false;
	for (int i = 0; !done && i < n1.size(); i++)
		if (M0.find(n1[i]) != M0.end())
		{
			M0.merge(to);
			done = true;
		}

	connect(to, n1n);
	connect(from, to);
	cut(n1);
}

void petri_net::insert(int a, canonical root, bool active)
{
	smap<int, int> pbranch, cbranch;
	instruction *owner;
	petri_index n0, n1;

	if (at(arcs[a].first).pbranch.size() > at(arcs[a].second).pbranch.size() ||
		at(arcs[a].first).cbranch.size() > at(arcs[a].second).cbranch.size())
	{
		pbranch = at(arcs[a].first).pbranch;
		cbranch = at(arcs[a].first).cbranch;
		owner = at(arcs[a].first).owner;
	}
	else
	{
		pbranch = at(arcs[a].second).pbranch;
		cbranch = at(arcs[a].second).cbranch;
		owner = at(arcs[a].second).owner;
	}

	if (arcs[a].first.is_place())
	{
		n0 = put_transition(root, active, pbranch, cbranch, owner);
		n1 = put_place(at(arcs[a].first).index, pbranch, cbranch, owner);
	}
	else
	{
		n0 = put_place(at(arcs[a].second).index, pbranch, cbranch, owner);
		n1 = put_transition(root, active, pbranch, cbranch, owner);
	}

	connect(arcs[a].first, n0);
	connect(n0, n1);
	arcs[a].first = n1;
}

void petri_net::insert_alongside(petri_index from, petri_index to, canonical root, bool active, instruction *owner)
{

}

petri_index petri_net::duplicate(petri_index n)
{
	petri_index result;
	if (n.is_place())
	{
		S.push_back(at(n));
		result = petri_index(S.size()-1, true);
	}
	else
	{
		T.push_back(at(n));
		result = petri_index(T.size()-1, false);
	}

	for (int i = 0; i < arcs.size(); i++)
	{
		if (arcs[i].first == n)
			arcs.push_back(petri_arc(result, arcs[i].second));
		if (arcs[i].second == n)
			arcs.push_back(petri_arc(arcs[i].first, result));
	}

	if (M0.find(n) != M0.end())
		M0.push_back(result);

	return result;
}

svector<petri_index> petri_net::duplicate(svector<petri_index> n)
{
	svector<petri_index> result;
	for (svector<petri_index>::iterator i = n.begin(); i != n.end(); i++)
		result.push_back(duplicate(*i));
	return result;
}

petri_index petri_net::merge(petri_index n0, petri_index n1)
{
	for (svector<petri_arc>::iterator arc = arcs.begin(); arc != arcs.end(); arc++)
	{
		if (arc->second == n1)
			arcs.push_back(petri_arc(arc->first, n0));
		if (arc->first == n1)
			arcs.push_back(petri_arc(n0, arc->second));
	}

	at(n0).pbranch = at(n0).pbranch.set_intersection(at(n1).pbranch);
	at(n0).cbranch = at(n0).cbranch.set_intersection(at(n1).cbranch);
	at(n0).index |= at(n1).index;
	cut(n1);
	return n0;
}

petri_index petri_net::merge(svector<petri_index> n)
{
	petri_index n0 = n.back();
	n.pop_back();

	for (svector<petri_arc>::iterator arc = arcs.begin(); arc != arcs.end(); arc++)
	{
		for (svector<petri_index>::iterator ni = n.begin(); ni != n.end(); ni++)
		{
			if (arc->second == *ni)
				arcs.push_back(petri_arc(arc->first, n0));
			if (arc->first == *ni)
				arcs.push_back(petri_arc(n0, arc->second));
		}
	}

	for (svector<petri_index>::iterator ni = n.begin(); ni != n.end(); ni++)
	{
		at(n0).pbranch = at(n0).pbranch.set_intersection(at(*ni).pbranch);
		at(n0).cbranch = at(n0).cbranch.set_intersection(at(*ni).cbranch);
		at(n0).index |= at(*ni).index;
	}
	cut(n);
	return n0;
}

svector<petri_index> petri_net::next(petri_index n)
{
	svector<petri_index> result;
	for (svector<petri_arc>::iterator arc = arcs.begin(); arc != arcs.end(); arc++)
		if (arc->first == n)
			result.push_back(arc->second);

	return result.rsort();
}

svector<petri_index> petri_net::next(svector<petri_index> n)
{
	svector<petri_index> result;
	for (svector<petri_arc>::iterator arc = arcs.begin(); arc != arcs.end(); arc++)
		for (svector<petri_index>::iterator ni = n.begin(); ni != n.end(); ni++)
			if (arc->first == *ni)
				result.push_back(arc->second);

	return result.rsort();
}

svector<petri_index> petri_net::prev(petri_index n)
{
	svector<petri_index> result;
	for (svector<petri_arc>::iterator arc = arcs.begin(); arc != arcs.end(); arc++)
		if (arc->second == n)
			result.push_back(arc->first);

	return result.rsort();
}

svector<petri_index> petri_net::prev(svector<petri_index> n)
{
	svector<petri_index> result;
	for (svector<petri_arc>::iterator arc = arcs.begin(); arc != arcs.end(); arc++)
		for (svector<petri_index>::iterator ni = n.begin(); ni != n.end(); ni++)
			if (arc->second == *ni)
				result.push_back(arc->first);

	return result.rsort();
}

svector<int> petri_net::outgoing(petri_index n)
{
	svector<int> result;
	for (int a = 0; a < arcs.size(); a++)
		if (arcs[a].first == n)
			result.push_back(a);

	return result;
}

svector<int> petri_net::outgoing(svector<petri_index> n)
{
	svector<int> result;
	for (int a = 0; a < arcs.size(); a++)
		for (svector<petri_index>::iterator ni = n.begin(); ni != n.end(); ni++)
			if (arcs[a].first == *ni)
				result.push_back(a);

	return result;
}

svector<int> petri_net::incoming(petri_index n)
{
	svector<int> result;
	for (int a = 0; a < arcs.size(); a++)
		if (arcs[a].second == n)
			result.push_back(a);

	return result;
}

svector<int> petri_net::incoming(svector<petri_index> n)
{
	svector<int> result;
	for (int a = 0; a < arcs.size(); a++)
		for (svector<petri_index>::iterator ni = n.begin(); ni != n.end(); ni++)
			if (arcs[a].second == *ni)
				result.push_back(a);

	return result;
}

svector<int> petri_net::next_arc(int n)
{
	svector<int> result;
	for (int a = 0; a < arcs.size(); a++)
		if (arcs[a].first == arcs[n].second)
			result.push_back(a);

	return result;
}

svector<int> petri_net::next_arc(svector<int> n)
{
	svector<int> result;
	for (int a = 0; a < arcs.size(); a++)
		for (svector<int>::iterator ni = n.begin(); ni != n.end(); ni++)
			if (arcs[a].first == arcs[*ni].second)
				result.push_back(a);

	return result;
}

svector<int> petri_net::prev_arc(int n)
{
	svector<int> result;
	for (int a = 0; a < arcs.size(); a++)
		if (arcs[a].second == arcs[n].first)
			result.push_back(a);

	return result;
}

svector<int> petri_net::prev_arc(svector<int> n)
{
	svector<int> result;
	for (int a = 0; a < arcs.size(); a++)
		for (svector<int>::iterator ni = n.begin(); ni != n.end(); ni++)
			if (arcs[a].second == arcs[*ni].first)
				result.push_back(a);

	return result;
}

bool petri_net::is_floating(petri_index n)
{
	for (svector<petri_arc>::iterator arc = arcs.begin(); arc != arcs.end(); arc++)
		if (arc->first == n || arc->second == n)
			return false;

	return true;
}

bool petri_net::are_connected(petri_index n0, petri_index n1)
{
	svector<petri_index> cf1, cf2;
	svector<petri_index> ct1, ct2;

	for (svector<petri_arc>::iterator arc = arcs.begin(); arc != arcs.end(); arc++)
	{
		if (arc->first == n0)
			cf2.push_back(arc->second);
		else if (arc->second == n0)
			cf1.push_back(arc->first);
		if (arc->first == n1)
			ct2.push_back(arc->second);
		else if (arc->second == n1)
			ct1.push_back(arc->first);
	}

	cf1.unique();
	cf2.unique();
	ct1.unique();
	ct2.unique();

	for (svector<petri_index>::iterator i = cf2.begin(), j = ct1.begin(); i != cf2.end() && j != ct1.end();)
	{
		if (*i == *j)
			return true;
		else if (*i < *j)
			i++;
		else if (*i > *j)
			j++;
	}

	for (svector<petri_index>::iterator i = cf1.begin(), j = ct2.begin(); i != cf1.end() && j != ct2.end();)
	{
		if (*i == *j)
			return true;
		else if (*i < *j)
			i++;
		else if (*i > *j)
			j++;
	}

	return false;
}

bool petri_net::have_same_source(petri_index n0, petri_index n1)
{
	svector<petri_index> n0in;
	svector<petri_index> n1in;

	for (svector<petri_arc>::iterator arc = arcs.begin(); arc != arcs.end(); arc++)
	{
		if (arc->second == n0)
			n0in.push_back(arc->first);
		if (arc->second == n1)
			n1in.push_back(arc->first);
	}

	n0in.unique();
	n1in.unique();

	bool diff = true;
	if (n0in.size() == n1in.size())
	{
		diff = false;
		for (svector<petri_index>::iterator n0i = n0in.begin(); n0i != n0in.end() && !diff; n0i++)
		{
			diff = true;
			for (svector<petri_index>::iterator n1i = n1in.begin(); n1i != n1in.end() && diff; n1i++)
				if (at(*n0i).index == at(*n1i).index)
					diff = false;
		}
	}

	return !diff;
}

bool petri_net::have_same_dest(petri_index n0, petri_index n1)
{
	svector<petri_index> n0out;
	svector<petri_index> n1out;

	for (svector<petri_arc>::iterator arc = arcs.begin(); arc != arcs.end(); arc++)
	{
		if (arc->first == n0)
			n0out.push_back(arc->second);
		if (arc->first == n1)
			n1out.push_back(arc->second);
	}

	n0out.unique();
	n1out.unique();

	bool diff = true;
	if (n0out.size() == n1out.size())
	{
		diff = false;
		for (svector<petri_index>::iterator n0i = n0out.begin(); n0i != n0out.end() && !diff; n0i++)
		{
			diff = true;
			for (svector<petri_index>::iterator n1i = n1out.begin(); n1i != n1out.end() && diff; n1i++)
				if (at(*n0i).index == at(*n1i).index)
					diff = false;
		}
	}

	return !diff;
}

int petri_net::are_parallel_siblings(petri_index p0, petri_index p1)
{
	petri_node *n0 = &at(p0);
	petri_node *n1 = &at(p1);
	smap<int, int>::iterator bi, bj;
	for (bi = n0->pbranch.begin(); bi != n0->pbranch.end(); bi++)
		for (bj = n1->pbranch.begin(); bj != n1->pbranch.end(); bj++)
			if (bi->first == bj->first && bi->second != bj->second)
				return bi->first;
	return -1;
}

int petri_net::are_conditional_siblings(petri_index p0, petri_index p1)
{
	petri_node *n0 = &at(p0);
	petri_node *n1 = &at(p1);
	smap<int, int>::iterator bi, bj;
	for (bi = n0->cbranch.begin(); bi != n0->cbranch.end(); bi++)
		for (bj = n1->cbranch.begin(); bj != n1->cbranch.end(); bj++)
			if (bi->first == bj->first && bi->second != bj->second)
				return bi->first;
	return -1;
}

petri_node &petri_net::operator[](petri_index i)
{
	return i.is_place() ? S[i.data] : T[i.data & 0x7FFFFFFF];
}

petri_node &petri_net::at(petri_index i)
{
	return i.is_place() ? S[i.data] : T[i.data & 0x7FFFFFFF];
}

pair<int, int> petri_net::closest_input(svector<int> from, svector<int> to)
{
	svector<int> covered(arcs.size(), 0);
	list<pair<int, int> > nodes;
	for (int i = 0; i < to.size(); i++)
		nodes.push_back(pair<int, int>(0, to[i]));

	for (list<pair<int, int> >::iterator n = nodes.begin(); n != nodes.end(); n = nodes.erase(n))
	{
		if (covered[n->second] == 0)
		{
			if (from.find(n->second) != from.end())
				return *n;

			for (int a = 0; a < arcs.size(); a++)
				if (arcs[a].second == arcs[n->second].first)
					nodes.push_back(pair<int, int>(n->first+1, a));
		}

		covered[n->second]++;
	}

	return pair<int, int>(arcs.size(), -1);
}

pair<int, int> petri_net::closest_output(svector<int> from, svector<int> to)
{
	svector<int> covered(arcs.size(), 0);
	list<pair<int, int> > nodes;
	for (int i = 0; i < from.size(); i++)
		nodes.push_back(pair<int, int>(0, from[i]));

	for (list<pair<int, int> >::iterator n = nodes.begin(); n != nodes.end(); n = nodes.erase(n))
	{
		if (covered[n->second] == 0)
		{
			if (to.find(n->second) != to.end())
				return *n;

			for (int a = 0; a < arcs.size(); a++)
				if (arcs[a].first == arcs[n->second].second)
					nodes.push_back(pair<int, int>(n->first+1, a));
		}

		covered[n->second]++;
	}

	return pair<int, int>(arcs.size(), -1);
}

void petri_net::get_paths(svector<int> from, svector<int> to, path_space *result)
{
	for (int i = 0; i < from.size(); i++)
	{
		svector<int> ex = from;
		ex.erase(ex.begin() + i);

		result->paths.push_back(path(arcs.size(), from[i], from[i]));
		list<path>::iterator path = ::prev(result->paths.end());
		while (path != result->paths.end())
		{
			/* If we hit an arc that should be excluded or we
			 * found a loop then we kill this path and move on.
			 */
			if (ex.find(path->to.front()) != ex.end() || path->nodes[path->to.front()] > 0)
				path = result->paths.erase(path);
			/* If we have reached one of the designated ending nodes
			 * then we are done here and we can move on to the next path.
			 */
			else if (to.find(path->to.front()) != to.end())
				path++;
			/* Otherwise we record this location in the path and
			 * increment to the next set of arcs.
			 */
			else
			{
				path->nodes[path->to.front()]++;

				svector<int> n = next_arc(path->to.front());
				for (int j = n.size()-1; j >= 0; j--)
				{
					if (j == 0)
						path->to.front() = n[j];
					else
					{
						result->paths.push_back(*path);
						result->paths.back().to.front() = n[j];
					}
				}
			}
		}
	}

	// Accumulate the resulting paths into the total count.
	result->total.nodes.resize(arcs.size(), 0);
	for (list<path>::iterator path = result->begin(); path != result->end(); path++)
	{
		result->total.from.merge(path->from);
		result->total.to.merge(path->to);
		for (int i = 0; i < arcs.size(); i++)
			result->total.nodes[i] += path->nodes[i];
	}
	result->total.from.unique();
	result->total.to.unique();
}

void petri_net::zero_paths(path_space *paths, petri_index from)
{
	for (int i = 0; i < (int)arcs.size(); i++)
		if (arcs[i].first == from || arcs[i].second == from)
			paths->zero(i);
}

void petri_net::zero_paths(path_space *paths, svector<petri_index> from)
{
	for (int i = 0; i < (int)from.size(); i++)
		for (int j = 0; j < (int)arcs.size(); j++)
			if (arcs[j].first == from[i] || arcs[j].second == from[i])
				paths->zero(j);
}

void petri_net::zero_ins(path_space *paths, petri_index from)
{
	for (int i = 0; i < (int)arcs.size(); i++)
		if (arcs[i].second == from)
			paths->zero(i);
}

void petri_net::zero_ins(path_space *paths, svector<petri_index> from)
{
	for (int i = 0; i < (int)from.size(); i++)
		for (int j = 0; j < (int)arcs.size(); j++)
			if (arcs[j].second == from[i])
				paths->zero(j);
}

void petri_net::zero_outs(path_space *paths, petri_index from)
{
	for (int i = 0; i < (int)arcs.size(); i++)
		if (arcs[i].first == from)
			paths->zero(i);
}

void petri_net::zero_outs(path_space *paths, svector<petri_index> from)
{
	for (int i = 0; i < (int)from.size(); i++)
		for (int j = 0; j < (int)arcs.size(); j++)
			if (arcs[j].first == from[i])
				paths->zero(j);
}

svector<int> petri_net::start_path(int from, svector<int> ex)
{
	svector<int> result, oa;
	smap<int, pair<petri_index, petri_index> >::iterator ci;
	smap<int, int>::iterator bi, bj, bk;
	int j, k;
	bool same;

	result.push_back(from);
	for (bi = at(arcs[from].second).cbranch.begin(); bi != at(arcs[from].second).cbranch.end(); bi++)
	{
		ci = conditional_places.find(bi->first);
		if (ci != conditional_places.end())
		{
			oa = outgoing(ci->second.second);
			for (j = 0; j < (int)oa.size(); j++)
			{
				bk = at(arcs[oa[j]].second).cbranch.find(bi->first);
				same = (bk->second == bi->second);

				for (k = 0; k < (int)ex.size() && !same; k++)
				{
					bj = at(arcs[ex[k]].second).cbranch.find(bi->first);
					if (bj != at(arcs[ex[k]].second).cbranch.end() && bj->second == bk->second)
						same = true;
				}

				if (!same)
					result.push_back(oa[j]);
			}
		}
		else
		{
			cerr << "Error: Internal failure at line " << __LINE__ << " in file " << __FILE__ << "." << endl;
			cout << "Failed to find the conditional branch " << bi->first << "." << endl;
			print_branch_ids(&cout);
		}
	}
	result.unique();

	/*cout << "Start {" << petri_node_name(from) << "} -> {";
	for (j = 0; j < (int)result.size(); j++)
		cout << petri_node_name(result[j]) << " ";
	cout << "}" << endl;*/

	return result;
}

svector<int> petri_net::start_path(svector<int> from, svector<int> ex)
{
	svector<int> result, oa;
	smap<int, pair<petri_index, petri_index> >::iterator ci;
	smap<int, int>::iterator bi, bj, bk;
	int i, j, k;
	bool same = false;

	for (i = 0; i < (int)from.size(); i++)
	{
		result.push_back(from[i]);
		for (bi = (*this)[arcs[from[i]].second].cbranch.begin(); bi != (*this)[arcs[from[i]].second].cbranch.end(); bi++)
		{
			ci = conditional_places.find(bi->first);
			if (ci != conditional_places.end())
			{
				oa = outgoing(ci->second.second);
				for (j = 0; j < (int)oa.size(); j++)
				{
					bk = at(arcs[oa[j]].second).cbranch.find(bi->first);

					same = false;
					for (k = 0; k < (int)from.size() && !same; k++)
					{
						bj = at(arcs[from[k]].second).cbranch.find(bi->first);
						if (bj != at(arcs[from[k]].second).cbranch.end() && bj->second == bk->second)
							same = true;
					}

					for (k = 0; k < (int)ex.size() && !same; k++)
					{
						bj = at(arcs[ex[k]].second).cbranch.find(bi->first);
						if (bj != at(arcs[ex[k]].second).cbranch.end() && bj->second == bk->second)
							same = true;
					}

					if (!same)
						result.push_back(oa[j]);
				}
			}
			else
			{
				cerr << "Error: Internal failure at line " << __LINE__ << " in file " << __FILE__ << "." << endl;
				cout << "Failed to find the conditional branch " << bi->first << "." << endl;
				print_branch_ids(&cout);
			}
		}
	}
	result.unique();

	/*cout << "Start {";
	for (i = 0; i < (int)from.size(); i++)
		cout << petri_node_name(from[i]) << " ";
	cout << "} -> {";

	for (i = 0; i < (int)result.size(); i++)
		cout << petri_node_name(result[i]) << " ";

	cout << "}" << endl;*/

	return result;
}

svector<petri_index> petri_net::end_path(petri_index to, svector<petri_index> ex)
{
	svector<petri_index> result, oa;
	smap<int, pair<petri_index, petri_index> >::iterator ci;
	smap<int, int>::iterator bi, bj, bk;
	int j, k;
	bool same;

	result.push_back(to);
	for (bi = at(to).cbranch.begin(); bi != at(to).cbranch.end(); bi++)
	{
		ci = conditional_places.find(bi->first);
		if (ci != conditional_places.end())
		{
			oa = prev(ci->second.first);
			for (j = 0; j < (int)oa.size(); j++)
			{
				bk = at(oa[j]).cbranch.find(bi->first);
				same = (bk->second == bi->second);

				for (k = 0; k < (int)ex.size() && !same; k++)
				{
					bj = at(ex[k]).cbranch.find(bi->first);
					if (bj != at(ex[k]).cbranch.end() && bj->second == bk->second)
						same = true;
				}

				if (!same)
					result.push_back(oa[j]);
			}
		}
		else
		{
			cerr << "Error: Internal failure at line " << __LINE__ << " in file " << __FILE__ << "." << endl;
			cout << "Failed to find the conditional branch " << bi->first << "." << endl;
			print_branch_ids(&cout);
		}
	}
	result.unique();

	/*cout << "End {" << petri_node_name(to) << "} -> {";
	for (j = 0; j < (int)result.size(); j++)
		cout << petri_node_name(result[j]) << " ";
	cout << "}" << endl;*/

	return result;
}

svector<petri_index> petri_net::end_path(svector<petri_index> to, svector<petri_index> ex)
{
	svector<petri_index> result, oa;
	smap<int, pair<petri_index, petri_index> >::iterator ci;
	smap<int, int>::iterator bi, bj, bk;
	int i, j, k;
	bool same = false;

	for (i = 0; i < (int)to.size(); i++)
	{
		result.push_back(to[i]);
		for (bi = at(to[i]).cbranch.begin(); bi != at(to[i]).cbranch.end(); bi++)
		{
			ci = conditional_places.find(bi->first);
			if (ci != conditional_places.end())
			{
				oa = prev(ci->second.first);
				for (j = 0; j < (int)oa.size(); j++)
				{
					bk = at(oa[j]).cbranch.find(bi->first);

					same = false;
					for (k = 0; k < (int)to.size() && !same; k++)
					{
						bj = (*this)[to[k]].cbranch.find(bi->first);
						if (bj != (*this)[to[k]].cbranch.end() && bj->second == bk->second)
							same = true;
					}

					for (k = 0; k < (int)ex.size() && !same; k++)
					{
						bj = at(ex[k]).cbranch.find(bi->first);
						if (bj != at(ex[k]).cbranch.end() && bj->second == bk->second)
							same = true;
					}

					if (!same)
						result.push_back(oa[j]);
				}
			}
			else
			{
				cerr << "Error: Internal failure at line " << __LINE__ << " in file " << __FILE__ << "." << endl;
				cout << "Failed to find the conditional branch " << bi->first << "." << endl;
				print_branch_ids(&cout);
			}
		}
	}
	result.unique();

	/*cout << "End {";
	for (i = 0; i < (int)to.size(); i++)
		cout << petri_node_name(to[i]) << " ";
	cout << "} -> {";

	for (i = 0; i < (int)result.size(); i++)
		cout << petri_node_name(result[i]) << " ";

	cout << "}" << endl;*/

	return result;
}

void petri_net::expand()
{
	print_dot(&cout, "this");

	svector<petri_state> states;
	svector<petri_index> transitions;
	svector<petri_arc> new_arcs;

	list<pair<petri_index, petri_state> > execs;
	petri_state start(this, M0);
	execs.push_back(pair<petri_index, petri_state>(petri_index(), petri_state(this, M0)));

	/**
	 * Run through all possible executions from the starting index
	 * looking for deadlock.
	 */
	for (list<pair<petri_index, petri_state> >::iterator exec = execs.begin(); exec != execs.end(); exec++)
	{
		//cout << "\tStart Execution" << endl;
		bool done = false;
		while (!done)
		{
			svector<int> places_ready;
			svector<int> transitions_ready;
			/*
			 * Move transitions first so that we can force each
			 * index to land on a place at the same time. This
			 * allows us to check against the covered flags and
			 * signal done.
			 */
			for (int j = 0; j < exec->second.state.size(); j++)
			{
				if (exec->second.state[j].is_trans())
					transitions_ready.push_back(j);
				else
				{
					int total = 0;
					for (int k = j; k < exec->second.state.size(); k++)
						if (next(exec->second.state[k])[0] == next(exec->second.state[j])[0])
							total++;

					if (total == incoming(next(exec->second.state[j])[0]).size())
					{
						for (int k = j+1; k < exec->second.state.size(); )
						{
							if (next(exec->second.state[k])[0] == next(exec->second.state[j])[0])
								exec->second.state.erase(exec->second.state.begin() + k);
							else
								k++;
						}
						places_ready.push_back(j);
					}
				}
			}

			// Check to see if we are done here...
			petri_state s = exec->second;
			s.state.sort();

			if (s.is_state())
			{
				svector<petri_state>::iterator found;
				if ((found = states.find(s)) != states.end())
				{
					if (exec->first.data != -1 && !exec->first.is_place())
						new_arcs.push_back(petri_arc(exec->first, petri_index(found - states.begin(), true)));
					exec->first = petri_index(found - states.begin(), true);
					done = true;
				}
				else
				{
					states.push_back(s);
					if (exec->first.data != -1 && !exec->first.is_place())
						new_arcs.push_back(petri_arc(exec->first, petri_index(states.size()-1, true)));
					exec->first = petri_index(states.size()-1, true);
				}
			}
			else
			{
				bool found = false;
				for (int i = 0; !found && i < s.state.size(); i++)
					if (s.state[i].is_trans())
					{
						transitions.push_back(s.state[i]);
						if (exec->first.data != -1 && !exec->first.is_trans())
							new_arcs.push_back(petri_arc(exec->first, petri_index(transitions.size()-1, false)));
						exec->first = petri_index(transitions.size()-1, false);
						found = true;
					}
			}

			cout << "\t{";
			for (int i = 0; i < s.state.size(); i++)
			{
				if (i != 0)
					cout << " ";
				cout << s.state[i];
			}
			cout << "}" << endl;

			/* If we are not done, handle the next set of movements
			 * duplicating executions or indices when necessary.
			 */
			if (!done && transitions_ready.size() > 0)
			{
				for (int j = 0; j < transitions_ready.size(); j++)
				{
					svector<petri_index> n = next(exec->second.state[transitions_ready[j]]);

					for (int k = n.size()-1; k >= 0; k--)
					{
						if (k > 0)
							exec->second.state.push_back(n[k]);
						else
							exec->second.state[transitions_ready[j]] = n[k];
					}
				}
			}
			else if (!done && places_ready.size() > 0)
			{
				for (int j = places_ready.size()-1; j >= 0; j--)
				{
					list<pair<petri_index, petri_state> >::iterator temp = exec;
					if (j > 0)
					{
						execs.push_back(*exec);
						temp = ::prev(execs.end());
					}

					svector<petri_index> n = next(temp->second.state[places_ready[j]]);

					for (int k = n.size()-1; k >= 0; k--)
					{
						if (k > 0)
						{
							execs.push_back(*temp);
							execs.back().second.state[places_ready[j]] = n[k];
						}
						else
							temp->second.state[places_ready[j]] = n[k];
					}
				}
			}
			else if (!done)
			{
				cerr << "Error: Deadlock Detected" << endl;
				done = true;
			}
		}
	}

	svector<petri_node> new_S;
	svector<petri_node> new_T;

	for (int i = 0; i < states.size(); i++)
	{
		petri_node new_node;
		new_node.index = 1;
		new_node.assumptions = 1;
		new_node.active = false;
		for (int j = 0; j < states[i].state.size(); j++)
		{
			new_node.index &= at(states[i].state[j]).index;
			new_node.assumptions &= at(states[i].state[j]).assumptions;
			new_node.assertions.merge(at(states[i].state[j]).assertions);
			new_node.active = new_node.active || at(states[i].state[j]).active;
		}

		new_S.push_back(new_node);
	}

	for (int i = 0; i < transitions.size(); i++)
		new_T.push_back(at(transitions[i]));

	S.clear();
	T.clear();
	arcs.clear();
	S = new_S;
	T = new_T;
	arcs = new_arcs;
	M0 = svector<petri_index>(1, petri_index(0, true));

	print_dot(&cout, "this");
}

void petri_net::generate_observed()
{
	for (int i = 0; i < S.size(); i++)
		S[i].observed.clear();
	for (int i = 0; i < T.size(); i++)
		T[i].observed.clear();

	svector<pair<petri_index, bool> > counters;
	for (int i = 0; i < M0.size(); i++)
		counters.push_back(pair<petri_index, bool>(M0[i], false));

	bool done = false;
	while (!done)
	{
		for (int i = 0; i < counters.size(); i++)
			cout << counters[i].first << ":" << counters[i].second << " ";
		cout << endl;

		svector<int> movable;
		for (int i = 0; i < counters.size(); i++)
		{
			svector<petri_index> p = prev(counters[i].first);
			int total = 1;
			for (int j = i+1; j < counters.size(); j++)
				if (counters[j].first == counters[i].first)
					total++;

			if (total == p.size())
			{
				for (int j = i+1; j < counters.size(); )
				{
					if (counters[j].first == counters[i].first)
					{
						counters[i].second = counters[i].second && counters[j].second;
						counters.erase(counters.begin() + j);
					}
					else
						j++;
				}

				movable.push_back(i);
			}
		}

		for (int i = 0; i < movable.size(); i++)
		{
			map<petri_state, canonical> old = at(counters[movable[i]].first).observed;
			at(counters[movable[i]].first).observed.clear();
			if (!counters[movable[i]].second && counters[movable[i]].first.is_place())
			{
				petri_state s;
				s.state.push_back(counters[movable[i]].first);
				at(counters[movable[i]].first).observed.insert(pair<petri_state, canonical>(s, canonical()));

				svector<petri_index> inputs = prev(counters[movable[i]].first);
				for (int j = 0; j < inputs.size(); j++)
				{
					for (map<petri_state, canonical>::iterator k = at(inputs[j]).observed.begin(); k != at(inputs[j]).observed.end(); k++)
					{
						map<petri_state, canonical>::iterator l = at(counters[movable[i]].first).observed.find(k->first);
						canonical c = (k->second | ~at(inputs[j]).index);
						if (l != at(counters[movable[i]].first).observed.end())
							l->second &= c;
						else
							at(counters[movable[i]].first).observed.insert(pair<petri_state, canonical>(k->first, c));
					}
				}
			}
			else if (!counters[movable[i]].second && counters[movable[i]].first.is_trans())
			{
				svector<petri_index> inputs = prev(counters[movable[i]].first);
				svector<map<petri_state, canonical> > in_observed;
				svector<petri_state> state_set;
				for (int j = 0; j < inputs.size(); j++)
				{
					in_observed.push_back(at(inputs[j]).observed);
					for (map<petri_state, canonical>::iterator k = at(inputs[j]).observed.begin(); k != at(inputs[j]).observed.end(); k++)
						state_set.push_back(k->first);
				}
				state_set.unique();

				cout << "\tCommon States ";
				for (int j = 0; j < state_set.size(); j++)
				{
					canonical c = 0;
					bool found = true;
					for (int k = 0; found && k < in_observed.size(); k++)
					{
						map<petri_state, canonical>::iterator l = in_observed[k].find(state_set[j]);
						if (l == in_observed[k].end())
							found = false;
						else
							c |= l->second;
					}

					if (found)
					{
						cout << state_set[j] << " ";
						at(counters[movable[i]].first).observed.insert(pair<petri_state, canonical>(state_set[j], c));
						for (int k = 0; k < in_observed.size(); k++)
							in_observed[k].erase(in_observed[k].find(state_set[j]));
					}
				}
				cout << endl;

				svector<map<petri_state, canonical>::iterator> j;
				for (int k = 0; k < in_observed.size(); )
				{
					if (in_observed[k].size() > 0)
					{
						j.push_back(in_observed[k].begin());
						k++;
					}
					else
						in_observed.erase(in_observed.begin() + k);
				}

				while (j.size() > 0 && j.back() != in_observed.back().end())
				{
					petri_state s;
					canonical c = 0;
					cout << "\t";
					for (int k = 0; k < j.size(); k++)
					{
						cout << j[k]->first << " ";
						s.state.merge(j[k]->first.state);
						c |= j[k]->second;
					}
					cout << endl;

					s.state.unique();

					map<petri_state, canonical>::iterator z = at(counters[movable[i]].first).observed.find(s);
					if (z != at(counters[movable[i]].first).observed.end())
						z->second &= c;
					else
						at(counters[movable[i]].first).observed.insert(pair<petri_state, canonical>(s, c));

					j[0]++;
					for (int k = 0; j[k] == in_observed[k].end() && k < in_observed.size()-1; k++)
					{
						j[k] = in_observed[k].begin();
						j[k+1]++;
					}
				}
			}

			if (!counters[movable[i]].second && at(counters[movable[i]].first).observed == old)
				counters[movable[i]].second = true;

			svector<petri_index> n = next(counters[movable[i]].first);
			for (int j = n.size()-1; j >= 0; j--)
			{
				if (j > 0)
					counters.push_back(pair<petri_index, bool>(n[j], counters[movable[i]].second));
				else
					counters[movable[i]].first = n[j];
			}
		}

		done = true;
		for (int i = 0; done && i < counters.size(); i++)
			if (!counters[i].second)
				done = false;
	}

	cout << "Observed" << endl;
	for (int i = 0; i < S.size(); i++)
	{
		cout << petri_index(i, true) << endl;
		for (map<petri_state, canonical>::iterator j = S[i].observed.begin(); j != S[i].observed.end(); j++)
			cout << "\t" << j->first << ": " << j->second.print(vars) << endl;
	}
}

void petri_net::check_assertions()
{
	for (int i = 0; i < (int)S.size(); i++)
	{
		for (int j = 0; j < (int)S[i].assertions.size(); j++)
			if ((S[i].index & S[i].assertions[j]) != 0)
				cerr << "Error: Assertion " << (~S[i].assertions[j]).print(vars) << " fails at state " << i << " with a state encoding of " << S[i].index.print(vars) << "." << endl;

		for (int j = 0; j < (int)vars->requirements.size(); j++)
			if ((S[i].index & vars->requirements[j]) != 0)
				cerr << "Error: Requirement " << (~vars->requirements[j]).print(vars) << " fails at state " << i << " with a state encoding of " << S[i].index.print(vars) << "." << endl;
	}
}

canonical petri_net::base(svector<int> idx)
{
	canonical res(1);
	int i;
	for (i = 0; i < (int)idx.size(); i++)
		res = res & S[idx[i]].index;

	return res;
}



pair<int, int> petri_net::get_input_sense_count(petri_index idx)
{
	pair<int, int> result(0, 0), temp;
	svector<petri_index> input = prev(idx);
	for (int j = 0; j < input.size(); j++)
	{
		temp = at(input[j]).sense_count();
		result.first += temp.first;
		result.second += temp.second;
	}
	return result;
}

pair<int, int> petri_net::get_input_sense_count(svector<petri_index> idx)
{
	pair<int, int> result(0, 0), temp;
	svector<petri_index> input = prev(idx);
	for (int j = 0; j < input.size(); j++)
	{
		temp = at(input[j]).sense_count();
		result.first += temp.first;
		result.second += temp.second;
	}
	return result;
}

petri_index petri_net::get_split_place(petri_index merge_place, svector<bool> *covered)
{
	petri_index i = merge_place;
	svector<petri_index> ot, op, it, ip;
	svector<bool> c;
	bool loop;

	if ((*covered)[i.data])
		return petri_index(-1, false);

	loop = true;
	it = prev(i);
	if ((int)it.size() <= 0)
		return i;
	else if ((int)it.size() == 1)
	{
		ot = next(i);
		for (int j = 0; j < (int)ot.size() && loop; j++)
		{
			op = next(ot[j]);
			for (int k = 0; k < (int)op.size() && loop; k++)
				if (!(*covered)[op[k].data])
					loop = false;
		}
	}

	(*covered)[i.data] = true;

	while (loop)
	{
		it = prev(i);
		if ((int)it.size() <= 0)
			return i;
		else if ((int)it.size() == 1)
		{
			ip = prev(it[0]);
			if (ip.size() == 0)
				return i;
			i = ip[0];

			if ((*covered)[i.data])
				return petri_index(-1, false);
		}
		else
		{
			petri_index k(-1, true);
			int j = ip.size();
			for (int l = 0; l < (int)it.size() && k.data == -1; l++)
			{
				ip = prev(it[l]);
				for (j = 0; j < (int)ip.size() && k.data == -1; j++)
				{
					c = *covered;
					k = get_split_place(ip[j], &c);
				}
			}

			if (k.data == -1)
				return i;
			else
				i = ip[--j];
		}

		(*covered)[i.data] = true;

		loop = true;
		ot = next(i);
		for (int j = 0; j < (int)ot.size() && loop; j++)
		{
			op = next(ot[j]);
			for (int k = 0; k < (int)op.size() && loop; k++)
				if (!(*covered)[op[k].data])
					loop = false;
		}
	}

	return i;
}

void petri_net::add_conflict_pair(smap<petri_index, list<svector<petri_index> > > *c, petri_index i, petri_index j)
{
	smap<petri_index, list<svector<petri_index> > >::iterator ri;
	list<svector<petri_index> >::iterator li;
	svector<list<svector<petri_index> >::iterator > gi;
	svector<petri_index> group;

	ri = c->find(i);
	if (ri != c->end())
	{
		gi.clear();
		for (li = ri->second.begin(); li != ri->second.end(); li++)
			for (int k = 0; k < (int)li->size(); k++)
				if (are_connected(j, (*li)[k]))// || psiblings(j, (*li)[k]) != -1)
				{
					gi.push_back(li);
					k = (int)li->size();
				}

		group = svector<petri_index>(1, j);
		for (int k = 0; k < (int)gi.size(); k++)
		{
			group.insert(group.end(), gi[k]->begin(), gi[k]->end());
			ri->second.erase(gi[k]);
		}
		group.unique();
		ri->second.push_back(group);
	}
	else
		c->insert(pair<petri_index, list<svector<petri_index> > >(i, list<svector<petri_index> >(1, svector<petri_index>(1, j))));
}

void petri_net::gen_conditional_places()
{
	smap<int, pair<petri_index, petri_index> >::iterator ci;
	svector<petri_index> oa, ia;
	svector<int> sibs;

	conditional_places.clear();
	for (petri_index i(0, true); i < (int)S.size(); i++)
	{
		oa = next(i);
		ia = prev(i);

		sibs.clear();
		for (int j = 0; j < (int)oa.size(); j++)
			for (int k = j+1; k < (int)oa.size(); k++)
				sibs.push_back(are_conditional_siblings(oa[j], oa[k]));
		sibs.unique();

		for (int k = 0; k < (int)sibs.size(); k++)
			if (sibs[k] != -1)
			{
				ci = conditional_places.find(sibs[k]);
				if (ci == conditional_places.end())
					conditional_places.insert(pair<int, pair<petri_index, petri_index> >(sibs[k], pair<petri_index, petri_index>(petri_index(-1, false), i)));
				else
					ci->second.second = i;
			}

		sibs.clear();
		for (int j = 0; j < (int)ia.size(); j++)
			for (int k = j+1; k < (int)ia.size(); k++)
				sibs.push_back(are_conditional_siblings(ia[j], ia[k]));
		sibs.unique();

		for (int k = 0; k < (int)sibs.size(); k++)
			if (sibs[k] != -1)
			{
				ci = conditional_places.find(sibs[k]);
				if (ci == conditional_places.end())
					conditional_places.insert(pair<int, pair<petri_index, petri_index> >(sibs[k], pair<petri_index, petri_index>(i, petri_index(-1, false))));
				else
					ci->second.first = i;
			}
	}
}

bool petri_net::are_sibling_guards(petri_index i, petri_index j)
{
	svector<petri_index> ii = prev(i);
	svector<petri_index> ij = prev(j);
	for (int k = 0; k < ii.size(); k++)
	{
		svector<petri_index> iii = prev(ii[k]);
		for (int l = 0; l < ij.size(); l++)
		{
			svector<petri_index> iij = prev(ij[l]);
			for (int m = 0; m < iii.size(); m++)
				for (int n = 0; n < iij.size(); n++)
					if (iii[m] == iij[n] && !at(ii[k]).active && !at(ij[l]).active)
						return true;
		}
	}
	return false;
}

void petri_net::gen_conflicts()
{
	smap<int, list<svector<int> > >::iterator ri;
	list<svector<int> >::iterator li;
	svector<list<svector<int> >::iterator > gi;
	smap<int, int>::iterator bi, bj;
	svector<int> group;
	svector<int> temp;

	conflicts.clear();
	indistinguishable.clear();

	for (petri_index i(0, true); i < S.size(); i++)
	{
		cout << i << "/" << S.size() << endl;
		svector<petri_index> oi = next(i);

		minterm ti = 1;
		for (int j = 0; j < oi.size(); j++)
			if (at(oi[j]).active)
				ti &= at(oi[j]).index.terms[0];

		canonical nti = ~ti;
		canonical si, st;
		for (int j = 0; j < at(i).index.terms.size(); j++)
		{
			si.terms.push_back(at(i).index.terms[j] & ti);
			st.terms.push_back(at(i).index.terms[j] & ti.inverse());
		}

		for (int k = 0, l = 0; k < si.terms.size() && l < at(i).index.terms.size(); l++)
		{
			if (si.terms[k] != 0 && st.terms[l] == 0)
				si.terms.erase(si.terms.begin() + k);
			else
			{
				si.terms[k] = si.terms[k].xoutnulls() | at(i).index.terms[l];
				k++;
			}
		}

		for (petri_index j(0, true); j < S.size(); j++)
		{
			canonical sj = get_effective_place_encoding(j, svector<petri_index>(1, i));

			/* States are indistinguishable if:
			 *  - they are not the same state
			 *  	> i != j
			 *  - the two states do not exist in parallel
			 *  	> are_parallel_siblings(i, j) < 0
			 *  - the two state encodings are not mutually exclusive
			 *    taking into account the inactive firings between them
			 *  	> si & get_effective_place_encoding(j, svector<petri_index>(1, i)) != 0
			 */
			if (i != j && are_parallel_siblings(i, j) < 0 && !is_mutex(&si, &sj))
			{
				svector<petri_index> oj = next(j);
				minterm tj = 1;
				for (int k = 0; k < oj.size(); k++)
					if (at(oj[k]).active)
						tj &= at(oj[k]).index.terms[0];
				canonical ntj = ~tj;
				canonical jtj = (sj >> tj);

				cout << "CONFLICT " << i.name() << "=" << si.print(vars) << "," << ti.print(vars) << " " << j.name() << "=" << sj.print(vars) << "," << tj.print(vars) << endl;

				/* States are conflicting if:
				 *  - they are indistinguishable
				 *  - the conflict is not caused my non-mutually exclusive guards in a conditional
				 *  	> !are_sibling_guards(i, j)
				 *  - the transition which causes the conflict is not a vacuous firing in the other state
				 *  - the transition which causes the conflict would not normally happen anyways as a result of the other state
				 */
				if (!is_mutex(&nti, &at(i).index, &jtj))
				{
					if (!are_sibling_guards(i, j))
						add_conflict_pair(&conflicts, i, j);
					else
						cerr << "Warning: Conditional near S" << i << " and S" << j << " has non mutually exclusive guards." << endl;
				}

				add_conflict_pair(&indistinguishable, i, j);
			}
		}
	}

	max_indistinguishables = 0;
	for (smap<petri_index, list<svector<petri_index> > >::iterator l = indistinguishable.begin(); l != indistinguishable.end(); l++)
		if ((int)l->second.size() > max_indistinguishables)
			max_indistinguishables = l->second.size();
}

void petri_net::gen_bubbleless_conflicts()
{
	smap<petri_index, list<svector<petri_index> > >::iterator ri;
	list<svector<petri_index> >::iterator li;
	svector<list<svector<petri_index> >::iterator > gi;
	smap<int, int>::iterator bi, bj;
	svector<petri_index> group;
	svector<petri_index> oa;
	svector<int> vl;
	svector<int> temp;
	canonical ntp(0), sp1(0);
	canonical ntn(0), sn1(0);
	canonical s2;
	bool parallel;
	bool strict;

	positive_conflicts.clear();
	positive_indistinguishable.clear();
	negative_conflicts.clear();
	negative_indistinguishable.clear();

	gen_conflicts();

	for (petri_index i(0, true); i < S.size(); i++)
	{
		ri = conflicts.find(i);
		oa = next(i);

		// POSITIVE
		vl.clear();
		canonical tp = 1;
		for (int j = 0; j < oa.size(); j++)
			if (at(oa[j]).active)
			{
				at(oa[j]).negative.vars(&vl);
				tp &= at(oa[j]).negative;
			}
		vl.unique();
		sp1 = at(i).positive.hide(vl);

		// NEGATIVE
		vl.clear();
		canonical tn = 1;
		for (int j = 0; j < oa.size(); j++)
			if (at(oa[j]).active)
			{
				at(oa[j]).positive.vars(&vl);
				tn &= at(oa[j]).positive;
			}
		vl.unique();
		sn1 = at(i).negative.hide(vl);

		for (petri_index j(0, true); j < S.size(); j++)
		{
			strict = false;
			if (ri != conflicts.end())
				for (li = ri->second.begin(); li != ri->second.end() && !strict; li++)
					if (li->find(j) != li->end())
						strict = true;

			if (!strict)
			{
				/* A petri_node has to have at least one output transition due to the trim function.
				 * A petri_node can only have one output transition if that transition is active,
				 * otherwise it can have multiple.
				 */
				oa = next(j);

				/* States are conflicting if:
				 *  - they are not the same state
				 *  - one is not in the tail of another (the might as well be here case)
				 *  - the transition which causes the conflict is not a vacuous firing in the other state
				 *  - they are indistinguishable
				 *  - the two states do not exist in parallel
				 */

				s2 = get_effective_place_encoding(j, svector<petri_index>(1, i));

				parallel = (are_parallel_siblings(i, j) >= 0);
				// POSITIVE
				if (i != j && !parallel && !is_mutex(&sp1, &s2))
				{
					// is it a conflicting state? (e.g. not vacuous)
					ntp = ~tp;
					if (at(i).active && (!is_mutex(&ntp, &sp1, &at(j).index) || (oa.size() > 0 && at(oa[0]).active && is_mutex(&tp, &at(oa[0]).index))))
					{
						if (!are_sibling_guards(i, j))
							add_conflict_pair(&positive_conflicts, i, j);
						else
							cerr << "Warning: Conditional near S" << i << " and S" << j << " has non mutually exclusive guards." << endl;
					}

					// it is at least an indistinguishable state at this point
					add_conflict_pair(&positive_indistinguishable, i, j);
				}

				// NEGATIVE
				if (i != j && !parallel && !is_mutex(&sn1, &s2))
				{
					// is it a conflicting state? (e.g. not vacuous)
					ntn = ~tn;
					if (at(i).active && (!is_mutex(&ntn, &sn1, &at(j).index) || (oa.size() > 0 && at(oa[0]).active && is_mutex(&tn, &at(oa[0]).index))))
					{
						if (!are_sibling_guards(i, j))
							add_conflict_pair(&negative_conflicts, i, j);
						else
							cerr << "Warning: Conditional near S" << i << " and S" << j << " has non mutually exclusive guards." << endl;
					}
					// it is at least an indistinguishable state at this point
					add_conflict_pair(&negative_indistinguishable, i, j);
				}
			}
		}
	}

	max_positive_indistinguishables = 0;
	for (smap<petri_index, list<svector<petri_index> > >::iterator l = positive_indistinguishable.begin(); l != positive_indistinguishable.end(); l++)
		if ((int)l->second.size() > max_positive_indistinguishables)
			max_positive_indistinguishables = l->second.size();

	max_negative_indistinguishables = 0;
	for (smap<petri_index, list<svector<petri_index> > >::iterator l = negative_indistinguishable.begin(); l != negative_indistinguishable.end(); l++)
		if ((int)l->second.size() > max_negative_indistinguishables)
			max_negative_indistinguishables = l->second.size();
}

void petri_net::gen_senses()
{
	int i;
	for (i = 0; i < S.size(); i++)
	{
		S[i].positive = S[i].index.pabs();
		S[i].negative = S[i].index.nabs();
	}

	for (i = 0; i < T.size(); i++)
	{
		T[i].positive = T[i].index.pabs();
		T[i].negative = T[i].index.nabs();
	}
}

canonical petri_net::apply_debug(int pc)
{
	return (vars->enforcements & S[pc].assumptions);
}

void petri_net::trim_branch_ids()
{
	smap<int, svector<int> > pbranch_counts;
	smap<int, svector<int> > cbranch_counts;
	smap<int, svector<int> >::iterator bci;
	smap<int, int>::iterator bi;
	svector<smap<int, int>::iterator > remove;
	int i, j;

	for (i = 0; i < pbranch_count; i++)
		pbranch_counts.insert(pair<int, svector<int> >(i, svector<int>()));

	for (i = 0; i < cbranch_count; i++)
		cbranch_counts.insert(pair<int, svector<int> >(i, svector<int>()));

	for (i = 0; i < T.size(); i++)
	{
		for (bi = T[i].pbranch.begin(); bi != T[i].pbranch.end(); bi++)
			pbranch_counts[bi->first].push_back(bi->second);
		for (bi = T[i].cbranch.begin(); bi != T[i].cbranch.end(); bi++)
			cbranch_counts[bi->first].push_back(bi->second);
	}

	for (bci = pbranch_counts.begin(); bci != pbranch_counts.end(); bci++)
		bci->second.unique();

	for (bci = cbranch_counts.begin(); bci != cbranch_counts.end(); bci++)
		bci->second.unique();

	for (i = 0; i < T.size(); i++)
	{
		remove.clear();
		for (bi = T[i].pbranch.begin(); bi != T[i].pbranch.end(); bi++)
			if (pbranch_counts[bi->first].size() <= 1)
				remove.push_back(bi);

		for (j = 0; j < remove.size(); j++)
			T[i].pbranch.erase(remove[j]);

		remove.clear();
		for (bi = T[i].cbranch.begin(); bi != T[i].cbranch.end(); bi++)
			if (cbranch_counts[bi->first].size() <= 1)
				remove.push_back(bi);

		for (j = 0; j < remove.size(); j++)
			T[i].cbranch.erase(remove[j]);
	}

	for (i = 0; i < S.size(); i++)
	{
		remove.clear();
		for (bi = S[i].pbranch.begin(); bi != S[i].pbranch.end(); bi++)
			if (pbranch_counts[bi->first].size() <= 1)
				remove.push_back(bi);

		for (j = 0; j < remove.size(); j++)
			S[i].pbranch.erase(remove[j]);

		remove.clear();
		for (bi = S[i].cbranch.begin(); bi != S[i].cbranch.end(); bi++)
			if (cbranch_counts[bi->first].size() <= 1)
				remove.push_back(bi);

		for (j = 0; j < remove.size(); j++)
			S[i].cbranch.erase(remove[j]);
	}
}

smap<pair<int, int>, pair<bool, bool> > petri_net::gen_isochronics()
{
	svector<pair<int, int> > net;

	svector<canonical> minimal_guards(vars->global.size()*2);
	svector<petri_index> inputs, inputs2, outputs, outputs2;
	svector<int> tvars, ivars;
	petri_index idx;
	for (petri_index i(0, true); i < T.size(); i++)
	{
		if (at(i).active)
		{
			inputs = prev(i);

			for (int j = 0; j < inputs.size(); j++)
			{
				inputs2 = prev(inputs[j]);
				for (int k = 0; k < inputs2.size(); k++)
					if (at(inputs2[k]).index == 1)
					{
						idx = inputs2[k];
						inputs2.erase(inputs2.begin() + k);
						k = 0;
						inputs2.merge(prev(prev(idx)));
						inputs2.unique();
					}

				for (int k = 0; k < inputs2.size(); k++)
					if (!at(inputs2[k]).active)
						for (int l = 0; l < at(i).index.terms.size(); l++)
						{
							tvars = at(i).index.terms[l].vars();
							for (int m = 0; m < at(inputs2[k]).index.terms.size(); m++)
							{
								ivars = at(inputs2[k]).index.terms[m].vars();
								for (int n = 0; n < tvars.size(); n++)
									for (int o = 0; o < ivars.size(); o++)
										net.push_back(pair<int, int>(ivars[o]*2 + at(inputs2[k]).index.terms[m].val(ivars[o]), tvars[n]*2 + at(i).index.terms[l].val(tvars[n])));
							}
						}
			}
		}
	}

	net.unique();

	sstring label;
	cout << "digraph g0" << endl;
	cout << "{" << endl;

	for (smap<sstring, variable>::iterator i = vars->global.begin(); i != vars->global.end(); i++)
	{
		cout << "\tV" << i->second.uid*2 << " [label=\"" << i->second.name << "-\"];" << endl;
		cout << "\tV" << i->second.uid*2+1 << " [label=\"" << i->second.name << "+\"];" << endl;
	}

	for (int i = 0; i < net.size(); i++)
		cout << "\tV" << net[i].first << " -> V" << net[i].second << endl;

	cout << "}" << endl;

	svector<pair<int, int> > removed;
	svector<pair<int, int> >::iterator remarc;
	for (smap<sstring, variable>::iterator i = vars->global.begin(); i != vars->global.end(); i++)
		for (smap<sstring, variable>::iterator j = vars->global.begin(); j != vars->global.end(); j++)
			if (i != j)
			{
				if ((remarc = net.find(pair<int, int>(i->second.uid*2, j->second.uid*2))) != net.end() && net.find(pair<int, int>(i->second.uid*2+1, j->second.uid*2)) != net.end())
				{
					cerr << "Crap: " << i->second.name << " -> " << j->second.name << "-" << endl;
					removed.push_back(*remarc);
					net.erase(remarc);
				}

				if (net.find(pair<int, int>(i->second.uid*2, j->second.uid*2+1)) != net.end() && (remarc = net.find(pair<int, int>(i->second.uid*2+1, j->second.uid*2+1))) != net.end())
				{
					cerr << "Crap: " << i->second.name << " -> " << j->second.name << "+" << endl;
					removed.push_back(*remarc);
					net.erase(remarc);
				}

				if ((remarc = net.find(pair<int, int>(i->second.uid*2, j->second.uid*2))) != net.end() && net.find(pair<int, int>(i->second.uid*2, j->second.uid*2+1)) != net.end())
				{
					cerr << "Crap: " << i->second.name << "- -> " << j->second.name << endl;
					removed.push_back(*remarc);
					net.erase(remarc);
				}

				if (net.find(pair<int, int>(i->second.uid*2+1, j->second.uid*2)) != net.end() && (remarc = net.find(pair<int, int>(i->second.uid*2+1, j->second.uid*2+1))) != net.end())
				{
					cerr << "Crap: " << i->second.name << "+ -> " << j->second.name << endl;
					removed.push_back(*remarc);
					net.erase(remarc);
				}
			}

	cout << "digraph g1" << endl;
	cout << "{" << endl;

	for (smap<sstring, variable>::iterator i = vars->global.begin(); i != vars->global.end(); i++)
	{
		cout << "\tV" << i->second.uid*2 << " [label=\"" << i->second.name << "-\"];" << endl;
		cout << "\tV" << i->second.uid*2+1 << " [label=\"" << i->second.name << "+\"];" << endl;
	}

	for (int i = 0; i < net.size(); i++)
		cout << "\tV" << net[i].first << " -> V" << net[i].second << endl;

	cout << "}" << endl;

	smap<pair<int, int>, pair<bool, bool> > net2;
	smap<pair<int, int>, pair<bool, bool> >::iterator net2iter;
	for (int i = 0; i < net.size(); i++)
	{
		net2iter = net2.find(pair<int, int>(net[i].first/2, net[i].second/2));
		if (net2iter == net2.end())
			net2.insert(pair<pair<int, int>, pair<bool, bool> >(pair<int, int>(net[i].first/2, net[i].second/2), pair<bool, bool>(true, net[i].first%2 == net[i].second%2)));
		else
			net2iter->second.first = false;
	}

	cout << "digraph g2" << endl;
	cout << "{" << endl;

	for (smap<sstring, variable>::iterator i = vars->global.begin(); i != vars->global.end(); i++)
		cout << "\tV" << i->second.uid << " [label=\"" << i->second.name << "\"];" << endl;

	for (net2iter = net2.begin(); net2iter != net2.end(); net2iter++)
		cout << "\tV" << net2iter->first.first << " -> V" << net2iter->first.second << (net2iter->second.first ? " [style=dashed]" : "") << (net2iter->second.second ? " [arrowhead=odotnormal]" : "") << endl;

	cout << "}" << endl;

	/*for (int i = 0; i < removed.size(); i++)
	{
		cout << vars->get_name(petri_node_name(ed[i].first/2) << (removed[i].first%2 == 0 ? "-" : "+") << " -> " << vars->get_name(removed[i].second/2) << (removed[i].second%2 == 0 ? "-" : "+") << "\t{";
		for (int j = 0; j < T.size(); j++)
		{
			if (T[j].index.val(removed[i].second/2) == (uint32_t)removed[i].second%2)
			{
				cout << "T" << j << ", ";
				inputs = input_petri_nodes(trans_id(j));
				for (int k = 0; k < inputs.size(); k++)
				{
					for (int l = 0; l < S[inputs[k]].index.terms.size(); l++)
						S[inputs[k]].index.terms[l].sv_union(removed[i].first/2, (removed[i].first%2 == 0 ? v1 : v0));
				}
			}
		}
		cout << "}" << endl;
	}*/

	return net2;
}


struct place_encoding_execution
{
	place_encoding_execution() {}
	place_encoding_execution(const place_encoding_execution &c)
	{
		good = c.good;
		net = c.net;
		contributions = c.contributions;
		location = c.location;
		covered = c.covered;
		old = c.old;
		value = c.value;
	}
	place_encoding_execution(petri_index start, petri_net *n)
	{
		net = n;
		location = petri_state(n, svector<petri_index>(1, start), true);
		covered.resize(n->S.size() + n->T.size(), false);
		value = 0;
		old = 0;
		good = true;
	}
	~place_encoding_execution() {}

	petri_net *net;
	petri_state location;
	svector<bool> covered;
	canonical old;
	canonical value;
	svector<canonical> contributions;
	bool good;

	place_encoding_execution &operator=(place_encoding_execution c)
	{
		good = c.good;
		net = c.net;
		location = c.location;
		covered = c.covered;
		old = c.old;
		value = c.value;
		contributions = c.contributions;
		return *this;
	}

	bool check_covered(petri_index i)
	{
		if (i.is_trans())
			return covered[i.idx() + net->S.size()];
		else
			return covered[i.idx()];
	}

	void set_covered(petri_index i)
	{
		if (i.is_trans())
			covered[i.idx() + net->S.size()] = true;
		else
			covered[i.idx()] = true;
	}
};

canonical petri_net::get_effective_place_encoding(petri_index place, svector<petri_index> observer)
{
	cout << "Effective Place Encoding from " << place << " to {";
	for (int i = 0; i < observer.size(); i++)
		cout << observer[i] << " ";
	cout << "}" << endl;
	svector<petri_state> execs(1, petri_state(this, svector<petri_index>(1, place), false));
	petri_state s(this, observer, false);
	svector<bool> covered(S.size() + T.size(), false);

	cout << execs[0] << " -> " << s << endl;

	for (int i = 0; i < execs[0].state.size(); )
	{
		if (s.state.find(execs[0].state[i]) != s.state.end())
			execs[0].state.erase(execs[0].state.begin() + i);
		else
			i++;
	}

	canonical encoding = at(place).index;

	for (int i = 0; i < execs.size(); i++)
	{
		bool done = false;
		while (!done)
		{
			execs[i].state.sort();
			cout << execs[i] << endl;

			svector<int> ready_places;
			svector<int> ready_transitions;
			for (int j = 0; j < execs[i].state.size(); j++)
				if (execs[i].state[j].is_trans() && !covered[S.size() + execs[i].state[j].idx()])
					ready_transitions.push_back(j);

			if (ready_transitions.size() == 0)
			{
				for (int j = 0; j < execs[i].state.size(); j++)
				{
					bool found = false;
					for (int k = 0; !found && k < observer.size(); k++)
						if (at(observer[k]).observed.find(execs[i]) != at(observer[k]).observed.end())
							found = true;

					if (execs[i].state[j].is_place() && !covered[execs[i].state[j].idx()] && execs[i].state[j] != place && !found)
					{
						int total = 0;
						for (int k = j; k < execs[i].state.size(); k++)
							if (prev(execs[i].state[k]) == prev(execs[i].state[j]))
								total++;

						if (total == next(prev(execs[i].state[j])).size())
						{
							for (int k = j+1; k < execs[i].state.size(); )
							{
								if (prev(execs[i].state[k]) == prev(execs[i].state[j]))
									execs[i].state.erase(execs[i].state.begin() + k);
								else
									k++;
							}

							ready_places.push_back(j);
						}
					}
				}
			}

			if (ready_transitions.size() > 0)
			{
				for (int j = 0; j < ready_transitions.size(); j++)
				{
					covered[S.size() + execs[i].state[ready_transitions[j]].idx()] = true;

					svector<petri_index> n = prev(execs[i].state[ready_transitions[j]]);
					for (int k = n.size()-1; k >= 0; k--)
					{
						if (k > 0)
							execs[i].state.push_back(n[k]);
						else
							execs[i].state[ready_transitions[j]] = n[k];
					}
				}
			}
			else if (ready_places.size() > 0)
			{
				for (int j = ready_places.size()-1; j >= 0; j--)
				{
					covered[execs[i].state[ready_places[j]].idx()] = true;
					int i1 = i;
					if (j > 0)
					{
						execs.push_back(execs[i]);
						i1 = execs.size()-1;
					}

					svector<petri_index> n = prev(execs[i1].state[ready_places[j]]);
					for (int k = n.size()-1; k >= 0; k--)
					{
						if (k > 0)
						{
							execs.push_back(execs[i1]);
							execs.back().state[ready_places[j]] = n[k];
						}
						else
							execs[i1].state[ready_places[j]] = n[k];
					}
				}
			}
			else
				done = true;
		}
	}

	execs.unique();

	for (int i = 0; i < execs.size(); i++)
	{
		canonical temp = 0;
		bool found = false;
		for (int j = 0; j < observer.size(); j++)
		{
			map<petri_state, canonical>::iterator k = at(observer[j]).observed.find(execs[i]);
			if (k != at(observer[j]).observed.end())
			{
				temp |= k->second;
				found = true;
			}
		}

		if (found)
			encoding &= temp;
	}

	return encoding;
}

canonical petri_net::get_effective_state_encoding(petri_state state, petri_state observer, svector<petri_index> path)
{
	cout << "Effective State Encoding from " << state << " to " << observer << endl;
	svector<pair<petri_index, canonical> > idx;
	for (int i = 0; i < state.state.size(); i++)
		idx.push_back(pair<petri_index, canonical>(state.state[i], canonical(0)));

	canonical encoding = 1;
	for (int i = 0; i < state.state.size(); i++)
		encoding &= at(state.state[i]).index;

	cout << "Start " << encoding.print(vars) << endl;

	canonical result = 0;

	bool bypass = false;
	while (idx.size() > 0)
	{
		svector<int> movable;
		for (int i = 0; i < idx.size(); i++)
		{
			int total = 0;
			for (int j = i; j < idx.size(); j++)
				if (idx[j].first == idx[i].first)
					total++;

			if (bypass || total == prev(idx[i].first).size())
			{
				for (int j = i+1; j < idx.size();)
				{
					if (idx[j].first == idx[i].first)
					{
						if (idx[i].first.is_place())
							idx[i].second &= idx[j].second;
						else if (idx[i].first.is_trans())
							idx[i].second |= idx[j].second;

						idx.erase(idx.begin() + j);
					}
					else
						j++;
				}

				movable.push_back(i);
			}
		}

		for (int i = 0; i < movable.size(); i++)
			cout << "{" << idx[movable[i]].first << ", " << idx[movable[i]].second.print(vars) << "} ";
		cout << endl;

		for (int i = 0; i < movable.size(); i++)
		{
			svector<petri_index> n = next(idx[movable[i]].first);
			for (int j = 0; j < n.size(); )
			{
				if (path.find(n[j]) == path.end())
					n.erase(n.begin() + j);
				else
					j++;
			}

			if (n.size() == 0 || observer.state.find(idx[movable[i]].first) != observer.state.end() || idx[movable[i]].second == 1)
			{
				cout << "Done " << idx[movable[i]].second.print(vars) << endl;
				if (idx[movable[i]].second != 1)
					result |= idx[movable[i]].second;

				for (int j = i+1; j < movable.size(); j++)
					if (movable[j] > movable[i])
						movable[j]--;
				idx.erase(idx.begin() + movable[i]);
			}
			else
			{
				if (idx[movable[i]].first.is_trans())
					idx[movable[i]].second = ::merge(idx[movable[i]].second, ~at(idx[movable[i]].first).index);

				for (int j = n.size()-1; j >= 0; j--)
				{
					if (j > 0)
					{
						idx.push_back(idx[movable[i]]);
						idx.back().first = n[j];
					}
					else
						idx[movable[i]].first = n[j];
				}
			}
		}

		bypass = false;
		if (movable.size() == 0)
			bypass = true;
	}

	if (result != 0)
		return (encoding & result);
	else
		return encoding;
}

void petri_net::compact()
{
	cout << "Compacting" << endl;
	bool change = true;
	while (change)
	{
		change = false;
		for (petri_index i(0, true); i < S.size();)
		{
			svector<int> ia = incoming(i);
			svector<petri_index> oa = next(i);

			if (ia.size() == 0)
			{
				if (M0.find(i) != M0.end())
				{
					bool all_active = true;
					for (int j = 0; all_active && j < oa.size(); j++)
						if (!at(oa[j]).active || is_mutex(vars->reset, ~at(oa[j]).index))
							all_active = false;

					if (all_active)
					{
						cout << "No input edges" << endl;
						cout << "Reset {";
						for (int i = 0; i < M0.size(); i++)
							cout << M0[i] << " ";
						M0.merge(oa);
						cut(i);
						cout << "} -> {";
						for (int i = 0; i < M0.size(); i++)
							cout << M0[i] << " ";
						cout << "}" << endl;
						change = true;
					}
					else
						i++;
				}
				else
				{
					cout << "No input edges" << endl;
					cout << "Reset {";
					for (int i = 0; i < M0.size(); i++)
						cout << M0[i] << " ";
					cut(i);
					cout << "} -> {";
					for (int i = 0; i < M0.size(); i++)
						cout << M0[i] << " ";
					cout << "}" << endl;
					change = true;
				}
			}
			else
				i++;
		}

		for (petri_index i(0, false); i < T.size();)
		{
			svector<petri_index> n = next(i), p = prev(i), np = prev(n), pn = next(p), nn = next(n), pp = prev(p);

			if (at(i).index == 0 || p.size() == 0)
			{
				cout << "No input edges" << endl;
				cout << "Reset {";
				for (int i = 0; i < M0.size(); i++)
					cout << M0[i] << " ";
				if (M0.find(i) != M0.end())
				{
					M0.merge(n);
					if (at(i).index != 0)
						vars->reset = vars->reset >> at(i).index;
				}

				cut(i);
				cout << "} -> {";
				for (int i = 0; i < M0.size(); i++)
					cout << M0[i] << " ";
				cout << "}" << endl;
				change = true;
			}
			/*else if (at(i).index == 1 && (nn.size() == 1 || p.size() == 1) && n.size() == 1 && np.size() == 1)
			{
				pinch_forward(i);
				change = true;
			}*/
			else if (at(i).index == 1 && (pp.size() == 1 || n.size() == 1) && p.size() == 1 && pn.size() == 1)
			{
				cout << "Pinch" << endl;
				cout << "Reset {";
				for (int i = 0; i < M0.size(); i++)
					cout << M0[i] << " ";
				pinch_backward(i);
				cout << "} -> {";
				for (int i = 0; i < M0.size(); i++)
					cout << M0[i] << " ";
				cout << "}" << endl;
				change = true;
			}
			else
				i++;
		}

		for (petri_index i(0, true); i < S.size(); )
		{
			bool vacuous = true;
			for (petri_index j(0, false); j < T.size() && vacuous; j++)
			{
				vacuous = false;
				for (smap<int, int>::iterator bi = at(i).pbranch.begin(); bi != at(i).pbranch.end() && !vacuous; bi++)
					if (find(at(j).pbranch.begin(), at(j).pbranch.end(), *bi) == at(j).pbranch.end())
						vacuous = true;
			}

			if (vacuous)
			{
				cout << "Vacuous" << endl;
				cout << "Reset {";
				for (int i = 0; i < M0.size(); i++)
					cout << M0[i] << " ";
				if (M0.find(i) != M0.end())
					M0.merge(next(i));
				cut(i);
				cout << "} -> {";
				for (int i = 0; i < M0.size(); i++)
					cout << M0[i] << " ";
				cout << "}" << endl;
				change = true;
			}
			else
				i++;
		}

		for (petri_index i(0, false); i < T.size(); i++)
			for (petri_index j = i+1; j < T.size(); )
			{
				if (!at(i).active && !at(j).active && prev(i) == prev(j) && next(i) == next(j))
				{
					cout << "parallel guards" << endl;
					cout << "Reset {";
					for (int i = 0; i < M0.size(); i++)
						cout << M0[i] << " ";
					at(i).index |= at(j).index;
					if (M0.find(j) != M0.end())
						M0.push_back(i);
					cut(j);
					cout << "} -> {";
					for (int i = 0; i < M0.size(); i++)
						cout << M0[i] << " ";
					cout << "}" << endl;
					change = true;
				}
				else
					j++;
			}
	}
}

bool petri_net::trim()
{
	bool result = false;
	smap<int, int>::iterator bi;
	svector<pair<int, int> >::iterator ai;
	bool vacuous;

	// Remove unreachable places
	/**
	 * A place is "unreachable" when there is no possible state encoding
	 * that would enable an input transition.
	 */
	for (petri_index i(0, true); i < S.size();)
	{
		if (at(i).index == 0)
		{
			cut(i);
			result = true;
		}
		else
			i++;
	}

	for (petri_index i(0, false); i < T.size();)
	{
		svector<petri_index> ia = prev(i);
		svector<petri_index> oa = next(i);

		canonical v = 1;
		for (int j = 0; j < ia.size(); j++)
			v &= at(ia[j]).index;

		// Impossible and dangling transitions
		/**
		 * A transition is "impossible" when it can never fire, regardless of the state
		 * encodings at it's input places. Logically, you would think that this
		 * never should happen in the first place, but this can happen during the
		 * compilation of infinite loops. In an infinite loop, you will always
		 * continue back to the start of a loop, except what if the circuit designer
		 * placed an instruction after an infinite loop? It is easier for us to just
		 * place an impossible transition at the exit of the infinite loop, and then
		 * concatenate other instructions on there.
		 *
		 * A transition is "dangling" when it either has no input arcs or has
		 * no output arcs. This can happen after removing an unreachable state.
		 * Doing so makes it's neighbors dangle.
		 */
		if (at(i).index == 0 || ia.size() == 0 || oa.size() == 0)
		{
			cut(i);
			result = true;
		}

		// Vacuous Transitions
		/**
		 * A transition is "vacuous" when it does nothing to change the state of a
		 * system. We can remove all but two cases: First, if it has more than one
		 * input arc and more than one output arc, it actually makes the system more
		 * complex overall to remove that transition because you would have to merge
		 * every input place with every output place. If there were n input places
		 * and m output places, the number of places in the system would increase
		 * by (n*m - n - m). There is also a case we can't handle where there is
		 * only one output arc, but the place at the end of that arc is a conditional
		 * merge. The reason we can't handle this is because it would break the structure
		 * of the petri-net. You essentially have a parallel merge right before a
		 * conditional merge, and removing the transition that acts as the parallel
		 * merge would break the parallel block.
		 *
		 * The problem with this "vacuous" check is that while it may be locally vacuous,
		 * it can be globally invacuous. For example, If you have *[S1; x-; S2] where
		 * S1 and S2 don't modify the value of x, then x will always be 0 and the x-
		 * transition will be locally vacuous. However, if you then remove the x- transition
		 * and have *[S1;S2], you no longer know the value of x. Therefore, the x- transition
		 * is globally invacuous.
		 */
		else if (at(i).definitely_vacuous && ia.size() == 1 && oa.size() == 1 && next(ia[0]).size() == 1)
		{
			for (int l = 0; l < arcs.size(); l++)
				if (arcs[l].second == ia[0])
					arcs[l].second = oa[0];

			at(oa[0]).assumptions = at(ia[0]).assumptions >> at(oa[0]).assumptions;
			at(oa[0]).assertions.insert(at(oa[0]).assertions.end(), at(ia[0]).assertions.begin(), at(ia[0]).assertions.end());

			cut(ia[0]);
			cut(i);
			result = true;
		}
		else if (at(i).definitely_vacuous && ia.size() == 1 && oa.size() == 1 && prev(oa[0]).size() == 1)
		{
			for (int l = 0; l < arcs.size(); l++)
				if (arcs[l].first == oa[0])
					arcs[l].first = ia[0];

			at(ia[0]).assumptions = at(ia[0]).assumptions >> at(oa[0]).assumptions;
			at(ia[0]).assertions.insert(at(ia[0]).assertions.end(), at(oa[0]).assertions.begin(), at(oa[0]).assertions.end());

			cut(oa[0]);
			cut(i);
			result = true;
		}
		/*else if (T[index(i)].definitely_vacuous && oa.size() == 1 && input_petri_nodes(oa[0]).size() == 1 && (output_petri_nodes(oa[0]).size() == 1 || input_petri_nodes(trans_id(i)).size() == 1))
		{
			for (j = 0; j < ia.size(); j++)
			{
				for (l = 0; l < Wn[oa[0]].size(); l++)
				{
					Wn[ia[j]][l] = (Wn[ia[j]][l] || Wn[oa[0]][l]);
					Wp[ia[j]][l] = (Wp[ia[j]][l] || Wp[oa[0]][l]);
				}

				S[ia[j]].assumptions = S[ia[j]].assumptions >> S[oa[0]].assumptions;
				S[ia[j]].assertions.insert(S[ia[j]].assertions.end(), S[oa[0]].assertions.begin(), S[oa[0]].assertions.end());
			}

			remove_place(oa[0]);

			Wp.remr(i);
			Wn.remr(i);
			T.erase(T.begin() + i);
			result = true;
		}*/
		// All other transitions are fine
		else
			i++;
	}

	// Vacuous pbranches

	/**
	 * Whenever a branch in a parallel block has no transitions, it is a "vacuous branch".
	 * It is equivalent to saying that "nothing" is happening in parallel with "something".
	 * This can happen after the removal of vacuous transitions in a parallel block.
	 */
	for (petri_index i(0, true); i < S.size(); )
	{
		vacuous = true;
		for (petri_index j(0, false); j < T.size() && vacuous; j++)
		{
			vacuous = false;
			for (bi = at(i).pbranch.begin(); bi != at(i).pbranch.end() && !vacuous; bi++)
				if (find(at(j).pbranch.begin(), at(j).pbranch.end(), *bi) == at(j).pbranch.end())
					vacuous = true;
		}

		if (vacuous)
		{
			cut(i);
			result = true;
		}
		else
			i++;
	}

	// Merge Or
	for (petri_index i(0, false); i < T.size(); i++)
		for (petri_index j = i+1; j < T.size(); )
		{
			if (!at(i).active && !at(j).active && prev(i) == prev(j) && next(i) == next(j))
			{
				at(i).index |= at(j).index;
				cut(j);
			}
			else
				j++;
		}

	// Check for petri_nodes with no input arcs
	for (int i = 0; i < M0.size(); i++)
	{
		svector<int> ia = incoming(M0[i]);

		if (ia.size() == 0)
		{
			svector<petri_index> oa = next(M0[i]);

			bool all_active = true;
			for (int j = 0; all_active && j < oa.size(); j++)
				if (!at(oa[j]).active || is_mutex(vars->reset, ~at(oa[j]).index))
					all_active = false;


			if (all_active)
			{
				cut(M0[i]);
				for (int j = 0; j < oa.size(); j++)
				{
					vars->reset = vars->reset >> at(oa[j]).index;
					cut(oa[j]);
				}
			}
		}
	}

	/* Despite our best efforts to fix up the pbranch id's on the fly, we still
	 * need to clean up.
	 */
	trim_branch_ids();

	return result;
}

void petri_net::merge_conflicts()
{
	svector<petri_index>				remove;
	svector<petri_index>				ia, oa;
	svector<int>				vli, vlji, vljo;
	svector<svector<petri_index> >	groups(S.size());
	svector<pair<canonical, canonical> > merge_value(S.size());	// the aggregated state encoding of input places and aggregated state encoding of output places for each group
	svector<pair<canonical, canonical> >::iterator ai;
	smap<int, int>			pbranch, cbranch;
	int						j, k;
	bool					conflict;

	/**
	 * Identify the groups of indistinguishable states and keep track of their aggregate
	 * state encoding. Aggregated state encodings the of input and output places within a
	 * group must be tracked separately because the aggregate state encoding of input
	 * places is the binary AND of all of the individual state encodings and the aggregate
	 * state encoding of the output places is the binary OR of all of the individual state
	 * encodings. Finally, the two aggregate values are combined with a binary AND. The
	 * reason for this is that if you have two transitions that have the same input
	 * place, the state encoding for that input place must allow for either transition
	 * to fire at any time. That means that there cannot exist a state encoding within
	 * the input place that prevents one of the transitions from firing, hence binary AND
	 * (intersection). Also, if there are two transitions with the same output places,
	 * then either transition can fire and place a token in that place, hence binary OR
	 * (union).
	 */
	for (int i = 0; i < S.size(); i++)
	{
		groups[i].push_back(petri_index(i, true));
		if (prev(i).size() == 0)
		{
			merge_value[i].first = S[i].index;
			merge_value[i].second = 0;
		}
		else if (next(i).size() == 0)
		{
			merge_value[i].first = 1;
			merge_value[i].second = S[i].index;
		}
	}

	for (petri_index i(0, true); i < S.size(); i++)
	{
		int p = groups.size();
		for (j = 0; j < p; j++)
			if (groups[j].find(i) == groups[j].end())
			{
				ia = prev(i);
				oa = next(i);

				conflict = true;
				for (k = 0; k < groups[j].size() && conflict; k++)
					if ((at(i).index & at(groups[j][k]).index) == 0)
						conflict = false;

				/**
				 * To add a place into a conflict group, it's state encoding must be indistinguishable
				 * with the state encoding of every other place in that group.
				 */
				if (conflict)
				{
					vli.clear();
					vlji.clear();
					vljo.clear();

					at(i).index.vars(&vli);
					merge_value[j].first.vars(&vlji);
					merge_value[j].second.vars(&vljo);

					vli.unique();
					vlji.unique();
					vljo.unique();

					/**
					 * Furthermore, if it is an input place, the amount of information that it's state
					 * encoding contains must be less than or equal to the amount of information contained
					 * in the state encoding of every output place currently in the group and exactly equal
					 * to the amount of state encoding in every input place currently in the group. If it
					 * is an output place, the amount of information that it's state encode contains must
					 * be greater than or equal to the amount of information contained in every input place
					 * currently in the group.
					 *
					 * For example if we start out knowing that the variable X is equal to 0, and then do a
					 * transition and that changes the value of X to 1, we can't suddenly know that the
					 * value of Y is 1 in order to execute the next transition. Also, if we know that
					 * X must be 1 to fire transition A, and X and Y must both be 1 to fire transition B,
					 * we can't suddenly know that Y is 1 every time we want to fire transition A.
					 */

					/* This check for information can be streamlined by checking which variables are
					 * included in the encoding, and not checking their value. The check to make sure that
					 * the state encodings are indistinguishable does the value check for us. For example, if we
					 * knew that X must be 1 to fire transition A, we can't suddenly know that X must also
					 * be 0 to fire transition B. Those two state encodings are not conflicting.
					 */
					/*if ((ia.size() == 0 && (vli == vlji && includes(vljo.begin(), vljo.end(), vli.begin(), vli.end()))) ||
						(oa.size() == 0 && (includes(vli.begin(), vli.end(), vlji.begin(), vlji.end()))))
					{*/
						groups.push_back(groups[j]);
						merge_value.push_back(merge_value[j]);
						groups[j].push_back(i);

						if (ia.size() == 0)
							merge_value[j].first = merge_value[j].first & at(i).index;

						if (oa.size() == 0)
							merge_value[j].second = merge_value[j].second | at(i).index;
					//}
				}
			}
	}

	/* The code segment above results in many duplicate groups and groups that
	 * are subsets of other groups. We need to remove these to make sure that we
	 * only get one place for each unique set of indistinguishable places.
	 */
	for (int i = 0; i < groups.size(); i++)
		groups[i].unique();

	for (int i = 0; i < groups.size(); i++)
		for (j = 0; j < groups.size(); j++)
			if (i != j && (includes(groups[i].begin(), groups[i].end(), groups[j].begin(), groups[j].end()) || groups[j].size() <= 1))
			{
				groups.erase(groups.begin() + j);
				merge_value.erase(merge_value.begin() + j);
				j--;
				if (i > j)
					i--;
			}

	cout << "MERGES" << endl;
	for (int i = 0; i < groups.size(); i++)
	{
		for (j = 0; j < groups[i].size(); j++)
			cout << groups[i][j] << " ";
		cout << endl;
	}
	cout << endl << endl;

	// Merge all of the places in each group identified above.
	for (int i = 0; i < groups.size(); i++)
	{
		pbranch.clear();
		cbranch.clear();
		for (j = 0; j < groups[i].size(); j++)
		{
			pbranch.merge(at(groups[i][j]).pbranch);
			cbranch.merge(at(groups[i][j]).cbranch);
		}
		if (merge_value[i].second == 0)
			merge_value[i].second = 1;

		petri_index p = put_place((merge_value[i].first & merge_value[i].second), pbranch, cbranch, NULL);
		for (j = 0; j < groups[i].size(); j++)
		{
			for (k = 0; k < arcs.size(); k++)
			{
				if (arcs[k].first == groups[i][j])
					arcs.push_back(petri_arc(p, arcs[k].second));
				if (arcs[k].second == groups[i][j])
					arcs.push_back(petri_arc(arcs[k].first, p));
			}
			remove.push_back(groups[i][j]);
		}
	}

	remove.unique();
	for (int i = remove.size()-1; i >= 0; i--)
		cut(remove[i]);
}

void petri_net::zip()
{
	svector<svector<petri_index> > groups;

	/**
	 * First, check the places for possible merges, and then move on to the
	 * transitions because the act of merging places gives more information
	 * about the transitions to which they are connected.
	 *
	 * If two places have exactly the same neighborhood (the same sets of
	 * input and output arcs), then these two places are really just two
	 * possible state encodings for the same place. This generally happens
	 * directly after a conditional merge (where there is one state encoding
	 * for each possible path through the conditional). Instead of merging
	 * just two places at a time, we go ahead and merge all places that have
	 * the same neighborhoods.
	 */
	for (petri_index i(0, true); i < S.size(); i++)
		groups.push_back(svector<petri_index>(1, i));
	for (petri_index i(0, true); i < S.size(); i++)
	{
		int p = groups.size();
		for (int j = 0; j < p; j++)
			if (groups[j].find(i) == groups[j].end() && have_same_source(i, groups[j][0]) && have_same_dest(i, groups[j][0]))
			{
				groups.push_back(groups[j]);
				groups[j].push_back(i);
			}
	}

	/* The code segment above results in many duplicate groups and groups that
	 * are subsets of other groups. We need to remove these to make sure that we
	 * only get one place for each unique neighborhood once we have merge all of
	 * the places in each individual group.
	 */
	for (int i = 0; i < groups.size(); i++)
		groups[i].unique();
	groups.unique();
	for (int i = 0; i < groups.size(); i++)
		for (int j = 0; j < groups.size(); j++)
			if (i != j && (includes(groups[i].begin(), groups[i].end(), groups[j].begin(), groups[j].end()) || groups[j].size() <= 1))
			{
				groups.erase(groups.begin() + j);
				j--;
				if (i > j)
					i--;
			}

	svector<petri_index> remove;
	for (int i = 0; i < groups.size(); i++)
		if (groups[i].size() > 1)
		{
			merge(groups[i]);
			remove.merge(groups[i]);
		}

	remove.unique();
	for (int i = remove.size()-1; i >= 0; i--)
		cut(remove[i]);

	remove.clear();
	groups.clear();

	/**
	 * After checking the places for possible merge points, we also need to
	 * check the transitions. Logically, If there are multiple transitions that
	 * start at the same place and do the same thing, then they should also
	 * have the same result. So we can just merge their output arcs and delete
	 * all but one of the transitions.
	 */
	for (petri_index i(0, false); i < T.size(); i++)
		for (petri_index j = i+1; j < T.size(); j++)
			if (at(i).index == at(j).index && prev(i) == prev(j))
			{
				for (int k = 0; k < arcs.size(); k++)
					if (arcs[k].second == j)
						arcs.push_back(petri_arc(arcs[k].first, i));

				at(i).pbranch = at(i).pbranch.set_intersection(at(j).pbranch);
				at(i).cbranch = at(i).cbranch.set_intersection(at(j).cbranch);

				remove.push_back(j);
			}

	remove.unique();
	cut(remove);

	/* Despite our best efforts to fix up the branch id's on the fly, we still
	 * need to clean up.
	 */
	trim_branch_ids();
}



void petri_net::print_dot(ostream *fout, sstring name)
{
	sstring label;
	(*fout) << "digraph " << name << endl;
	(*fout) << "{" << endl;

	for (petri_index i(0, true); i < S.size(); i++)
		if (!is_floating(i))
		{
			(*fout) << "\t" << i << " [label=\"" << i << " ";
			label = at(i).index.print(vars);

			int k = 0;
			for (int j = 0; j < label.size(); j++)
				if (label[j] == '|')
				{
					(*fout) << label.substr(k, j+1 - k) << "\\n";
					k = j+1;
				}

			(*fout) << label.substr(k) << "\"];" << endl;
		}

	for (petri_index i(0, false); i < T.size(); i++)
	{
		label = at(i).index.print(vars);
		if (!at(i).active)
			label = "[" + label + "]";
		label = i.name() + " " + label;
		if (label != "")
			(*fout) << "\t" << i << " [shape=box] [label=\"" << label << "\"];" << endl;
		else
			(*fout) << "\t" << i << " [shape=box];" << endl;
	}

	for (int i = 0; i < arcs.size(); i++)
		(*fout) << "\t" << arcs[i].first.name() << " -> " << arcs[i].second.name() << "[label=\" " << i << " \"];" <<  endl;

	(*fout) << "}" << endl;
}

void petri_net::print_branch_ids(ostream *fout)
{
	for (int i = 0; i < S.size(); i++)
	{
		(*fout) << "S" << i << ": ";
		for (smap<int, int>::iterator j = S[i].pbranch.begin(); j != S[i].pbranch.end(); j++)
			(*fout) << "p{" << j->first << " " << j->second << "} ";
		for (smap<int, int>::iterator j = S[i].cbranch.begin(); j != S[i].cbranch.end(); j++)
			(*fout) << "c{" << j->first << " " << j->second << "} ";
		(*fout) << endl;
	}
	for (int i = 0; i < T.size(); i++)
	{
		(*fout) << "T" << i << ": ";
		for (smap<int, int>::iterator j = T[i].pbranch.begin(); j != T[i].pbranch.end(); j++)
			(*fout) << "p{" << j->first << " " << j->second << "} ";
		for (smap<int, int>::iterator j = T[i].cbranch.begin(); j != T[i].cbranch.end(); j++)
			(*fout) << "c{" << j->first << " " << j->second << "} ";
		(*fout) << endl;
	}
	(*fout) << endl;
}

void petri_net::print_conflicts(ostream &fout, string name)
{
	fout << "Conflicts: " << name << endl;
	for (smap<petri_index, list<svector<petri_index> > >::iterator i = conflicts.begin(); i != conflicts.end(); i++)
	{
		fout << i->first.name() << ": ";
		for (list<svector<petri_index> >::iterator lj = i->second.begin(); lj != i->second.end(); lj++)
		{
			fout << "{";
			for (int j = 0; j < lj->size(); j++)
				fout << (*lj)[j].name() << " ";
			fout << "} ";
		}
		fout << endl;
	}
}

void petri_net::print_indistinguishables(ostream &fout, string name)
{
	smap<petri_index, list<svector<petri_index> > >::iterator i;
	list<svector<petri_index> >::iterator lj;

	fout << "Indistinguishables: " << name << endl;
	for (i = indistinguishable.begin(); i != indistinguishable.end(); i++)
	{
		fout << i->first.name() << ": ";
		for (lj = i->second.begin(); lj != i->second.end(); lj++)
		{
			fout << "{";
			for (int j = 0; j < lj->size(); j++)
				fout << (*lj)[j].name() << " ";
			fout << "} ";
		}
		fout << endl;
	}
}

void petri_net::print_positive_conflicts(ostream &fout, string name)
{
	smap<petri_index, list<svector<petri_index> > >::iterator i;
	list<svector<petri_index> >::iterator lj;

	fout << "Negative Indistinguishables: " << name << endl;
	for (i = negative_indistinguishable.begin(); i != negative_indistinguishable.end(); i++)
	{
		fout << i->first.name() << ": ";
		for (lj = i->second.begin(); lj != i->second.end(); lj++)
		{
			fout << "{";
			for (int j = 0; j < lj->size(); j++)
				fout << (*lj)[j].name() << " ";
			fout << "} ";
		}
		fout << endl;
	}
}

void petri_net::print_positive_indistinguishables(ostream &fout, string name)
{
	smap<petri_index, list<svector<petri_index> > >::iterator i;
	list<svector<petri_index> >::iterator lj;

	fout << "Positive Indistinguishables: " << name << endl;
	for (i = positive_indistinguishable.begin(); i != positive_indistinguishable.end(); i++)
	{
		fout << i->first.name() << ": ";
		for (lj = i->second.begin(); lj != i->second.end(); lj++)
		{
			fout << "{";
			for (int j = 0; j < lj->size(); j++)
				fout << (*lj)[j].name() << " ";
			fout << "} ";
		}
		fout << endl;
	}
}

void petri_net::print_negative_conflicts(ostream &fout, string name)
{
	smap<petri_index, list<svector<petri_index> > >::iterator i;
	list<svector<petri_index> >::iterator lj;

	fout << "Positive Conflicts: " << name << endl;
	for (i = positive_conflicts.begin(); i != positive_conflicts.end(); i++)
	{
		fout << i->first.name() << ": ";
		for (lj = i->second.begin(); lj != i->second.end(); lj++)
		{
			fout << "{";
			for (int j = 0; j < lj->size(); j++)
				fout << (*lj)[j].name() << " ";
			fout << "} ";
		}
		fout << endl;
	}
}

void petri_net::print_negative_indistinguishables(ostream &fout, string name)
{
	smap<petri_index, list<svector<petri_index> > >::iterator i;
	list<svector<petri_index> >::iterator lj;

	fout << "Negative Conflicts: " << name << endl;
	for (i = negative_conflicts.begin(); i != negative_conflicts.end(); i++)
	{
		fout << i->first.name() << ": ";
		for (lj = i->second.begin(); lj != i->second.end(); lj++)
		{
			fout << "{";
			for (int j = 0; j < lj->size(); j++)
				fout << (*lj)[j].name() << " ";
			fout << "} ";
		}
		fout << endl;
	}
}

petri_state::petri_state()
{

}

struct petri_state_execution
{
	petri_state_execution(){}
	petri_state_execution(svector<petri_index> s, petri_net *net){state = s; covered.resize(net->S.size() + net->T.size(), false);}
	~petri_state_execution(){}

	svector<petri_index> state;
	svector<bool> covered;
};

/**
 * Initializes a petri state given a single place. The resulting state
 * is one that has an index at the given place and then enough indices
 * at each parent parallel merge point to allow through travel of the
 * first index.
 */
petri_state::petri_state(petri_net *net, svector<petri_index> start, bool backward)
{
	state.merge(start);
	list<petri_state_execution> execs(1, petri_state_execution(start, net));

	/**
	 * Run through all possible executions from the starting index
	 * looking for deadlock.
	 */
	for (list<petri_state_execution>::iterator exec = execs.begin(); exec != execs.end(); exec = execs.erase(exec))
	{
		//cout << "\tStart Execution" << endl;
		bool done = false;
		while (!done)
		{
			svector<int> movable;
			for (int i = 0; i < exec->state.size(); i++)
			{
				svector<petri_index> n = backward ? net->prev(exec->state[i]) : net->next(exec->state[i]);
				if (exec->state[i].is_place())
				{
					int total = 0;
					for (int k = i; k < exec->state.size(); k++)
						if ((backward ? net->prev(exec->state[k]) : net->next(exec->state[k]))[0] == n[0])
							total++;

					if (total == (backward ? net->next(n) : net->prev(n)).unique().size())
						movable.push_back(i);
				}
				else
					movable.push_back(i);
			}

			if (movable.size() != 0)
			{
				// Check to see if we are done here...
				done = true;
				for (int i = 0; done && i < exec->state.size(); i++)
					if (!exec->covered[exec->state[i].is_trans() ? exec->state[i].idx() + net->S.size() : exec->state[i].idx()])
						done = false;

				// Mark the nodes we just covered...
				for (int i = 0; i < exec->state.size(); i++)
					exec->covered[exec->state[i].is_trans() ? exec->state[i].idx() + net->S.size() : exec->state[i].idx()] = true;
			}

			/*cout << "\t{";
			for (int i = 0; i < exec->state.size(); i++)
			{
				if (i != 0)
					cout << " ";
				cout << exec->state[i];
			}
			cout << "}" << endl;*/

			/* If we are not done, handle the next set of movements
			 * duplicating executions or indices when necessary.
			 */
			for (int i = 0; !done && i < movable.size(); i++)
			{
				if (exec->state[movable[i]].is_place())
				{
					for (int k = movable[i]+1; k < exec->state.size(); )
					{
						if ((backward && net->prev(exec->state[k])[0] == net->prev(exec->state[movable[i]])[0]) ||
						   (!backward && net->next(exec->state[k])[0] == net->next(exec->state[movable[i]])[0]))
						{
							for (int j = i+1; j < movable.size(); j++)
								if (movable[j] > k)
									movable[j]--;

							exec->state.erase(exec->state.begin() + k);
						}
						else
							k++;
					}
				}

				svector<petri_index> n = backward ? net->prev(exec->state[movable[i]]) : net->next(exec->state[movable[i]]);
				for (int k = n.size()-1; k >= 0; k--)
				{
					if (k > 0)
					{
						if (exec->state[movable[i]].is_place())
						{
							execs.push_back(*exec);
							execs.back().state[movable[i]] = n[k];
						}
						else
							exec->state.push_back(n[k]);
					}
					else
						exec->state[movable[i]] = n[k];
				}
			}

			/**
			 * Every time deadlock is detected in the execution,
			 * insert enough indices at the merge point to allow
			 * execution to continue. Also, insert these indices
			 * into the state we are trying to initialize.
			 */
			if (!done && movable.size() == 0)
			{
				svector<petri_index> counts;
				// Count up how many indices we already have at each merge point.
				for (int i = 0; i < exec->state.size(); i++)
					counts.merge(backward ? net->next(net->prev(exec->state[i])) : net->prev(net->next(exec->state[i])));
				counts.unique();

				for (int i = 0; i < counts.size(); i++)
				{
					if (exec->state.find(counts[i]) == exec->state.end())
					{
						exec->state.push_back(counts[i]);
						state.push_back(counts[i]);
					}
				}
			}
		}
	}

	// Sort the state so that we can have some standard for comparison.
	this->state.sort();
}

petri_state::~petri_state()
{

}

/**
 * Count up the number of indices that equal the jth index
 */
int petri_state::count(int j)
{
	int total = 0;
	for (int i = j; i < state.size(); i++)
		if (state[i] == state[j])
			total++;
	return total;
}

/**
 * Delete all but one of the indices that equal the jth index.
 * This preserves the index at j.
 */
void petri_state::merge(int j)
{
	for (int i = j+1; i < state.size();)
	{
		if (state[i] == state[j])
			state.erase(state.begin() + i);
		else
			i++;
	}
}

bool petri_state::is_state()
{
	for (int i = 0; i < state.size(); i++)
		if (state[i].is_trans())
			return false;
	return true;
}

petri_state &petri_state::operator=(petri_state s)
{
	state = s.state;
	return *this;
}

ostream &operator<<(ostream &os, petri_state s)
{
	os << "{";
	for (int i = 0; i < s.state.size(); i++)
	{
		if (i != 0)
			os << " ";
		os << s.state[i];
	}
	os << "}";
	return os;
}

bool operator==(petri_state s1, petri_state s2)
{
	return s1.state == s2.state;
}

bool operator!=(petri_state s1, petri_state s2)
{
	return s1.state != s2.state;
}

bool operator<(petri_state s1, petri_state s2)
{
	return s1.state < s2.state;
}

bool operator>(petri_state s1, petri_state s2)
{
	return s1.state > s2.state;
}

bool operator<=(petri_state s1, petri_state s2)
{
	return s1.state <= s2.state;
}

bool operator>=(petri_state s1, petri_state s2)
{
	return s1.state >= s2.state;
}
