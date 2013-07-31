/*
 * variable_space.h
 *
 * A variable space collects information about all the variables in a given program.
 * It includes a list variables, as well as types and labels for those variables.
 * The concept behind variable_space is to consolidate information about variables into one
 * easy to access place. variable_space is primarily used in program.
 */

#ifndef variable_space_h
#define variable_space_h

#include "../common.h"
#include "../type/keyword.h"
#include "variable.h"

struct variable_space
{
	type_space *types;
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
	vector<string> get_driven();

	vector<string> get_names();
	vector<int> x_channel(vector<int> av);
	void x_channel(vector<int> av, map<int, int> *result);

	string unique_name(string prefix);

	bool vdef(string str);

	map<string, string> instantiate(string parent, bool parent_arg, variable_space* s, bool arg);
	map<string, string> call(string parent, bool parent_arg, variable_space* s);

	int insert(variable v);
	void clear();

	variable_space &operator=(variable_space s);

	void print(string t, ostream *fout = (&cout));
};

ostream &operator<<(ostream &os, variable_space s);

#endif
