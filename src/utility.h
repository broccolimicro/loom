/*
 * utility.h
 *
 *  Created on: Feb 8, 2013
 *      Author: nbingham
 */

#ifndef utility_h
#define utility_h

#include "common.h"
#include "type.h"
#include "data/variable.h"

pair<string, instruction*> add_unique_variable(instruction *parent, string prefix, string postfix, string type, vspace *vars, string tab, int verbosity);
instruction *expand_instantiation(instruction *parent, string chp, vspace *vars, list<string> *input, string tab, int verbosity, bool allow_process);
size_t find_name(string subject, string search, size_t pos = 0);
string demorgan(string exp, int depth, bool invert);
string strip(string exp);
vector<string> distribute(string exp);
string flatten_slice(string slices);
string restrict_exp(string exp, string var, int val);
void gen_variables(string exp, vspace *vars);
vector<string> extract_names(string exp);
vector<int> extract_ids(string exp, vspace *vars);
string remove_var(string exp, string var);
string remove_comments(string str);
string remove_whitespace(string str);

#endif
