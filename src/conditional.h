/*
 * conditional.h
 *
 *  Created on: Oct 28, 2012
 *      Author: Ned Bingham
 */

#include "block.h"
#include "common.h"

#ifndef conditional_h
#define conditional_h

struct conditional : block
{

	void parse(string raw);
};

#endif
