/*
 * vspace.cpp
 *
 *  Created on: Feb 9, 2013
 *      Author: nbingham
 */

#include "vspace.h"

variable *vspace::find(int uid)
{
	map<string, variable>::iterator i;
	for (i = global.begin(); i != global.end(); i++)
		if (i->second.uid == uid)
			return &(i->second);
	for (i = label.begin(); i != label.end(); i++)
		if (i->second.uid == uid)
			return &(i->second);
	return NULL;
}

variable *vspace::find(string name)
{
	map<string, variable>::iterator i;
	i = global.find(name);
	if (i == global.end());
		i = label.find(name);

	if (i == label.end())
		return NULL;

	return &(i->second);
}

string vspace::get_name(int uid)
{
	map<string, variable>::iterator i;
	for (i = global.begin(); i != global.end(); i++)
		if (i->second.uid == uid)
			return i->second.name;
	for (i = label.begin(); i != label.end(); i++)
		if (i->second.uid == uid)
			return i->second.name;
	return "";
}

string vspace::get_type(int uid)
{
	map<string, variable>::iterator i;
	for (i = global.begin(); i != global.end(); i++)
		if (i->second.uid == uid)
			return i->second.type;
	for (i = label.begin(); i != label.end(); i++)
		if (i->second.uid == uid)
			return i->second.type;
	return "";
}

string vspace::get_type(string name)
{
	map<string, variable>::iterator i;
	i = global.find(name);
	if (i == global.end());
		i = label.find(name);

	if (i == label.end())
		return "";

	return i->second.type;
}

int vspace::get_uid(string name)
{
	map<string, variable>::iterator i;
	i = global.find(name);
	if (i == global.end());
		i = label.find(name);

	if (i == label.end())
		return -1;

	return i->second.uid;
}

string vspace::unique_name(string prefix)
{
	int id = 0;
	while (global.find(prefix + to_string(id)) != global.end() || label.find(prefix + to_string(id)) != label.end())
		id++;

	return prefix + to_string(id);
}
