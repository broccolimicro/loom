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
	int verbosity;

	program &operator=(program p);

	void parse(string chp, int verbosity);
	void merge();
	void project();
	void decompose();
	void reshuffle();


	void generate_states();
	void insert_state_vars();
	void generate_prs();
	void factor_prs();

	void print_hse();
	void print_prs();
	void print_TS();
};

#endif
