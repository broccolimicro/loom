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
#include "canonical.h"

struct variable_space
{
	variable_space();
	~variable_space();

	type_space *types;
	smap<sstring, variable> global;
	smap<sstring, variable> label;
	svector<logic> requirements;
	logic enforcements;
	logic reset;

	int size();

	variable *find(int uid);
	variable *find(sstring name);
	keyword	 *find_type(sstring name);

	sstring get_name(int uid);
	sstring get_type(int uid);

	sstring get_type(sstring name);
	sstring get_kind(sstring name);
	sstring get_info(sstring name);
	int	   get_uid(sstring name);
	int    get_width(sstring name);
	svector<sstring> get_driven();

	svector<sstring> get_names();
	bool part_of_channel(int uid);
	svector<int> x_channel(svector<int> av);
	void x_channel(svector<int> av, smap<int, logic> *result);

	sstring unique_name(sstring prefix);
	sstring invert_name(sstring name);

	bool vdef(sstring str);

	smap<sstring, sstring> instantiate(sstring parent, bool parent_arg, variable_space* s, bool arg);
	smap<sstring, sstring> call(sstring parent, bool parent_arg, variable_space* s);

	int insert(variable v);
	void clear();

	logic increment_pcs(logic t, bool active);

	variable_space &operator=(variable_space s);

	void print(sstring t, ostream *fout = (&cout));
};

ostream &operator<<(ostream &os, variable_space s);

#endif
