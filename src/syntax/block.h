/*
 * block.h
 *
 * This structure represents a block. An conditional statement
 * or loop can be considered a block. By definition, it is
 * something that we can pull out and analyze independently of
 * every other structure in the program. Within a block, there
 * can be a process instantiation, variable declarations, and sub-blocks.
 * We also include a list of spaces to keep track of the state space
 * transition affected by this block.
 */

#ifndef block_h
#define block_h

#include "instruction.h"


struct block : instruction
{
	block();
	block(instruction *parent, string chp, vspace *vars, string tab, int verbosity);
	~block();

	list<instruction*>			instrs;		// an ordered list of instructions in block

	block &operator=(block b);

	void init(string chp, vspace *vars, string tab, int verbosity);

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

	void clear();

	void insert_instr(int uid, int nid, instruction *instr);

	void print_hse(string t);

	void push(instruction *i);
};

//bool cycle(space start, space end, list<rule> prs);

size_t search_back(string s, size_t offset);
size_t search_front(string s, size_t offset);
//list<size_t> state_variable_positions(space left, space right, string tab, int verbosity);
//bool production_rule_check(string *raw, block *b, string tab, int verbosity);

#endif
