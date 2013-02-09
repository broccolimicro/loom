/*
 * vspace.h
 *
 *  Created on: Feb 9, 2013
 *      Author: nbingham
 */

#ifndef vspace_h
#define vspace_h

#include "../common.h"
#include "variable.h"

struct vspace
{
	map<string, variable> global;
	map<string, variable> label;

	variable *find(int uid);
	variable *find(string name);

	string get_name(int uid);
	string get_type(int uid);

	string get_type(string name);
	int	   get_uid(string name);

	string unique_name(string prefix);
};

#endif
