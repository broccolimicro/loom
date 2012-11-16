/*
 * process.cpp
 *
 *  Created on: Oct 29, 2012
 *      Author: Ned Bingham
 */

#include "process.h"
#include "record.h"
#include "common.h"
#include "channel.h"

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
	size_t name_start = chp.find_first_of(" ")+1;
	size_t name_end = chp.find_first_of("(");
	size_t input_start = chp.find_first_of("(")+1;
	size_t input_end = chp.find_first_of(")");
	size_t block_start = chp.find_first_of("{")+1;
	size_t block_end = chp.length()-1;
	size_t comm, comm_s, comm_e;
	string io_block;
	string def_block;
	string::iterator i, j;

	map<string, variable*> temp;
	map<string, keyword*>::iterator ti;

	name = chp.substr(name_start, name_end - name_start);
	io_block = chp.substr(input_start, input_end - input_start);

	cout << "\tname!   -> "+name << endl;
	cout << "\tinputs! -> "+io_block << endl;

	for (i = io_block.begin(), j = io_block.begin(); i != io_block.end(); i++)
	{
		if (*(i+1) == ',' || i+1 == io_block.end())
		{
			temp = expand(io_block.substr(j-io_block.begin(), i+1 - j), "", types, "\t");
			local.insert(temp.begin(), temp.end());
			global.insert(temp.begin(), temp.end());
			j = i+2;
		}
	}

	map<string, variable*>::iterator vi;
	//for (vi = local.begin(); vi != local.end(); vi++)
	//	cout << *(vi->second) << endl;

	string right, left;
	def_block = chp.substr(block_start, block_end - block_start);
	while ((comm = def_block.find_first_of("@?!")) != def_block.npos)
	{
		comm_s = def_block.find_last_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789._", comm-1);
		comm_e = def_block.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789._", comm+1);
		left = def_block.substr(comm_s+1, comm - comm_s - 1);
		right = def_block.substr(comm+1, comm_e - comm - 1);

		if ((vi = global.find(left)) != global.end())
		{
			if ((ti = types.find(vi->second->type)) != types.end())
			{
				if (ti->second != NULL)
				{
					if (ti->second->kind() == "channel")
					{
						if (def_block[comm] == '!')
							def_block = def_block.substr(0, comm_s+1) + ((channel*)(ti->second))->send.def.chp + def_block.substr(comm_e);
						else if (def_block[comm] == '?')
							def_block = def_block.substr(0, comm_s+1) + ((channel*)(ti->second))->recv.def.chp + def_block.substr(comm_e);
						//else if (def_block[comm] == '@')
						//	def_block = def_block.substr(0, comm_s+1) + ((channel*)(ti->second))->probe.def.chp + def_block.substr(comm_e);
						else
						{
							cout << "Error: Unknown operation " << def_block[comm] << endl;
							break;
						}
					}
					else
					{
						cout << "Error: Invalid use of " << ti->second->kind() << " type" << endl;
						break;
					}
				}
				else
				{
					cout << "Error: Internal" << endl;
					break;
				}
			}
			else
			{
				cout << "Error: Undefined type " << vi->second->type << endl;
				break;
			}
		}
		else
		{
			cout << "Error: Undefined channel " << left << endl;
			break;
		}
	}

	def.parse(def_block, types, global,  map<string, state>(), "\t");
}
