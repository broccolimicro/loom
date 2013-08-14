/*
 * assertion.cpp
 *
 *  Created on: Aug 13, 2013
 *      Author: nbingham
 */

#include "assertion.h"

assertion::assertion()
{

}

assertion::assertion(instruction *parent, string chp, variable_space *vars, flag_space *flags)
{

}

assertion::~assertion()
{

}

instruction *assertion::duplicate(instruction *parent, variable_space *vars, map<string, string> convert)
{

}

void assertion::expand_shortcuts()
{

}

void assertion::parse()
{

}

void assertion::merge()
{

}

vector<int> assertion::generate_states(petri *n, vector<int> f, map<int, int> pbranch, map<int, int> cbranch)
{

}

void assertion::insert_instr(int uid, int nid, instruction *instr)
{

}

void assertion::print_hse(string t, ostream *fout)
{

}
