/*
 * loop.cpp
 *
 *  Created on: Oct 29, 2012
 *      Author: Ned Bingham
 */

#include "loop.h"
#include "common.h"

loop::loop()
{
	_kind = "loop";
}

loop::loop(string raw)
{
	_kind = "loop";
	parse(raw);
}

loop::~loop()
{
	_kind = "loop";
}

void loop::parse(string raw)
{
	chp = raw;
	cout << "\t\tLoop:\t" << chp << endl;
}
