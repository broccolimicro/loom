/*
 * record.h
 *
 *  Created on: Oct 23, 2012
 *      Author: Ned Bingham
 */

#include "common.h"
#include "keyword.h"

#ifndef record_h
#define record_h

/* This structure represents a structure or record. A record
 * contains a bunch of member variables that help you index
 * segments of bits within the multibit signal.
 */
struct record : keyword
{
	record()
	{
		name = "";
	}
	record(string chp)
	{
		parse(chp);
	}
	~record()
	{
		name = "";
	}

	map<string, variable> vars;	// the list of member variables that make up this record

	record &operator=(record r)
	{
		vars = r.vars;
		return *this;
	}

	void parse(string chp)
	{
		cout << "record! -> " << chp << endl;
		int name_start = chp.find_first_of(" ")+1;
		int name_end = chp.find_first_of("{");
		int block_start = chp.find_first_of("{")+1;
		int block_end = chp.length()-1;
		string::iterator i, j;
		string io_block;

		variable v;

		name = chp.substr(name_start, name_end - name_start);
		io_block = chp.substr(block_start, block_end - block_start);

		cout << "\tname!   -> " << name << endl;
		cout << "\tblock!  -> " << io_block << endl;

		for (i = io_block.begin(), j = io_block.begin(); i != io_block.end(); i++)
		{
			if (*(i+1) == ';')
			{
				v.parse(io_block.substr(j-io_block.begin(), i+1 - j));
				vars.insert(pair<string, variable>(name, v));
				j = i+2;
			}
		}
	}
};

#endif
