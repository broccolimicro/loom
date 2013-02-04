/*
 * assignment.h
 *
 *  Created on: Jan 22, 2013
 *      Author: nbingham
 */

#include "common.h"
#include "state.h"
#include "variable.h"
#include "keyword.h"
#include "rule.h"
#include "instruction.h"
#include "graph.h"

#ifndef assignment_h
#define assignment_h

struct assignment : instruction
{
	assignment();
	assignment(string chp, map<string, keyword*> types, map<string, variable> *globals, map<string, variable> *label, string tab, int verbosity);
	~assignment();

	int uid;					// indexes into the state in the state space
	list<pair<string, string> > expr;

	assignment &operator=(assignment a);

	instruction *duplicate(map<string, variable> *globals, map<string, variable> *labels, map<string, string> convert);

	void expand_shortcuts();
	void parse(map<string, keyword*> types);
	int generate_states(state_space *space, graph *trans, int init);
	void generate_prs();
};

/*
 *
 * TODO Add support for signed numbers (specifically a*-b should not be a*0-b)
 * TODO Add support for less than and greater than operators
 */
template <class t>
t evaluate(string raw, map<string, variable> *globals, vector<t> init, string tab, int verbosity)
{
	// Tested to be fairly functional:
	// Adds, subtracts
	// Multiplies
	// Comparisons
	// Equality Tests
	// Ands and Ors
	// Variables
	// Parens

	// Weakest binding set, descending into strongest binding.
	if (verbosity >= VERB_PARSE)
		cout << tab << "Evaluate: " << raw << endl;

	typename map<string, variable>::iterator v;
	list<string> ops;
	size_t p;

	p = find_first_of_l0(raw, "|");
	if (p != raw.npos)
		return evaluate(raw.substr(0, p-1), globals, init, tab+"\t", verbosity) | evaluate(raw.substr(p), globals, init, tab+"\t", verbosity);

	p = find_first_of_l0(raw, "&");
	if (p != raw.npos)
		return evaluate(raw.substr(0, p-1), globals, init, tab+"\t", verbosity) & evaluate(raw.substr(p), globals, init, tab+"\t", verbosity);

	ops.clear();
	ops.push_back("==");
	ops.push_back("~=");
	p = find_first_of_l0(raw, ops);
	if (p != raw.npos && raw.substr(p, 2) == "==")
		return evaluate(raw.substr(0, p-1), globals, init, tab+"\t", verbosity) == evaluate(raw.substr(p), globals, init, tab+"\t", verbosity);
	else if (p != raw.npos && raw.substr(p, 2) == "!=")
		return evaluate(raw.substr(0, p-1), globals, init, tab+"\t", verbosity) != evaluate(raw.substr(p), globals, init, tab+"\t", verbosity);

	ops.clear();
	ops.push_back("<=");
	ops.push_back(">=");
	p = find_first_of_l0(raw, ops);
	if (p != raw.npos && raw.substr(p, 2) == "<=")
		return evaluate(raw.substr(0, p-1), globals, init, tab+"\t", verbosity) <= evaluate(raw.substr(p), globals, init, tab+"\t", verbosity);
	else if (p != raw.npos && raw.substr(p, 2) == ">=")
		return evaluate(raw.substr(0, p-1), globals, init, tab+"\t", verbosity) >= evaluate(raw.substr(p), globals, init, tab+"\t", verbosity);

	// TODO Add support for greater than and less than operators

	ops.clear();
	ops.push_back("<<");
	ops.push_back(">>");
	p = find_first_of_l0(raw, ops);
	if (p != raw.npos && raw.substr(p, 2) == "<<")
		return evaluate(raw.substr(0, p-1), globals, init, tab+"\t", verbosity) << evaluate(raw.substr(p), globals, init, tab+"\t", verbosity);
	else if (p != raw.npos && raw.substr(p, 2) == ">>")
		return evaluate(raw.substr(0, p-1), globals, init, tab+"\t", verbosity) >> evaluate(raw.substr(p), globals, init, tab+"\t", verbosity);

	p = find_first_of_l0(raw, "+-");
	if (p != raw.npos && raw[p] == '+')
		return evaluate(raw.substr(0, p-1), globals, init, tab+"\t", verbosity) + evaluate(raw.substr(p), globals, init, tab+"\t", verbosity);
	else if (p != raw.npos && raw[p] == '-')
		return evaluate(raw.substr(0, p-1), globals, init, tab+"\t", verbosity) - evaluate(raw.substr(p), globals, init, tab+"\t", verbosity);

	p = find_first_of_l0(raw, "*/");
	if (p != raw.npos && raw[p] == '*')
		return evaluate(raw.substr(0, p-1), globals, init, tab+"\t", verbosity) * evaluate(raw.substr(p), globals, init, tab+"\t", verbosity);
	else if (p != raw.npos && raw[p] == '/')
		return evaluate(raw.substr(0, p-1), globals, init, tab+"\t", verbosity) / evaluate(raw.substr(p), globals, init, tab+"\t", verbosity);

	p = find_first_of_l0(raw, "~");
	if (p != raw.npos)
		return ~ evaluate(raw.substr(p), globals, init, tab+"\t", verbosity);

	if (raw[0] == '(' && raw[raw.length()-1] == ')')
		return evaluate(raw.substr(1, raw.length()-2), globals, init, tab+"\t", verbosity);

	v = globals->find(raw);
	if (v != globals->end() && v->second.uid < (int)init.size())
		return init[v->second.uid];
	else if (v != globals->end())
		cout << "Error: Undefined variable " << raw << "." << endl;

	p = raw.find_first_of("bx");
	if (p != raw.npos && raw[p] == 'x')
		return t(hex_to_bin(raw.substr(p)));
	else if (p != raw.npos && raw[p] == 'b')
		return t(raw.substr(p));
	else
		return t(dec_to_bin(raw));
}

#endif
