/*
 * block.h
 *
 *  Created on: Oct 28, 2012
 *      Author: Ned Bingham and Nicholas Kramer
 *
 *  DO NOT DISTRIBUTE
 */

#ifndef block_h
#define block_h

#include "instruction.h"

/* This structure represents a block. An conditional statement
 * or loop can be considered a block. By definition, it is
 * something that we can pull out and analyze independently of
 * every other structure in the program. Within a block, there
 * can be a process instantiation, variable declarations, and sub-blocks.
 * We also include a list of spaces to keep track of the state space
 * transition affected by this block.
 */
struct block : instruction
{
	block();
	block(string chp, vspace *vars, string tab, int verbosity);
	~block();

	list<instruction*>			instrs;		// an ordered list of instructions in block

	block &operator=(block b);

	void init(string chp, vspace *vars, string tab, int verbosity);

	instruction *duplicate(vspace *vars, map<string, string> convert, string tab, int verbosity);

	void expand_shortcuts();
	void parse();
	void simplify();
	int generate_states(graph *trans, int init);
	void generate_scribes();

	void clear();

	void print_hse();

	void push(instruction *i);
};

//bool cycle(space start, space end, list<rule> prs);

size_t search_back(string s, size_t offset);
size_t search_front(string s, size_t offset);
//list<size_t> state_variable_positions(space left, space right, string tab, int verbosity);
//bool production_rule_check(string *raw, block *b, string tab, int verbosity);

#endif
