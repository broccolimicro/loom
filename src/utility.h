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

pair<string, instruction*> add_unique_variable(string prefix, string postfix, string type, map<string, keyword*> types, vspace *vars, string tab, int verbosity);
instruction *expand_instantiation(string chp, map<string, keyword*> types, vspace *vars, list<string> *input, string tab, int verbosity, bool allow_process);
size_t find_name(string subject, string search, size_t pos = 0);
string get_kind(string name, vspace *vars, map<string, keyword*> types);
string demorgan(string exp, bool invert);
string strip(string exp);
string distribute(string exp, string sib);

#endif
