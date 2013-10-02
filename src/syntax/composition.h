/*
 * composition.h
 *
 *  Created on: Jul 16, 2013
 *      Author: nbingham
 */

#include "instruction.h"

#ifndef composition_h
#define composition_h

struct composition : instruction
{
	composition();
	virtual ~composition();

	list<instruction*> instrs;		// an ordered list of instructions in sequential

	void init(sstring chp, variable_space *vars, flag_space *flags);
	void clear();

	virtual void push(instruction *i) = 0;

	instruction *expand_assignment(sstring chp);
	instruction *expand_condition(sstring chp);
	instruction *expand_loop(sstring chp);
};

#endif
