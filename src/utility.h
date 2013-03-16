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
string distribute(string exp, string sib);
string flatten_slice(string slices);

#endif
