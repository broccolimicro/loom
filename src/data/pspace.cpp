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
		total[i] += s.total[i];
}

void path_space::push_back(path p)
{
	paths.push_back(p);

	for (int i = 0; i < p.size(); i++)
		total[i] += p[i];
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
	return total[n];
}

int path_space::coverage_max()
{
	int r = -1;
	int t = 0;
	for (int i = 0; i != total.size(); i++)
	{
		if (total[i] > t)
		{
			t = total[i];
			r = i;
		}
	}

	return r;
}

path_space path_space::remainder(int n)
{
	list<path>::iterator i;
	path_space result(size());

	for (i = paths.begin(); i != paths.end(); i++)
		if (!i->contains(n))
			result.push_back(*i);

	return result;
}

path_space path_space::coverage(int n)
{
	list<path>::iterator i;
	path_space result(size());

	for (i = paths.begin(); i != paths.end(); i++)
		if (i->contains(n))
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
