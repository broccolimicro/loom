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

pair<string, instruction*> add_unique_variable(string prefix, string postfix, string type, map<string, keyword*> types, map<string, variable> *global, map<string, variable> *label, string tab, int verbosity);
instruction *expand_instantiation(string chp, map<string, keyword*> types, map<string, variable> *global, map<string, variable> *label, list<string> *input, string tab, int verbosity, bool allow_process);
size_t find_name(string subject, string search, size_t pos = 0);
string get_type(string name, map<string, variable> *global, map<string, variable> *label);
string get_kind(string name, map<string, variable> *global, map<string, variable> *label, map<string, keyword*> types);
string get_name(int uid, map<string, variable> *global, map<string, variable> *label);

#endif
