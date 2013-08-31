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
	string _kind;

public:
	instruction();
	virtual ~instruction();


	// The raw CHP of this instruction.
	instruction *parent;
	vector<int> from;
	vector<int> uid;
	string chp;

	// Some pointers for good use
	variable_space *vars;
	rule_space *prs;
	petri  *net;

	// For outputting debugging messages
	flag_space *flags;

	string kind();

	virtual instruction *duplicate(instruction *parent, variable_space *vars, map<string, string> convert) = 0;

	virtual void expand_shortcuts() = 0;
	virtual void parse() = 0;
	virtual void simulate() = 0;
	virtual void rewrite() = 0;
	virtual void reorder() = 0;
	virtual vector<int> generate_states(petri *n, rule_space *p, vector<int> f, map<int, int> pbranch, map<int, int> cbranch) = 0;

	virtual void print_hse(string t, ostream *fout = &cout) = 0;

	pair<string, instruction*> expand_expression(string expr, string top);
};

#endif
