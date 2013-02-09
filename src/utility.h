/*
 * utility.h
 *
 *  Created on: Feb 8, 2013
 *      Author: nbingham
 */

#include "common.h"
#include "variable.h"
#include "keyword.h"

#ifndef utility_h
#define utility_h

size_t find_name(string subject, string search, size_t pos = 0);
string get_type(string name, map<string, variable> *global, map<string, variable> *label);
string get_kind(string name, map<string, variable> *global, map<string, variable> *label, map<string, keyword*> types);
string get_name(int uid, map<string, variable> *global, map<string, variable> *label);

#endif
