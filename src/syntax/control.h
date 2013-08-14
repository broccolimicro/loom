/*
 * control.h
 *
 *  Created on: Jul 16, 2013
 *      Author: nbingham
 */

#include "instruction.h"
#include "sequential.h"
#include "guard.h"

#ifndef control_h
#define control_h

enum control_type
{
	unknown = 0,
	mutex = 1,
	choice = 2
};

struct control : instruction
{
	control();
	virtual ~control();

	control_type type;
	list<pair<sequential*, guard*> > instrs;		//Guards index instructions

	void clear();

	pair<string, instruction*> expand_guard(string chp);
};

#endif
