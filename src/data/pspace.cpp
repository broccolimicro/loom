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
	total.nodes.assign(p.nodes.size(), 0);
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

int path_space::coverage_count(int n)
{
	return total.nodes[n];
}

int path_space::coverage_max()
{
	return total.max();
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
