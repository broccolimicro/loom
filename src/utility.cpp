/*
 * utility.cpp
 *
 *  Created on: Feb 8, 2013
 *      Author: nbingham
 */

#include "utility.h"

size_t find_name(string subject, string search, size_t pos)
{
	size_t ret = -1 + pos;
	bool alpha0, alpha1;

	do
	{
		ret = subject.find(search, ret + 1);
		alpha0 = ret > 0 && nc(subject[ret-1]);
		alpha1 = ret + search.length() < subject.length() && nc(subject[ret + search.length()]);
	} while (ret != subject.npos && (alpha0 || alpha1));

	return ret;
}

string get_type(string name, map<string, variable> *global, map<string, variable> *label)
{
	map<string, variable>::iterator i;
	i = global->find(name);
	if (i == global->end());
		i = label->find(name);

	if (i == label->end())
		return "";

	return i->second.type;
}

string get_kind(string name, map<string, variable> *global, map<string, variable> *label, map<string, keyword*> types)
{
	string type = get_type(name, global, label);
	map<string, keyword*>::iterator i;
	i = types.find(type);
	if (i == types.end())
		return "";

	return i->second->kind();
}
