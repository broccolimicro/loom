/*
 * conditional.h
 *
 * A conditional is a fundamental CHP syntax that takes the form [G1->S1[]G2->S2]
 * Where G1 and G2 are guards and S1 and S2 are statements to execute. If G1 evaluates.
 * S1 is to execute. If G2 evaluates, S2 is to execute. If neither guard evaluates,
 * the statement is to sequential until one of the guards does evaluate. The environment guarantees
 * mutual exclusion for [] (deterministic selection). Nondeterministic selection is yet to be
 * implemented. TODO: Description on what conditional programatically does.
 */

#ifndef conditional_h
#define conditional_h

#include "guard.h"
#include "sequential.h"
#include "parallel.h"

enum conditional_type
{
	unknown = 0,
	mutex = 1,
	choice = 2
};

struct conditional : parallel
{
	conditional();
	conditional(instruction *parent, string chp, vspace *vars, string tab, int verbosity);
	~conditional();

	conditional_type type;
	list<pair<sequential*, guard*> > instrs;		//Guards index instructions

	conditional &operator=(conditional c);

	instruction *duplicate(instruction *parent, vspace *vars, map<string, string> convert, string tab, int verbosity);
	state variant();
	state active_variant();
	state passive_variant();

	void expand_shortcuts();
	void parse();
	void merge();
	int generate_states(graph *trans, int init, state filter);
	state simulate_states(state init, state filter);
	void generate_scribes();

	void insert_instr(int uid, int nid, instruction *instr);

	void print_hse(string t);
};

#endif
