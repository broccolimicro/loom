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
}

path::path(int s)
{
	nodes.resize(s, 0);
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
	nodes.assign(nodes.size(), 0);
}

bool path::contains(int n)
{
	return (nodes[n] > 0);
}

void path::set(int n)
{
	nodes[n] = 1;
}

bool path::empty()
{
	for (int i = 0; i < (int)nodes.size(); i++)
		if (nodes[i] > 0)
			return false;
	return true;
}

vector<int> path::maxes()
{
	vector<int> r;
	int t = -1;
	size_t i;
	for (i = 0; i < nodes.size(); i++)
		if (nodes[i] > t)
			t = nodes[i];

	for (i = 0; i < nodes.size() && t > 0; i++)
		if (nodes[i] == t)
			r.push_back(i);

	return r;
}

int path::max()
{
	int r;
	int t = -1;
	int i;
	for (i = 0; i < (int)nodes.size(); i++)
		if (nodes[i] > t)
		{
			t = nodes[i];
			r = i;
		}

	return r;
}

path path::inverse()
{
	path result;
	int i;

	for (i = 0; i < nodes.size(); i++)
		result.nodes.push_back(1 - nodes[i]);

	result.from = from;
	result.to = to;

	return result;
}

path path::mask()
{
	path result;
	int i;

	for (i = 0; i < nodes.size(); i++)
		result.nodes.push_back(nodes[i] > 0);

	result.from = from;
	result.to = to;

	return result;
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
