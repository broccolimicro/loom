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
	sequential(instruction *parent, string chp, vspace *vars, string tab, int verbosity);
	~sequential();

	list<instruction*>			instrs;		// an ordered list of instructions in sequential

	instruction *duplicate(instruction *parent, vspace *vars, map<string, string> convert, string tab, int verbosity);

	void expand_shortcuts();
	void parse();
	void merge();
	vector<int> generate_states(petri *n, vector<int> f, map<int, int> pbranch, map<int, int> cbranch);

	void insert_instr(int uid, int nid, instruction *instr);

	void print_hse(string t = "", ostream *fout = &cout);

	void push(instruction *i);
};

//bool cycle(space start, space end, list<rule> prs);

size_t search_back(string s, size_t offset);
size_t search_front(string s, size_t offset);
//list<size_t> state_variable_positions(space left, space right, string tab, int verbosity);
//bool production_rule_check(string *raw, sequential *b, string tab, int verbosity);

#endif
