/*
 * instruction.h
 *
 *  Created on: Oct 23, 2012
 *      Author: Ned Bingham, Nicholas Kramer
 */

#include "common.h"

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
	instruction(string raw);
	~instruction();


	// The raw CHP of this instruction.
	string chp;
	/* the key is the name of the variable affected
	 * the value is the value of that variable at the end of the instruction
	 * 		the format of this value consists of 'i' or 'o' followed by n digits
	 * 		with possible values '0', '1', and 'X'.
	 */
	map<string, string> result;

	instruction &operator=(instruction i);
	string kind();

	void parse(string raw);
};

#endif
