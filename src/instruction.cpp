/*
 * instruction.cpp
 *
 *  Created on: Oct 28, 2012
 *      Author: Ned Bingham and Nicholas Kramer
 *
 *  DO NOT DISTRIBUTE
 */

#include "instruction.h"
#include "common.h"

instruction::instruction()
{
	chp = "";
	tab = "";
	_kind = "instruction";
	verbosity = VERB_SUPPRESS;
}

instruction::~instruction()
{
	_kind = "instruction";
}

string instruction::kind()
{
	return _kind;
}

void instruction::print_prs()
{
	list<rule>::iterator i;
	if (verbosity >= VERB_PRSALG)
	{
		cout << tab << "Production Rules" << endl;

		for (i = rules.begin(); i != rules.end(); i++)
			cout << tab + "\t" << *i << endl;
	}
}
