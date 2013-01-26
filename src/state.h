/*
 * state.h
 *
 *  Created on: Jan 26, 2013
 *      Author: nbingham
 */

#include "value.h"

#include "common.h"

#ifndef state_h
#define state_h

struct state
{
	// Variable indexed using uid
	vector<value> values;
};

#endif
