/*
 * expression.h
 *
 *  Created on: Mar 15, 2013
 *      Author: nbingham
 */


#include "../common.h"
#include "canonical.h"
#include "bdd.h"
#include "vspace.h"

#ifndef expression_h
#define expression_h

struct expression
{
	expression();
	expression(string e);
	expression(string e, vspace *v);
	expression(vector<minterm> t, vspace *v);
	~expression();

	string simple;
	vspace *vars;
	bool internal_vars;
	canonical expr;

	void gen_variables(string e);
	void gen_minterms(string e);
	void mccluskey();
	void gen_output();

	expression &operator()(string e);
	expression &operator()(vector<minterm> t, vspace *v);
};

template <class t>
t evaluate(string raw, vspace *vars, vector<t> values)
{
	int id;
	list<string> ops;
	list<string> ex;
	size_t p;

	p = find_first_of_l0(raw, "|");
	if (p != raw.npos)
		return evaluate(raw.substr(0, p), vars, values) | evaluate(raw.substr(p+1), vars, values);

	p = find_first_of_l0(raw, "&");
	if (p != raw.npos)
		return evaluate(raw.substr(0, p), vars, values) & evaluate(raw.substr(p+1), vars, values);

	p = find_first_of_l0(raw, "~");
	if (p != raw.npos)
		return ~ evaluate(raw.substr(p+1), vars, values);

	if (raw[0] == '(' && raw[raw.length()-1] == ')')
		return evaluate(raw.substr(1, raw.length()-2), vars, values);

	id = vars->get_uid(raw);
	if (id >= 0 && id < (int)values.size())
		return values[id];
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

list<string> extract_vars(string exp);
string remove_var(string exp, string var);


bdd solve(string raw, vspace *vars, string tab, int verbosity);
minterm estimate(string e, vspace *vars);

#endif
