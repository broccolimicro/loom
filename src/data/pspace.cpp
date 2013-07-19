/*
 * pspace.cpp
 *
 *  Created on: Mar 14, 2013
 *      Author: nbingham
 */

#include "pspace.h"

path_space::path_space()
{

}

path_space::path_space(int s)
{
	total.nodes.assign(s, 0);
}

path_space::path_space(path p)
{
	bool empty = true;
	for (int i = 0; i < (int)p.nodes.size() && empty; i++)
		if (p.nodes[i] > 0)
			empty = false;

	total.nodes.assign(p.nodes.size(), 0);
	if (!empty)
		push_back(p);

	total.from = p.from;
	total.to = p.to;
}

path_space::~path_space()
{

}

int path_space::size()
{
	return paths.size();
}

void path_space::merge(path_space s)
{
	paths.merge(s.paths);

	for (int i = 0; i < s.total.size(); i++)
		total.nodes[i] += s.total.nodes[i];

	total.from.insert(total.from.end(), s.total.from.begin(), s.total.from.end());
	total.to.insert(total.to.end(), s.total.to.begin(), s.total.to.end());
}

void path_space::push_back(path p)
{
	paths.push_back(p);

	for (int i = 0; i < p.size(); i++)
		total.nodes[i] += p.nodes[i];

	total.from.insert(total.from.end(), p.from.begin(), p.from.end());
	total.to.insert(total.to.end(), p.to.begin(), p.to.end());
}

list<path>::iterator path_space::begin()
{
	return paths.begin();
}

list<path>::iterator path_space::end()
{
	return paths.end();
}

void path_space::clear()
{
	paths.clear();
	total.clear();
}

void path_space::zero(int i)
{
	list<path>::iterator pi;
	for (pi = paths.begin(); pi != paths.end(); pi++)
	{
		pi->nodes[i] = 0;
		if (pi->empty())
			pi = paths.erase(pi);
	}
	total.nodes[i] = 0;
}

void path_space::zero(vector<int> i)
{
	list<path>::iterator pi;
	int j;
	for (pi = paths.begin(); pi != paths.end(); pi++)
	{
		for (j = 0; j < (int)i.size(); j++)
			pi->nodes[i[j]] = 0;
		if (pi->empty())
			pi = paths.erase(pi);
	}
	for (j = 0; j < (int)i.size(); j++)
		total.nodes[i[j]] = 0;
}

int path_space::coverage_count(int n)
{
	return total.nodes[n];
}

int path_space::coverage_count(vector<int> n)
{
	int i;
	int m = 0;
	for (i = 0; i < (int)n.size(); i++)
		if (total.nodes[n[i]] > m)
			m = total.nodes[n[i]];
	return m;
}

vector<int> path_space::coverage_maxes()
{
	return total.maxes();
}

int path_space::coverage_max()
{
	return total.max();
}

path path_space::get_mask()
{
	path result;
	for (int i = 0; i < (int)total.nodes.size(); i++)
		result.nodes.push_back((int)(total.nodes[i] > 0));
	result.from = total.from;
	result.to = total.to;
	return result;
}

void path_space::apply_mask(path m)
{
	list<path>::iterator pi;
	int i;

	for (pi = paths.begin(); pi != paths.end(); pi++)
	{
		for (i = 0; i < (int)pi->size(); i++)
			pi->nodes[i] *= m.nodes[i];
		if (pi->empty())
			pi = paths.erase(pi);
	}

	for (int i = 0; i < total.nodes.size(); i++)
		total.nodes[i] *= m.nodes[i];
}

path_space path_space::inverse()
{
	list<path>::iterator i;
	path_space result(total.size());

	for (i = paths.begin(); i != paths.end(); i++)
		result.push_back(i->inverse());

	return result;
}

path_space path_space::coverage(int n)
{
	list<path>::iterator i;
	path_space result(total.size());

	for (i = paths.begin(); i != paths.end(); i++)
		if (i->contains(n))
			result.push_back(*i);

	return result;
}

path_space path_space::avoidance(int n)
{
	list<path>::iterator i;
	path_space result(total.size());

	for (i = paths.begin(); i != paths.end(); i++)
		if (!i->contains(n))
			result.push_back(*i);

	return result;
}

path_space &path_space::operator=(path_space s)
{
	paths.clear();
	total.clear();
	paths = s.paths;
	total = s.total;
	return *this;
}

path &path_space::operator[](int i)
{
	list<path>::iterator j;
	for (j = paths.begin(); j != paths.end() && i > 0; j++, i--);

	return *j;
}

ostream &operator<<(ostream &os, path_space p)
{
	list<path>::iterator i;
	for (i = p.paths.begin(); i != p.paths.end(); i++)
		cout << *i << endl;
	return os;
}
