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
	path();
	path(int s);
	~path();

	vector<int> from, to;
	vector<int> nodes;

	int size();
	void clear();
	bool contains(int n);
	void set(int n);
	vector<int> max();
	path inverse();
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
