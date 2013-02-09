/*
 * utility.cpp
 *
 *  Created on: Feb 8, 2013
 *      Author: nbingham
 */

#include "utility.h"

instruction *expand_instantiation(string chp, map<string, keyword*> types, map<string, variable> *global, map<string, variable> *label, list<string> *input, string tab, int verbosity, bool allow_process)
{
	map<string, keyword*>::iterator var_type;
	map<string, variable>::iterator mem_var;
	variable v = variable(chp, global->size(), !allow_process, tab, verbosity);

	if (input != NULL)
		input->push_back(v.name);

	string name;

	if ((var_type = types.find(v.type)) != types.end())
	{
		if (v.type == "int")
			global->insert(pair<string, variable>(v.name, v));
		else
			label->insert(pair<string, variable>(v.name, v));

		if (var_type->second->kind() == "record" || var_type->second->kind() == "channel")
		{
			name = v.name;
			for (mem_var = ((record*)var_type->second)->globals.begin(); mem_var != ((record*)var_type->second)->globals.end(); mem_var++)
			{
				v = mem_var->second;

				if (v.type == "int")
					global->insert(pair<string, variable>(name + "." + v.name, variable(name + "." + v.name, global->size(), v.type, v.reset, v.width, !allow_process)));
				else
					label->insert(pair<string, variable>(name + "." + v.name, variable(name + "." + v.name, global->size(), v.type, v.reset, v.width, !allow_process)));
			}
		}
		else if (var_type->second->kind() == "process" || var_type->second->kind() == "operate")
		{
			if (allow_process)
			{
				cout << "Instantiating Process: " << v.type << " " << v.name << endl;

				map<string, string> convert;
				list<string>::iterator i, j;
				for (i = v.inputs.begin(), j = ((process*)var_type->second)->input.begin(); i != v.inputs.end() && j != ((process*)var_type->second)->input.end(); i++, j++)
					convert.insert(pair<string, string>(*j, *i));

				for (mem_var = ((process*)var_type->second)->global.begin(); mem_var != ((process*)var_type->second)->global.end(); mem_var++)
					if (!mem_var->second.io)
					{
						global->insert(pair<string, variable>(v.name + "." + mem_var->second.name, variable(mem_var->second.name, global->size(), mem_var->second.type, mem_var->second.reset, mem_var->second.width, mem_var->second.io)));
						convert.insert(pair<string, string>(mem_var->second.name, v.name + "." + mem_var->second.name));
					}
				for (mem_var = ((process*)var_type->second)->label.begin(); mem_var != ((process*)var_type->second)->label.end(); mem_var++)
					if (!mem_var->second.io)
					{
						label->insert(pair<string, variable>(v.name + "." + mem_var->second.name, variable(mem_var->second.name, global->size(), mem_var->second.type, mem_var->second.reset, mem_var->second.width, mem_var->second.io)));
						convert.insert(pair<string, string>(mem_var->second.name, v.name + "." + mem_var->second.name));
					}

				return ((process*)var_type->second)->def.duplicate(global, label, convert, tab, verbosity);
			}
			else
				cout << "Error: Invalid use of type " << var_type->second->kind() << " in record definition." << endl;
		}
	}
	else
		cout << "Error: Invalid typename: " << v.type << endl;

	return NULL;
}

pair<string, instruction*> add_unique_variable(string prefix, string postfix, string type, map<string, keyword*> types, map<string, variable> *global, map<string, variable> *label, string tab, int verbosity)
{
	int id = 0;
	while (global->find(prefix + to_string(id)) != global->end() || label->find(prefix + to_string(id)) != label->end())
		id++;

	return pair<string, instruction*>(prefix + to_string(id), expand_instantiation(type + " " + prefix + to_string(id) + postfix, types, global, label, NULL, tab, verbosity, true));
}

size_t find_name(string subject, string search, size_t pos)
{
	size_t ret = -1 + pos;
	bool alpha0, alpha1;

	do
	{
		ret = subject.find(search, ret + 1);
		alpha0 = ret > 0 && (nc(subject[ret-1]) || subject[ret-1] == '.');
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

string get_name(int uid, map<string, variable> *global, map<string, variable> *label)
{
	map<string, variable>::iterator i;
	for (i = global->begin(); i != global->end(); i++)
		if (i->second.uid == uid)
			return i->first;
	for (i = label->begin(); i != label->end(); i++)
		if (i->second.uid == uid)
			return i->first;
	return "";
}
