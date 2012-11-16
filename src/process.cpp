/*
 * process.cpp
 *
 *  Created on: Oct 29, 2012
 *      Author: Ned Bingham
 */

#include "process.h"
#include "record.h"
#include "common.h"

process::process()
{
	name = "";
	_kind = "process";
}

process::process(string chp, map<string, keyword*> types, map<string, variable*> vars)
{
	parse(chp, types, vars);
	_kind = "process";
}

process::~process()
{
	name = "";
	_kind = "process";

	map<string, variable*>::iterator i;
	for (i = local.begin(); i != local.end(); i++)
	{
		if (i->second != NULL)
			delete i->second;
		i->second = NULL;
	}

	local.clear();
	global.clear();
}

process &process::operator=(process p)
{
	def = p.def;
	prs = p.prs;
	local = p.local;
	global = p.global;
	return *this;
}

void process::parse(string chp, map<string, keyword*> types, map<string, variable*> vars)
{
	global = vars;
	cout << "process! -> " << chp << endl;
	int name_start = chp.find_first_of(" ")+1;
	int name_end = chp.find_first_of("(");
	int input_start = chp.find_first_of("(")+1;
	int input_end = chp.find_first_of(")");
	int block_start = chp.find_first_of("{")+1;
	int block_end = chp.length()-1;
	string io_block;
	string::iterator i, j;

	map<string, variable*> temp;

	name = chp.substr(name_start, name_end - name_start);
	io_block = chp.substr(input_start, input_end - input_start);

	cout << "\tname!   -> "+name << endl;
	cout << "\tinputs! -> "+io_block << endl;

	for (i = io_block.begin(), j = io_block.begin(); i != io_block.end(); i++)
	{
		if (*(i+1) == ',' || i+1 == io_block.end())
		{
			temp = expand(io_block.substr(j-io_block.begin(), i+1 - j), types, "\t");
			local.insert(temp.begin(), temp.end());
			global.insert(temp.begin(), temp.end());
			j = i+2;
		}
	}

	map<string, variable*>::iterator vi;
	for (vi = local.begin(); vi != local.end(); vi++)
		cout << *(vi->second) << endl;

	def.parse(chp.substr(block_start, block_end - block_start), types, global,  map<string, state>(), "\t");
}
