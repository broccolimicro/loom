/*
 * assignment.h
 *
 *  Created on: Jan 22, 2013
 *      Author: nbingham
 */

#ifndef assignment_h
#define assignment_h

#include "instruction.h"

struct assignment : instruction
{
	assignment();
	assignment(string chp, vspace *vars, string tab, int verbosity);
	~assignment();

	int uid;					// indexes into the state in the state space
	list<pair<string, string> > expr;

	assignment &operator=(assignment a);

	instruction *duplicate(vspace *vars, map<string, string> convert, string tab, int verbosity);

	void expand_shortcuts();
	void parse();
	int generate_states(graph *trans, int init);
	void generate_scribes();

	void print_hse();
};

instruction *expand_assignment(string chp, vspace *vars, string tab, int verbosity);
pair<string, instruction*> expand_expression(string chp, vspace *vars, string top, string tab, int verbosity);


/*
 *
 * TODO Add support for signed numbers (specifically a*-b should not be a*0-b)
 */
template <class t>
t evaluate(string raw, vspace *vars, vector<t> init, string tab, int verbosity)
{
	// TODO Bug with two character operators and sub string indices

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

	int id;
	list<string> ops;
	list<string> ex;
	size_t p;

	p = find_first_of_l0(raw, "|");
	if (p != raw.npos)
		return evaluate(raw.substr(0, p), vars, init, tab+"\t", verbosity) | evaluate(raw.substr(p+1), vars, init, tab+"\t", verbosity);

	p = find_first_of_l0(raw, "&");
	if (p != raw.npos)
		return evaluate(raw.substr(0, p), vars, init, tab+"\t", verbosity) & evaluate(raw.substr(p+1), vars, init, tab+"\t", verbosity);

	ops.clear();
	ops.push_back("==");
	ops.push_back("~=");
	p = find_first_of_l0(raw, ops);
	if (p != raw.npos && raw.substr(p, 2) == "==")
		return evaluate(raw.substr(0, p), vars, init, tab+"\t", verbosity) == evaluate(raw.substr(p+1), vars, init, tab+"\t", verbosity);
	else if (p != raw.npos && raw.substr(p, 2) == "!=")
		return evaluate(raw.substr(0, p), vars, init, tab+"\t", verbosity) != evaluate(raw.substr(p+1), vars, init, tab+"\t", verbosity);

	ops.clear();
	ops.push_back("<=");
	ops.push_back(">=");
	p = find_first_of_l0(raw, ops);
	if (p != raw.npos && raw.substr(p, 2) == "<=")
		return evaluate(raw.substr(0, p), vars, init, tab+"\t", verbosity) <= evaluate(raw.substr(p+1), vars, init, tab+"\t", verbosity);
	else if (p != raw.npos && raw.substr(p, 2) == ">=")
		return evaluate(raw.substr(0, p), vars, init, tab+"\t", verbosity) >= evaluate(raw.substr(p+1), vars, init, tab+"\t", verbosity);

	ops.clear();
	ops.push_back("<");
	ops.push_back(">");
	ex.clear();
	ex.push_back(">>");
	ex.push_back("<<");
	ex.push_back("<=");
	ex.push_back(">=");
	p = find_first_of_l0(raw, ops, 0, ex);
	if (p != raw.npos && raw[p] == '<')
		return evaluate(raw.substr(0, p), vars, init, tab+"\t", verbosity) < evaluate(raw.substr(p+1), vars, init, tab+"\t", verbosity);
	else if (p != raw.npos && raw[p] == '>')
		return evaluate(raw.substr(0, p), vars, init, tab+"\t", verbosity) > evaluate(raw.substr(p+1), vars, init, tab+"\t", verbosity);

	ops.clear();
	ops.push_back("<<");
	ops.push_back(">>");
	p = find_first_of_l0(raw, ops);
	if (p != raw.npos && raw.substr(p, 2) == "<<")
		return evaluate(raw.substr(0, p), vars, init, tab+"\t", verbosity) << evaluate(raw.substr(p+1), vars, init, tab+"\t", verbosity);
	else if (p != raw.npos && raw.substr(p, 2) == ">>")
		return evaluate(raw.substr(0, p), vars, init, tab+"\t", verbosity) >> evaluate(raw.substr(p+1), vars, init, tab+"\t", verbosity);

	p = find_first_of_l0(raw, "+-");
	if (p != raw.npos && raw[p] == '+')
		return evaluate(raw.substr(0, p), vars, init, tab+"\t", verbosity) + evaluate(raw.substr(p+1), vars, init, tab+"\t", verbosity);
	else if (p != raw.npos && raw[p] == '-')
		return evaluate(raw.substr(0, p), vars, init, tab+"\t", verbosity) - evaluate(raw.substr(p+1), vars, init, tab+"\t", verbosity);

	p = find_first_of_l0(raw, "*/");
	if (p != raw.npos && raw[p] == '*')
		return evaluate(raw.substr(0, p), vars, init, tab+"\t", verbosity) * evaluate(raw.substr(p+1), vars, init, tab+"\t", verbosity);
	else if (p != raw.npos && raw[p] == '/')
		return evaluate(raw.substr(0, p), vars, init, tab+"\t", verbosity) / evaluate(raw.substr(p+1), vars, init, tab+"\t", verbosity);

	p = find_first_of_l0(raw, "~");
	if (p != raw.npos)
		return ~ evaluate(raw.substr(p+1), vars, init, tab+"\t", verbosity);

	if (raw[0] == '(' && raw[raw.length()-1] == ')')
		return evaluate(raw.substr(1, raw.length()-2), vars, init, tab+"\t", verbosity);

	id = vars->get_uid(raw);
	if (id >= 0 && id < (int)init.size())
		return init[id];
	else if (raw.substr(0, 2) != "0x" && raw.substr(0, 2) != "0b" && raw.find_first_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_") != raw.npos)
		cout << "Error: Undefined variable " << raw << "." << endl;

	p = raw.find_first_of("bx");
	if (p != raw.npos && raw[p] == 'x')
		return t(hex_to_bin(raw.substr(p+1)));
	else if (p != raw.npos && raw[p] == 'b')
		return t(raw.substr(p+1));
	else
		return t(dec_to_bin(raw));
}

#endif
