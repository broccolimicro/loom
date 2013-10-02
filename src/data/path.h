/*
 * path.h
 *
 *  Created on: Mar 14, 2013
 *      Author: nbingham
 */

#include "../common.h"

#ifndef path_h
#define path_h

struct path
{
	path(int s);
	path(int s, int f, int t);
	path(int s, int f, svector<int> t);
	path(int s, svector<int> f, int t);
	path(int s, svector<int> f, svector<int> t);
	~path();

	svector<int> from, to;
	svector<int> nodes;

	int size();
	void clear();
	bool contains(int n);
	void set(int n);
	bool empty();
	svector<int> maxes();
	int max();
	path inverse();
	path mask();
	svector<int>::iterator begin();
	svector<int>::iterator end();

	path &operator=(path p);

	int &operator[](int i);
};

ostream &operator<<(ostream &os, path p);

path operator+(path p1, path p2);
path operator/(path p1, int n);
path operator*(path p1, int n);

bool operator==(path p1, path p2);
bool operator<(path p1, path p2);
bool operator>(path p1, path p2);

#endif
