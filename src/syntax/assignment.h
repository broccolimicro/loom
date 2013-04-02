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

	list<pair<string, string> > expr;

	assignment &operator=(assignment a);

	instruction *duplicate(instruction *parent, vspace *vars, map<string, string> convert, string tab, int verbosity);
	state variant();
	state active_variant();
	state passive_variant();
	void x_channel(state *s, string v);

	void expand_shortcuts();
	void parse();
	void merge();
	int generate_states(graph *trans, int init, state filter);
	state simulate_states(state init, state filter);
	void generate_scribes();

	void insert_instr(int uid, int nid, instruction *instr);

	void print_hse(string t);
};

instruction *expand_assignment(instruction *parent, string chp, vspace *vars, string tab, int verbosity);
pair<string, instruction*> expand_expression(string chp, vspace *vars, string top, string tab, int verbosity);

#endif
