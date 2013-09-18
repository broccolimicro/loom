/*
 * program_counter.cpp
 *
 *  Created on: Sep 16, 2013
 *      Author: nbingham
 */

#include "program_counter.h"
#include "petri.h"

program_counter::program_counter()
{
	net = NULL;
	index = 0;
	delta = 1;
}

program_counter::program_counter(petri *net, int idx)
{
	this->net = net;
	this->index = idx;
}

program_counter::~program_counter()
{
	net = NULL;
	index = 0;
	delta = 1;
}

program_counter_space::program_counter_space()
{

}

program_counter_space::program_counter_space(petri *net, int idx)
{
	pcs.push_back(program_counter(net, idx));
}

program_counter_space::~program_counter_space()
{

}

void program_counter_space::check_size(int s)
{
	if ((int)cov.size() < s)
		cov.resize(s, false);
}

program_counter_space &program_counter_space::operator=(program_counter_space s)
{
	this->pcs = s.pcs;
	this->cov = s.cov;
	return *this;
}
