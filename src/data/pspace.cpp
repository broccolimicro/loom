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
	total.nodes.resize(s);
	ntotal.nodes.resize(s);
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
	{
		total.nodes[i] += s.total.nodes[i];
		ntotal.nodes[i] += s.ntotal.nodes[i];
	}
}

void path_space::push_back(path p)
{
	paths.push_back(p);

	for (int i = 0; i < p.size(); i++)
	{
		total.nodes[i] += p.nodes[i];
		ntotal.nodes[i] += 1 - p.nodes[i];
	}
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
}

int path_space::coverage_count(int n)
{
	return total.nodes[n];
}

int path_space::avoidance_count(int n)
{
	return ntotal.nodes[n];
}

int path_space::coverage_max()
{
	return total.max();
}

int path_space::avoidance_max()
{
	return ntotal.max();
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

path_space path_space::inverse()
{
	path_space result(total.size());

	for (list<path>::iterator i = paths.begin(); i != paths.end(); i++)
	{
		path p(i->nodes.size());
		p.from = i->from;
		p.to = i->to;
		for (size_t j = 0; j < i->nodes.size(); j++)
			p.nodes.assign(j, 1 - i->nodes[j]);
		result.push_back(p);
	}

	return result;
}

path_space path_space::associations(int from)
{
	list<path>::iterator i;
	path_space result(total.size());

	for (i = paths.begin(); i != paths.end(); i++)
		if (i->from == from)
			result.push_back(*i);

	return result;
}

path_space &path_space::operator=(path_space s)
{
	paths.clear();
	total.clear();
	ntotal.clear();
	paths = s.paths;
	total = s.total;
	ntotal = s.ntotal;
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
