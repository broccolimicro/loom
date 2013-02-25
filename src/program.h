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
	vector<rule>			prs_up;
	vector<rule>			prs_down;
	list<string>			errors;
	vspace					vars;
	state_space				space;
	graph					trans;
	parallel				*prgm;

	program &operator=(program p);



	void parse(string chp, int verbosity);
	void print_space_to_console();
	void print_space_graph_to_console();
	int conflict_count(state impl, int fire_uid, string fire_dir);
	void build_implicants(state_space diff_space);
	void merge_implicants();
	//void weaken_guard(rule pr);
	void print_prs();
};


void print_line(int from, graph *trans);
void print_line_dot(int from, state_space *spaces, graph *trans); // Print a line following .dot graphvis formatting
void print_line_with_trans(int from, graph *trans);
void print_diff_space_to_console(state_space diff_space);
state_space delta_space_gen(state_space spaces,graph trans);


#endif
