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
	vspace *vars;
	petri  *net;

	// For outputting debugging messages
	string tab;
	int verbosity;

	string kind();

	virtual instruction *duplicate(instruction *parent, vspace *vars, map<string, string> convert, string tab, int verbosity) = 0;
	virtual vector<int> variant() = 0;
	virtual vector<int> active_variant() = 0;
	virtual vector<int> passive_variant() = 0;

	virtual void expand_shortcuts() = 0;
	virtual void parse() = 0;
	virtual void merge() = 0;
	virtual vector<int> generate_states(petri *n, vector<int> f, map<int, int> pbranch, map<int, int> cbranch) = 0;

	virtual void insert_instr(int uid, int nid, instruction *instr) = 0;

	virtual void print_hse(string t, ostream *fout = &cout) = 0;

	pair<string, instruction*> expand_expression(string expr, string top);
};

#endif
