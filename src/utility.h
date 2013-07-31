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

pair<string, instruction*> add_unique_variable(instruction *parent, string prefix, string postfix, string type, variable_space *vars, flag_space *flags);
instruction *expand_instantiation(instruction *parent, string chp, variable_space *vars, list<string> *input, flag_space *flags, bool allow_process);
size_t find_name(string subject, string search, size_t pos = 0);
string demorgan(string exp, int depth, bool invert);
string strip(string exp);
vector<string> distribute(string exp);
string flatten_slice(string slices);
string restrict_exp(string exp, string var, int val);
void gen_variables(string exp, variable_space *vars, flag_space *flags);
vector<string> extract_names(string exp);
vector<int> extract_ids(string exp, variable_space *vars);
string remove_var(string exp, string var);
string remove_comments(string str);
string remove_whitespace(string str);

#endif
