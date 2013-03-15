/*
 * path.cpp
 *
 *  Created on: Mar 14, 2013
 *      Author: nbingham
 */

#include "path.h"
#include "../common.h"

path::path()
{
	from = -1;
	to = -1;
}

path::path(int s)
{
	nodes.resize(s);
	from = -1;
	to = -1;
}

path::~path()
{

}

int path::size()
{
	return nodes.size();
}

void path::clear()
{
	nodes.clear();
}

bool path::contains(int n)
{
	if ((int)nodes.size() <= n)
		nodes.resize(n+1, 0);
	return (nodes[n] > 0);
}

void path::set(int n)
{
	if ((int)nodes.size() <= n)
		nodes.resize(n+1, 0);
	nodes[n] = 1;
}

vector<int>::iterator path::begin()
{
	return nodes.begin();
}

vector<int>::iterator path::end()
{
	return nodes.end();
}

path &path::operator=(path p)
{
	nodes.clear();
	nodes = p.nodes;
	from = p.from;
	to = p.to;
	return *this;
}

int &path::operator[](int i)
{
	if ((int)nodes.size() <= i)
		nodes.resize(i+1, 0);
	return nodes[i];
}

ostream &operator<<(ostream &os, path p)
{
	vector<int>::iterator i;
	for (i = p.begin(); i != p.end(); i++)
		os << *i << " ";
	return os;
}

bool operator==(path p1, path p2)
{
	vector<int>::iterator i, j;
	for (i = p1.begin(), j = p2.begin(); i != p1.end() && j != p2.end(); i++, j++)
		if (*i != *j)
			return false;

	return true;
}

bool operator<(path p1, path p2)
{
	vector<int>::iterator i, j;
	for (i = p1.begin(), j = p2.begin(); i != p1.end() && j != p2.end(); i++, j++)
		if (*i != *j)
			return *i < *j;

	return false;
}


bool operator>(path p1, path p2)
{
	vector<int>::iterator i, j;
	for (i = p1.begin(), j = p2.begin(); i != p1.end() && j != p2.end(); i++, j++)
		if (*i != *j)
			return *i > *j;

	return false;
}
