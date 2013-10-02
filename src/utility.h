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

pair<sstring, instruction*> add_unique_variable(instruction *parent, sstring prefix, sstring postfix, sstring type, variable_space *vars, flag_space *flags);
instruction *expand_instantiation(instruction *parent, sstring chp, variable_space *vars, list<sstring> *input, flag_space *flags, bool allow_process);
int find_name(sstring subject, sstring search, int pos = 0);
sstring demorgan(sstring exp, int depth, bool invert);
sstring strip(sstring exp);
svector<sstring> distribute(sstring exp);
sstring flatten_slice(sstring slices);
sstring restrict_exp(sstring exp, sstring var, int val);
void gen_variables(sstring exp, variable_space *vars, flag_space *flags);
svector<sstring> extract_names(sstring exp);
svector<int> extract_ids(sstring exp, variable_space *vars);
sstring remove_var(sstring exp, sstring var);
sstring remove_comments(sstring str);
sstring remove_whitespace(sstring str);

#endif
