/*
 * conditional.cpp
 *
 *  Created on: Oct 29, 2012
 *      Author: Ned Bingham
 */

#include "conditional.h"
#include "common.h"

conditional::conditional()
{
	_kind = "conditional";
}

conditional::conditional(string raw)
{
	_kind = "conditional";
	parse(raw);
}

conditional::~conditional()
{
	_kind = "conditional";
}

void conditional::parse(string raw)
{
	chp = raw;

	cout << "\t\tConditional:\t" << chp << endl;
}
