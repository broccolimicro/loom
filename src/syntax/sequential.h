/*
 * sequential.h
 *
 * This structure represents a sequential. An condition statement
 * or loop can be considered a sequential. By definition, it is
 * something that we can pull out and analyze independently of
 * every other structure in the program. Within a sequential, there
 * can be a process instantiation, variable declarations, and sub-sequentials.
 * We also include a list of spaces to keep track of the state space
 * transition affected by this sequential.
 */

#ifndef sequential_h
#define sequential_h

#include "composition.h"

struct sequential : composition
{
	sequential();
	sequential(instruction *parent, sstring chp, variable_space *vars, flag_space *flags);
	~sequential();

	list<instruction*>			instrs;		// an ordered list of instructions in sequential

	instruction *duplicate(instruction *parent, variable_space *vars, smap<sstring, sstring> convert);

	void expand_shortcuts();
	void parse();
	void simulate();
	void rewrite();
	void reorder();
	svector<petri_index> generate_states(petri_net *n, rule_space *p, svector<petri_index> f, smap<int, int> pbranch, smap<int, int> cbranch);

	void print_hse(sstring t = "", ostream *fout = &cout);

	void push(instruction *i);
};

//bool cycle(space start, space end, list<rule> prs);

int search_back(sstring s, int offset);
int search_front(sstring s, int offset);
//list<int> state_variable_positions(space left, space right, flag_space *flags);
//bool production_rule_check(sstring *raw, sequential *b, flag_space *flags);

#endif
