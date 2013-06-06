/*
 * instruction.cpp
 *
 *  Created on: Oct 28, 2012
 *      Author: Ned Bingham and Nicholas Kramer
 *
 *  DO NOT DISTRIBUTE
 */

#include "../common.h"
#include "../utility.h"
#include "../type.h"
#include "../data.h"

#include "instruction.h"

instruction::instruction()
{
	parent = NULL;
	vars = NULL;
	net = NULL;
	chp = "";
	tab = "";
	_kind = "instruction";
	verbosity = VERB_NONE;
}

instruction::~instruction()
{
	_kind = "instruction";
	parent = NULL;
}

string instruction::kind()
{
	return _kind;
}
