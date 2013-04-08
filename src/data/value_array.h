/*
 * value_array.h
 *
 *  Created on: Apr 5, 2013
 *      Author: nbingham
 */

#include "../common.h"
#include "value.h"

#ifndef value_array_h
#define value_array_h

#define v_ 0x00000000
#define v0 0x55555555
#define v1 0xAAAAAAAA
#define vX 0xFFFFFFFF

struct value_array
{
	value_array();
	value_array(const char *str);
	value_array(int s, uint32_t v);
	~value_array();

	vector<uint32_t> values;
	int size;

	void clear();
	vector<uint32_t>::iterator begin();
	vector<uint32_t>::iterator end();
	void inelastic_set(int uid, uint32_t v);
	void elastic_set(int uid, uint32_t v, uint32_t r = 0);

	uint32_t operator[](int i);

	value_array &operator=(value_array s);

	value_array &operator&=(value_array s);
	value_array &operator|=(value_array s);
};

value_array nullv(int s);
value_array fullv(int s);

bool all_(value_array s);
bool all0(value_array s);
bool all1(value_array s);
bool allX(value_array s);

bool subset(value_array s1, value_array s2);
bool conflict(value_array s1, value_array s2);
bool up_conflict(value_array s1, value_array s2);
bool down_conflict(value_array s1, value_array s2);

ostream &operator<<(ostream &os, value_array s);

value_array operator~(value_array s);
value_array operator&(value_array s1, value_array s2);
value_array operator|(value_array s1, value_array s2);

value_array operator!(value_array s);
value_array operator||(value_array s1, value_array s2);
value_array operator&&(value_array s1, value_array s2);

bool operator==(value_array s1, value_array s2);
bool operator!=(value_array s1, value_array s2);

int diff_count(value_array s1, value_array s2);

#endif
