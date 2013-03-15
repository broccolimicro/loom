/*
 * pspace.h
 *
 *  Created on: Mar 14, 2013
 *      Author: nbingham
 */

#include "path.h"

#ifndef pspace_h
#define pspace_h

struct path_space
{
	path_space();
	path_space(int s);
	~path_space();

	list<path> paths;
	path total;

	int size();
	void merge(path_space s);
	void push_back(path p);
	list<path>::iterator begin();
	list<path>::iterator end();

	int coverage_count(int n);
	int coverage_max();
	path_space remainder(int n);
	path_space coverage(int n);


	path_space &operator=(path_space s);

	path &operator[](int i);
};

ostream &operator<<(ostream &os, path_space p);

#endif
