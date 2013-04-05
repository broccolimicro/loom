/*
 * assignment.h
 *
 * An assignment is a basic CHP syntax of the form variable:=value, often shorthanded
 * with variable+ for assignment to 1 and variable- for assignment to 0.
 * The assignment structure contains a uid indicating the statespace state it affects,
 * as well as a list of simultaneous assignments (expression variable pairs).
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
