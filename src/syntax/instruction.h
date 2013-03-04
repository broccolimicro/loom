/*
 * instruction.h
 *
 *  Created on: Oct 23, 2012
 *      Author: Ned Bingham and Nicholas Kramer
 *
 *  DO NOT DISTRIBUTE
 */

#ifndef instruction_h
#define instruction_h

#include "../common.h"
#include "../data.h"
#include "../type/keyword.h"

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

	// This is the list of production rules that defines this instruction
	list<rule> rules;

	// Some pointers for good use
	vspace *vars;

	// For outputting debugging messages
	string tab;
	int verbosity;

	string kind();

	void print_prs();

	virtual instruction *duplicate(vspace *vars, map<string, string> convert, string tab, int verbosity) = 0;

	virtual void expand_shortcuts() = 0;
	virtual void parse() = 0;
	virtual int generate_states(graph *trans, int init) = 0;
	virtual void generate_prs() = 0;

	virtual void print_hse() = 0;
};

#endif
