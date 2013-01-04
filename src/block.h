/*
 * block.h
 *
 *  Created on: Oct 28, 2012
 *      Author: Ned Bingham and Nicholas Kramer
 *
 *  DO NOT DISTRIBUTE
 */

#include "common.h"
#include "instruction.h"
#include "variable.h"
#include "space.h"
#include "keyword.h"

#ifndef block_h
#define block_h

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
	block(string id, string raw, map<string, keyword*> types, map<string, variable*> vars, map<string, state> init, string tab, int verbosity);
	~block();

	map<string, variable*>	local;
	map<string, variable*>	global;
	list<instruction*>		instrs;		// an ordered list of instructions in block
	map<string, space>		states;		// (indexed by variable) the state space of this block. format "i####" or "o####"
	list<size_t> waits;
	list<map<string, state> >		   changes;

	block &operator=(block b);

	void parse(string id, string raw, map<string, keyword*> types, map<string, variable*> vars, map<string, state> init, string tab, int verbosity);
	void clear();
};

bool cycle(space start, space end, list<rule> prs);
list<rule> production_rule(list<instruction*> instrs, map<string, space> states, string tab, int verbosity);

size_t search_back(string s, size_t offset);
size_t search_front(string s, size_t offset);
list<size_t> state_variable_positions(space left, space right, string tab, int verbosity);
bool production_rule_check(string *raw, block *b, string tab, int verbosity);

#endif
