/*
 * process.cpp
 *
 *  Created on: Oct 29, 2012
 *      Author: Ned Bingham and Nicholas Kramer
 *
 *  DO NOT DISTRIBUTE
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

process::process(string raw, map<string, keyword*> types, map<string, variable> vars, int verbosity)
{
	parse(raw, types, vars, verbosity);
	_kind = "process";
}

process::~process()
{
	name = "";
	_kind = "process";

	global.clear();
}

process &process::operator=(process p)
{
	def = p.def;
	prs = p.prs;
	global = p.global;
	return *this;
}

void process::parse(string raw, map<string, keyword*> types, map<string, variable> vars, int verbosity)
{
	chp = raw;

	size_t name_start = chp.find_first_of(" ")+1;
	size_t name_end = chp.find_first_of("(");
	size_t input_start = chp.find_first_of("(")+1;
	size_t input_end = chp.find_first_of(")");
	size_t block_start = chp.find_first_of("{")+1;
	size_t block_end = chp.length()-1;
	size_t comm, comm_s, comm_e, var;
	string io_block;
	string def_block;
	string::iterator i, j;

	map<string, variable> temp;
	map<string, variable>::iterator vi, vj;
	map<string, keyword*>::iterator ti;

	cout << "Process:\t" << chp << endl;

	global.insert(vars.begin(), vars.end());

	name = chp.substr(name_start, name_end - name_start);
	io_block = chp.substr(input_start, input_end - input_start);

	if (verbosity >= VERB_PARSE)
	{
		cout << "\tName:\t" << name << endl;
		cout << "\tInputs:\t" << io_block << endl;
	}

	for (i = io_block.begin(), j = io_block.begin(); i != io_block.end(); i++)
	{
		if (*(i+1) == ',' || i+1 == io_block.end())
		{
			temp = expand(io_block.substr(j-io_block.begin(), i+1 - j), "", types, "\t", verbosity);
			for (vi = temp.begin(); vi != temp.end(); vi++)
			{
				vi->second.uid = global.size();
				global.insert(*vi);
			}
			j = i+2;
		}
	}

	string right, left, replace;
	int skip;
	def_block = chp.substr(block_start, block_end - block_start);
	while ((comm = def_block.find_first_of("@?!")) != def_block.npos)
	{
		comm_s = def_block.find_last_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789._", comm-1);
		comm_e = def_block.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789._", comm+1);
		left = def_block.substr(comm_s+1, comm - comm_s - 1);
		right = def_block.substr(comm+1, comm_e - comm - 1);

		if ((vi = global.find(left)) != global.end())
		{
			if ((ti = types.find(vi->second.type)) != types.end())
			{
				if (ti->second != NULL)
				{
					if (ti->second->kind() == "channel")
					{
						if (def_block[comm] == '!')
							replace = ((channel*)(ti->second))->send.def.chp;
						else if (def_block[comm] == '?')
							replace = ((channel*)(ti->second))->recv.def.chp;
						//else if (def_block[comm] == '@')
						//	def_block = def_block.substr(0, comm_s+1) + ((channel*)(ti->second))->probe.def.chp + def_block.substr(comm_e);
						else
						{
							cout << "Error: Unknown operation " << def_block[comm] << endl;
							break;
						}

						for (vj = ((channel*)(ti->second))->vars.begin(); vj != ((channel*)(ti->second))->vars.end(); vj++)
						{
							var = replace.find(vj->first);
							while (var != replace.npos)
							{
								if (replace[var + vj->first.length()] != '.' && replace[((var - 1) < 0) ? var : var - 1] != '.')
								{
									replace = replace.substr(0, var) + vi->first + "." + replace.substr(var);
									skip = vj->first.length() + vi->first.length() + 1;
								}
								else
									skip = vj->first.length();

								var = replace.find(vj->first, var + skip);
							}
						}

						def_block = def_block.substr(0, comm_s+1) + replace + def_block.substr(comm_e);
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
				cout << "Error: Undefined type " << vi->second.type << endl;
				break;
			}
		}
		else
		{
			cout << "Error: Undefined channel " << left << endl;
			break;
		}
	}

	def.init(def_block, types, &global, "\t", verbosity);
	cout << "Generating State Space" << endl;
	def.generate_states(&space, &trans, -1);
}
