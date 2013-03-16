/*
 * graph.h
 *
 */

#include "common.h"
#include "type.h"
#include "syntax.h"
#include "data.h"
#include "utility.h"

#ifndef program_h
#define program_h


struct program
{
	program();

	program(string chp, int verbosity);

	~program();

	map<string, keyword*>	type_space;
	vector<rule>			prs;
	list<string>			errors;
	vspace					vars;
	graph					space;
	parallel				*prgm;

	program &operator=(program p);

	void parse(string chp, int verbosity);
	void generate_states();
	void insert_state_vars();
	void generate_prs();
	void factor_prs();
	void print_prs();
};

#endif
