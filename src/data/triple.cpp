/*
 * triple.cpp
 *
 *  Created on: Jul 23, 2013
 *      Author: nbingham
 */

#include "../common.h"
#include "triple.h"

triple::triple()
{
}

triple::triple(int i, int l, int h)
{
	this->i = i;
	this->l = l;
	this->h = h;
}

triple::~triple()
{
}

triple &triple::operator=(triple t)
{
	this->i = t.i;
	this->l = t.l;
	this->h = t.h;
	return *this;
}

bool operator==(triple t1, triple t2)
{
	return (t1.i == t2.i && t1.l == t2.l && t1.h == t2.h);
}
