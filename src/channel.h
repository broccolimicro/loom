/*
 * channel.h
 *
 *  Created on: Oct 28, 2012
 *      Author: Ned Bingham
 */

#include "record.h"
#include "common.h"

#ifndef channel_h
#define channel_h

struct channel : record
{
	channel(){}
	channel(string raw, map<string, keyword*> types) : record(raw, types)
	{
	}
	~channel(){}

};

#endif
