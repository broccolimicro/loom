/*
 * skip.cpp
 *
 *  Created on: Aug 21, 2013
 *      Author: nbingham
 */

#include "../common.h"
#include "../utility.h"
#include "../type.h"
#include "../data.h"

#include "skip.h"
#include "assignment.h"

skip::skip()
{
	_kind = "skip";
}

skip::skip(instruction *parent, string chp, variable_space *vars, flag_space *flags)
{
	this->_kind		= "skip";
	this->chp		= chp;
	this->flags 	= flags;
	this->vars		= vars;
	this->parent	= parent;
}

skip::~skip()
{
	_kind = "skip";
}

/* This copies a skip to another process and replaces
 * all of the specified variables.
 */
instruction *skip::duplicate(instruction *parent, variable_space *vars, map<string, string> convert)
{
	skip *instr;

	instr 				= new skip();
	instr->chp			= this->chp;
	instr->vars			= vars;
	instr->flags		= flags;
	instr->parent		= parent;

	return instr;
}

void skip::expand_shortcuts()
{
}

void skip::parse()
{
}

void skip::simulate()
{

}

void skip::rewrite()
{
}

void skip::reorder()
{

}

vector<int> skip::generate_states(petri *n, rule_space *p, vector<int> f, map<int, int> pbranch, map<int, int> cbranch)
{
	flags->inc();
	from = f;
	prs = p;
	net = n;

	if (flags->log_base_state_space())
		(*flags->log_file) << flags->tab << "Skip " << endl;

	uid.push_back(net->insert_dummy(from, pbranch, cbranch, this));
	flags->dec();

	return uid;
}

void skip::print_hse(string t, ostream *fout)
{
	(*fout) << chp;
}
