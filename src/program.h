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

/* This structure describes a whole program. It contains a record of all
 * of the types described in this program and all of the global variables
 * defined in this program. It also contains a list of all of the errors
 * produced during the compilation and a list of all of the production rules
 * that result from this compilation. This is the top level of the compiler,
 * if you will. It contains all the information used throughout the entire
 * program's compilation. */

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
	void print_dot();
	void print_petrify();
	void print_prs();
};

#endif
