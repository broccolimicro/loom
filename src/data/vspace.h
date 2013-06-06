/*
 * vspace.h
 *
 * A variable space collects information about all the variables in a given program.
 * It includes a list variables, as well as types and labels for those variables.
 * The concept behind vspace is to consolidate information about variables into one
 * easy to access place. vspace is primarily used in program.
 */

#ifndef vspace_h
#define vspace_h

#include "../common.h"
#include "../type/keyword.h"
#include "variable.h"

struct vspace
{
	map<string, keyword*> *types;
	map<string, variable> global;
	map<string, variable> label;

	int size();

	variable *find(int uid);
	variable *find(string name);
	keyword	 *find_type(string name);

	string get_name(int uid);
	string get_type(int uid);

	string get_type(string name);
	string get_kind(string name);
	string get_info(string name);
	int	   get_uid(string name);
	int    get_width(string name);

	vector<string> get_names();

	string unique_name(string prefix);

	bool vdef(string str);

	map<string, string> instantiate(string parent, bool parent_arg, vspace* s, bool arg);
	map<string, string> call(string parent, bool parent_arg, vspace* s);

	int insert(variable v);
	void clear();

	vspace &operator=(vspace s);

	void print(string t);
};

ostream &operator<<(ostream &os, vspace s);

#endif
