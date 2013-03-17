/*
 * assignment.h
 *
 *  Created on: Jan 22, 2013
 *      Author: nbingham
 */

#ifndef assignment_h
#define assignment_h

#include "instruction.h"

struct assignment : instruction
{
	assignment();
	assignment(instruction *parent, string chp, vspace *vars, string tab, int verbosity);
	~assignment();

	int uid;					// indexes into the state in the state space
	list<pair<string, string> > expr;

	assignment &operator=(assignment a);

	instruction *duplicate(instruction *parent, vspace *vars, map<string, string> convert, string tab, int verbosity);
	state variant();

	void expand_shortcuts();
	void parse();
	int generate_states(graph *trans, int init);
	void generate_scribes();

	void print_hse();
};

instruction *expand_assignment(instruction *parent, string chp, vspace *vars, string tab, int verbosity);
pair<string, instruction*> expand_expression(string chp, vspace *vars, string top, string tab, int verbosity);

#endif
