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
	return (nodes[n] > 0);
}

void path::set(int n)
{
	nodes[n] = 1;
}

int path::max()
{
	int r = -1;
	int t = 0;
	for (size_t i = 0; i < nodes.size(); i++)
	{
		if (nodes[i] > t)
		{
			t = nodes[i];
			r = i;
		}
	}

	return r;
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

path operator+(path p1, path p2)
{
	path result(min(p1.size(), p2.size()));
	for (int i = 0; i < p1.size() && i < p2.size(); i++)
		result.nodes[i] = p1[i] + p2[i];

	return result;
}

path operator/(path p1, int n)
{
	for (int i = 0; i < p1.size(); i++)
		p1[i] /= n;
	return p1;
}

path operator*(path p1, int n)
{
	for (int i = 0; i < p1.size(); i++)
		p1[i] *= n;
	return p1;
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
