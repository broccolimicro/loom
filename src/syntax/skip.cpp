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

skip::skip(instruction *parent, sstring chp, variable_space *vars, flag_space *flags)
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
instruction *skip::duplicate(instruction *parent, variable_space *vars, smap<sstring, sstring> convert)
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

svector<petri_index> skip::generate_states(petri_net *n, rule_space *p, svector<petri_index> f, smap<int, int> pbranch, smap<int, int> cbranch)
{
	flags->inc();
	from = f;
	prs = p;
	net = n;

	if (flags->log_base_state_space())
		(*flags->log_file) << flags->tab << "Skip " << endl;

	uid.push_back(net->push_transition(from, pbranch, cbranch, this));
	flags->dec();

	return uid;
}

void skip::print_hse(sstring t, ostream *fout)
{
	(*fout) << chp;
}
