/*
 * rule.cpp
 *
 *  Created on: Nov 11, 2012
 *      Author: Ned Bingham and Nicholas Kramer
 *
 *  DO NOT DISTRIBUTE
 */

#include "rule.h"


rule::rule()
{
	//actual.clear();
	left = "";
	//desired.clear();
	right = "";
}

rule::~rule()
{
	//actual.clear();
	left = "";
	//desired.clear();
	right = "";
}

rule &rule::operator=(rule s)
{
	left = s.left;
	right = s.right;
	implicants = s.implicants;
	return *this;
}

void rule::clear(int n)
{
	//actual.clear();
	//for (int i = 0; i < n; i++)
	//	actual.push_back(value("1"));
	left = "";
	//desired.clear();
	right = "";
	implicants.clear();
}

/* This function returns the nth necessary firing of a production rule.
 *
 */
/*int rule::index(int n)
{
	vector<value>::iterator i;
	int j;
	for (i = desired.begin(), j = 0; i != desired.end() && n > 0; i++, j++)
		if (i->data == "1")
			n--;

	return j;
}*/

ostream &operator<<(ostream &os, rule r)
{
	list<state>::iterator i;

    os << r.left << " -> " << r.right;

    return os;
}
