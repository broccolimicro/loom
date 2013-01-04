/*
 * instruction.h
 *
 *  Created on: Oct 23, 2012
 *      Author: Ned Bingham and Nicholas Kramer
 *
 *  DO NOT DISTRIBUTE
 */

#include "common.h"
#include "state.h"
#include "variable.h"
#include "keyword.h"
#include "rule.h"

#ifndef instruction_h
#define instruction_h

/* This structure describes an instruction in the chp program, namely what lies between
 * two semicolons in a block of. This has not been expanded to ;S1||S2; type of composition.
 */
struct instruction
{
protected:
	string _kind;

public:
	instruction();
	instruction(string id, string raw, map<string, keyword*> types, map<string, variable*> vars, map<string, state> init, string tab, int verbosity);
	~instruction();


	// The raw CHP of this instruction.
	string chp;
	/* the key is the name of the variable affected
	 * the value is the value of that variable at the end of the instruction
	 * 		the format of this value consists of 'i' or 'o' followed by n digits
	 * 		with possible values '0', '1', and 'X'.
	 */
	map<string, state> result;

	// This is the list of production rules that defines this instruction
	list<rule>		rules;
	string			uid;

	instruction &operator=(instruction i);
	string kind();

	void parse(string id, string raw, map<string, keyword*> types, map<string, variable*> vars, map<string, state> init, string tab, int verbosity);
};

list<rule> production_rule(map<string, state> previous, map<string, state> next, map<string, variable*> globals, string tab, int verbosity);

/*
 *
 * TODO Add support for signed numbers (specifically a*-b should not be a*0-b)
 * TODO Add support for less than and greater than operators
 */
template <class t>
t expression(string raw, map<string, t> init, string tab, int verbosity)
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
		cout << tab << "Expression: " << raw << endl;

	typename map<string, t>::iterator v;
	list<string> ops;
	size_t p;

	p = find_first_of_l0(raw, "|");
	if (p != raw.npos)
		return expression(raw.substr(0, p-1), init, tab+"\t", verbosity) | expression(raw.substr(p), init, tab+"\t", verbosity);

	p = find_first_of_l0(raw, "&");
	if (p != raw.npos)
		return expression(raw.substr(0, p-1), init, tab+"\t", verbosity) & expression(raw.substr(p), init, tab+"\t", verbosity);

	ops.clear();
	ops.push_back("==");
	ops.push_back("~=");
	p = find_first_of_l0(raw, ops);
	if (p != raw.npos && raw.substr(p, 2) == "==")
		return expression(raw.substr(0, p-1), init, tab+"\t", verbosity) == expression(raw.substr(p), init, tab+"\t", verbosity);
	else if (p != raw.npos && raw.substr(p, 2) == "!=")
		return expression(raw.substr(0, p-1), init, tab+"\t", verbosity) != expression(raw.substr(p), init, tab+"\t", verbosity);

	ops.clear();
	ops.push_back("<=");
	ops.push_back(">=");
	p = find_first_of_l0(raw, ops);
	if (p != raw.npos && raw.substr(p, 2) == "<=")
		return expression(raw.substr(0, p-1), init, tab+"\t", verbosity) <= expression(raw.substr(p), init, tab+"\t", verbosity);
	else if (p != raw.npos && raw.substr(p, 2) == ">=")
		return expression(raw.substr(0, p-1), init, tab+"\t", verbosity) >= expression(raw.substr(p), init, tab+"\t", verbosity);

	// TODO Add support for greater than and less than operators

	ops.clear();
	ops.push_back("<<");
	ops.push_back(">>");
	p = find_first_of_l0(raw, ops);
	if (p != raw.npos && raw.substr(p, 2) == "<<")
		return expression(raw.substr(0, p-1), init, tab+"\t", verbosity) << expression(raw.substr(p), init, tab+"\t", verbosity);
	else if (p != raw.npos && raw.substr(p, 2) == ">>")
		return expression(raw.substr(0, p-1), init, tab+"\t", verbosity) >> expression(raw.substr(p), init, tab+"\t", verbosity);

	p = find_first_of_l0(raw, "+-");
	if (p != raw.npos && raw[p] == '+')
		return expression(raw.substr(0, p-1), init, tab+"\t", verbosity) + expression(raw.substr(p), init, tab+"\t", verbosity);
	else if (p != raw.npos && raw[p] == '-')
		return expression(raw.substr(0, p-1), init, tab+"\t", verbosity) - expression(raw.substr(p), init, tab+"\t", verbosity);

	p = find_first_of_l0(raw, "*/");
	if (p != raw.npos && raw[p] == '*')
		return expression(raw.substr(0, p-1), init, tab+"\t", verbosity) * expression(raw.substr(p), init, tab+"\t", verbosity);
	else if (p != raw.npos && raw[p] == '/')
		return expression(raw.substr(0, p-1), init, tab+"\t", verbosity) / expression(raw.substr(p), init, tab+"\t", verbosity);

	p = find_first_of_l0(raw, "~");
	if (p != raw.npos)
		return ~ expression(raw.substr(p), init, tab+"\t", verbosity);

	if (raw[0] == '(' && raw[raw.length()-1] == ')')
		return expression(raw.substr(1, raw.length()-2), init, tab+"\t", verbosity);

	v = init.find(raw);
	if (v != init.end())
		return v->second;

	p = raw.find_first_of("bx");
	if (p != raw.npos && raw[p] == 'x')
		return t(hex_to_bin(raw.substr(p)));
	else if (p != raw.npos && raw[p] == 'b')
		return t(raw.substr(p));
	else
		return t(dec_to_bin(raw));
}


#endif
