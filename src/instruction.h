/*
 * instruction.h
 *
 *  Created on: Oct 23, 2012
 *      Author: Ned Bingham and Nicholas Kramer
 *
 *  DO NOT DISTRIBUTE
 */

#include "common.h"
#include "state.h"
#include "variable.h"
#include "keyword.h"
#include "rule.h"

#ifndef instruction_h
#define instruction_h

/* This structure describes an instruction in the chp program, namely what lies between
 * two semicolons in a block of. This has not been expanded to ;S1||S2; type of composition.
 */
struct instruction
{
protected:
	string _kind;

public:
	instruction();
	virtual ~instruction();


	// The raw CHP of this instruction.
	string chp;
	// A unique identifier to address this instruction.
	string uid;	// TODO assign this in state space generation

	/* the key is the name of the variable affected
	 * the value is the value of that variable at the end of the instruction
	 * 		the format of this value consists of 'i' or 'o' followed by n digits
	 * 		with possible values '0', '1', and 'X'.
	 */
	map<string, space> states;
	map<string, variable*>		local;
	map<string, variable*>		global;

	// This is the list of production rules that defines this instruction
	list<rule> rules;

	// For outputting debugging messages
	string tab;
	int verbosity;

	string kind();

	void print_state_space();
	void print_prs();

	virtual void expand_shortcuts() = 0;
	virtual void parse(map<string, keyword*> *types) = 0;
	virtual void generate_states(map<string, state> init) = 0;
	virtual void generate_prs(map<string, variable*> globals) = 0;
};

#endif
