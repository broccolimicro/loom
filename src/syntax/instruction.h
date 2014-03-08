/*
 * instruction.h
 *
 * TODO: Come up with a pithy way to describe this.
 */

#ifndef instruction_h
#define instruction_h

#include "../common.h"
#include "../data.h"
#include "../type/keyword.h"
#include "../flag_space.h"
#include "rule_space.h"

/* This structure describes an instruction in the chp program, namely what lies between
 * two semicolons in a sequential of. This has not been expanded to ;S1||S2; type of composition.
 */
struct instruction
{
protected:
	sstring _kind;

public:
	instruction();
	virtual ~instruction();


	// The raw CHP of this instruction.
	instruction *parent;
	svector<petri_index> from;
	svector<petri_index> uid;
	sstring chp;

	// Some pointers for good use
	variable_space *vars;
	rule_space *prs;
	petri_net  *net;

	// For outputting debugging messages
	flag_space *flags;

	sstring kind();

	virtual instruction *duplicate(instruction *parent, variable_space *vars, smap<sstring, sstring> convert) = 0;

	virtual void expand_shortcuts() = 0;
	virtual void parse() = 0;
	virtual void simulate() = 0;
	virtual void rewrite() = 0;
	virtual void reorder() = 0;
	virtual svector<petri_index> generate_states(petri_net *n, rule_space *p, svector<petri_index> f, smap<int, int> pbranch, smap<int, int> cbranch) = 0;

	virtual void print_hse(sstring t, ostream *fout = &cout) = 0;

	pair<sstring, instruction*> expand_expression(sstring expr, sstring top);
};

#endif
