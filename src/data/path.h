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
	path(int s, int f, vector<int> t);
	path(int s, vector<int> f, int t);
	path(int s, vector<int> f, vector<int> t);
	~path();

	vector<int> from, to;
	vector<int> nodes;

	int size();
	void clear();
	bool contains(int n);
	void set(int n);
	bool empty();
	vector<int> maxes();
	int max();
	path inverse();
	path mask();
	vector<int>::iterator begin();
	vector<int>::iterator end();

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
