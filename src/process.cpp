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

process::process(string chp, map<string, keyword*> types)
{
	parse(chp, types);
	_kind = "process";
}

process::~process()
{
	name = "";
	_kind = "process";

	map<string, variable*>::iterator i;
	for (i = io.begin(); i != io.end(); i++)
	{
		if (i->second != NULL)
			delete i->second;
		i->second = NULL;
	}

	io.clear();
}

process &process::operator=(process p)
{
	def = p.def;
	prs = p.prs;
	io = p.io;
	return *this;
}

void process::parse(string chp, map<string, keyword*> types)
{
	cout << "process! -> " << chp << endl;
	int name_start = chp.find_first_of(" ")+1;
	int name_end = chp.find_first_of("(");
	int input_start = chp.find_first_of("(")+1;
	int input_end = chp.find_first_of(")");
	int block_start = chp.find_first_of("{")+1;
	int block_end = chp.length()-1;
	string io_block;
	string::iterator i, j;

	map<string, variable*> vars;

	name = chp.substr(name_start, name_end - name_start);
	io_block = chp.substr(input_start, input_end - input_start);

	cout << "\tname!   -> "+name << endl;
	cout << "\tinputs! -> "+io_block << endl;

	for (i = io_block.begin(), j = io_block.begin(); i != io_block.end(); i++)
	{
		if (*(i+1) == ',' || i+1 == io_block.end())
		{
			vars = expand(io_block.substr(j-io_block.begin(), i+1 - j), types, "\t");
			io.insert(vars.begin(), vars.end());
			j = i+2;
		}
	}

	map<string, variable*>::iterator vi;
	for (vi = io.begin(); vi != io.end(); vi++)
		cout << *(vi->second) << endl;

	def.parse(chp.substr(block_start, block_end - block_start), types, io,  map<string, state>(), "\t");
}
