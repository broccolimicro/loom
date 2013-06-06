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
	pids from;
	pids uid;
	string chp;

	// Some pointers for good use
	vspace *vars;
	petri  *net;

	// For outputting debugging messages
	string tab;
	int verbosity;

	string kind();

	virtual instruction *duplicate(instruction *parent, vspace *vars, map<string, string> convert, string tab, int verbosity) = 0;
	virtual minterm variant() = 0;
	virtual minterm active_variant() = 0;
	virtual minterm passive_variant() = 0;

	virtual void expand_shortcuts() = 0;
	virtual void parse() = 0;
	virtual void merge() = 0;
	virtual pids generate_states(petri *n, pids f, bids b, minterm filter) = 0;
	virtual place simulate_states(place init, minterm filter) = 0;

	virtual void insert_instr(int uid, int nid, instruction *instr) = 0;

	virtual void print_hse(string t, ostream *fout = &cout) = 0;
};

#endif
