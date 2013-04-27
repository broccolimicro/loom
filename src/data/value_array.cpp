/*
 * value_array.cpp
 *
 *  Created on: Apr 5, 2013
 *      Author: nbingham
 */

#include "value_array.h"
#include "../common.h"

value_array::value_array()
{
	values.clear();
	size = 0;
}

value_array::~value_array()
{
	values.clear();
	size = 0;
}

value_array::value_array(const char *str)
{
	size = 0;
	uint32_t val = 0;
	for (char *ch = (char*)str; *ch; ch++)
	{
		val = (val << 2) | (~(uint32_t)(*ch) & 0x00000001) | ((((uint32_t)(*ch) >> 5) ^ ((uint32_t)(*ch) << 1)) & 0x00000002);
		size++;

		if (size%16 == 0)
		{
			values.push_back(val);
			val = 0;
		}
	}

	val <<= (16 - (size%16))*2;
	values.push_back(val);
}

value_array::value_array(int s, uint32_t v)
{
	uint32_t w = s >> 4,
		l = (16 - (s & 0x000000F)) << 1,
		i;

	for (i = 0; i < w; i++)
		values.push_back(v);
	if (l < 32)
		values.push_back(v << l);
	size = s;
}

void value_array::clear()
{
	values.clear();
}

vector<uint32_t>::iterator value_array::begin()
{
	return values.begin();
}

vector<uint32_t>::iterator value_array::end()
{
	return values.end();
}

// assumes i < size
void value_array::inelastic_set(int uid, uint32_t v)
{
	int w = uid >> 4;
	uint32_t mask = (0x00000003 << (30 - ((uid & 0x0000000F)<<1)));
	values[w] = (values[w] & ~mask) | (v & mask);
}

void value_array::elastic_set(int uid, uint32_t v, uint32_t r)
{
	int w = uid >> 4, l = 30 - ((uid & 0x0000000F)<<1);
	uint32_t mask = (0x00000003 << l);

	if (w >= (int)values.size())
		values.resize(w+1, r);
	values[w] = (values[w] & ~mask) | (v & mask);
	if (uid >= size)
	{
		values[size >> 4] |= r & (0x3FFFFFFF >> (((size&0x0000000F)<<1) - 2));
		values[w] = (values[w] >> l) << l;
		size = uid+1;
	}
}

void value_array::sv_union(int uid, uint32_t v)
{
	values[uid >> 4] |= (v & (0x00000003 << (30 - ((uid & 0x0000000F)<<1))));
}

void value_array::sv_intersect(int uid, uint32_t v)
{
	values[uid >> 4] &= (v | ~(0x00000003 << (30 - ((uid & 0x0000000F)<<1))));
}

void value_array::sv_invert(int uid)
{
	values[uid >> 4] ^= (0x00000003 << (30 - ((uid & 0x0000000F)<<1)));
}

void value_array::sv_or(int uid, uint32_t v)
{

}

void value_array::sv_and(int uid, uint32_t v)
{

}

void value_array::sv_not(int uid)
{

}

value_array nullv(int s)
{
	return value_array(s, 0x00000000);
}

value_array fullv(int s)
{
	return value_array(s, 0xFFFFFFFF);
}

bool all_(value_array s)
{
	bool result = true;
	size_t i;
	for (i = 0; i < s.values.size()-1; i++)
		result = result && ((((s.values[i]>>1) | (s.values[i])) & 0x55555555) == 0);
	result = result && ((((s.values[i]>>1) | (s.values[i])) & (0x55555555 << (32 - ((s.size&0x0000000F) << 1)))) == 0);
	return result;
}

bool all0(value_array s)
{
	bool result = true;
	size_t i;
	for (i = 0; i < s.values.size()-1; i++)
		result = result && ((((s.values[i]>>1) | (~s.values[i])) & 0x55555555) == 0);
	result = result && ((((s.values[i]>>1) | (~s.values[i])) & (0x55555555 << (32 - ((s.size&0x0000000F) << 1)))) == 0);
	return result;
}

bool all1(value_array s)
{
	bool result = true;
	size_t i;
	for (i = 0; i < s.values.size()-1; i++)
		result = result && ((((~s.values[i]>>1) | (s.values[i])) & 0x55555555) == 0);
	result = result && ((((~s.values[i]>>1) | (s.values[i])) & (0x55555555 << (32 - ((s.size&0x0000000F) << 1)))) == 0);
	return result;
}

bool allX(value_array s)
{
	bool result = true;
	size_t i;
	for (i = 0; i < s.values.size()-1; i++)
		result = result && ((((~s.values[i]>>1) | (~s.values[i])) & 0x55555555) == 0);
	result = result && ((((~s.values[i]>>1) | (~s.values[i])) & (0x55555555 << (32 - ((s.size&0x0000000F) << 1)))) == 0);
	return result;
}

// Returns true if s2 is a subset of s1
// Returns false otherwise
// assumes that s1.size == s2.size
bool subset(value_array s1, value_array s2)
{
	bool result = true;
	for (size_t i = 0; i < s1.values.size() && i < s2.values.size(); i++)
		result = result && ((s1.values[i] & s2.values[i]) == s2.values[i]);
	return result;
}

// assumes that s1.size == s2.size
bool conflict(value_array s1, value_array s2)
{
	bool result = true;
	uint32_t C;
	size_t i;
	for (i = 0; i < s1.values.size() - 1; i++)
	{
		C = ~(s1.values[i] & s2.values[i]);
		result = result && (((C >> 1) & C & 0x55555555) == 0);
	}

	C = ~(s1.values[i] & s2.values[i]);
	result = result && (((C >> 1) & C & (0x55555555 << (32 - ((s1.size&0x0000000F)<<1)))) == 0);

	return result;
}

// assumes that s1.size == s2.size
bool up_conflict(value_array s1, value_array s2)
{
	bool result = true;
	size_t i;
	for (i = 0; i < s1.values.size() - 1; i++)
		result = result && ((~(s1.values[i] >> 1) & s1.values[i] & (s2.values[i] >> 1) & ~s2.values[i] & 0x55555555) == 0);
	result = result && ((~(s1.values[i] >> 1) & s1.values[i] & (s2.values[i] >> 1) & ~s2.values[i] & (0x55555555 << (32 - ((s1.size&0x0000000F)<<1)))) == 0);
	return result;
}

// assumes that s1.size == s2.size
bool down_conflict(value_array s1, value_array s2)
{
	bool result = true;
	size_t i;
	for (i = 0; i < s1.values.size() - 1; i++)
		result = result && (((s1.values[i] >> 1) & ~s1.values[i] & ~(s2.values[i] >> 1) & s2.values[i] & 0x55555555) == 0);
	result = result && (((s1.values[i] >> 1) & ~s1.values[i] & ~(s2.values[i] >> 1) & s2.values[i] & (0x55555555 << (32 - ((s1.size&0x0000000F)<<1)))) == 0);
	return result;
}

ostream &operator<<(ostream &os, value_array s)
{
	char a[256], x[256];
	int length, i, j, k;
	for (k = 0; k < (int)s.values.size(); k++)
	{
		for (j = 0; j < 256; j+=2)
		{
			a[j] = '\0';
			a[j+1] = '\0';
			x[j] = '_';
			x[j+1] = ' ';
		}

		itoa(s.values[k],a,2);
		length = strlen(a);
		for (i = 0; i < 32 /*&& (i/2 + k*16) < s.size*/; i+=2)
		{
			j = i - 32 + length;
			if (j >= -1)
			{
				if ((a[j] == '0' || j == -1)&& a[j+1] == '0')
					x[i] = '_';
				else if ((a[j] == '0' || j == -1) && a[j+1] == '1')
					x[i] = '0';
				else if (a[j] == '1' && a[j+1] == '0')
					x[i] = '1';
				else if (a[j] == '1' && a[j+1] == '1')
					x[i] = 'X';
			}
		}

		x[i] = '\0';


		os << x;
	}

	return os;
}

int diff_count(value_array s1, value_array s2)
{
	uint32_t a;
	int count = 0;
	for (size_t i = 0; i < s1.values.size(); i++)
	{
		// XOR to see what bits are different
		a = s1.values[i] ^ s2.values[i];
		// OR together any differences in the bit pairs (a single value)
		a = (a | (a >> 1)) & 0x55555555;

		// count the number of bits set to 1 (derived from Hacker's Delight)
		a = (a & 0x33333333) + ((a >> 2) & 0x33333333);
		a = (a & 0x0F0F0F0F) + ((a >> 4) & 0x0F0F0F0F);
		a = a + (a >> 8);
		a = a + (a >> 16);
		count += a & 0x0000003F;
	}
	return count;
}

uint32_t value_array::operator[](int i)
{
	return (values[i>>4] >> (2*(15 - (i & 0x0000000F))))&0x00000003;
}

value_array &value_array::operator=(value_array s)
{
	values = s.values;
	size = s.size;
	return *this;
}

value_array &value_array::operator&=(value_array s)
{
	*this = *this & s;
	return *this;
}

value_array &value_array::operator|=(value_array s)
{
	*this = *this | s;
	return *this;
}

// This assumes that s1.size == s2.size
value_array operator&(value_array s1, value_array s2)
{
	for (size_t i = 0; i < s1.values.size(); i++)
		s1.values[i] = (s1.values[i] & s2.values[i] & 0xAAAAAAAA) | (((s1.values[i]&(s2.values[i] >> 1)) | (s1.values[i]&s2.values[i]) | (s2.values[i]&(s1.values[i] >> 1))) & 0x55555555);

	return s1;
}

// This assumes that s1.size == s2.size
value_array operator|(value_array s1, value_array s2)
{
	for (size_t i = 0; i < s1.values.size(); i++)
		s1.values[i] = (((s1.values[i]&(s2.values[i] << 1)) | (s1.values[i]&s2.values[i]) | (s2.values[i]&(s1.values[i] << 1))) & 0xAAAAAAAA) | (s1.values[i] & s2.values[i] & 0x55555555);

	return s1;
}

value_array operator~(value_array s)
{
	for (size_t i = 0; i < s.values.size(); i++)
		s.values[i] = (((s.values[i] << 1) & 0xAAAAAAAA) | ((s.values[i] >> 1) & 0x55555555));

	return s;
}

bool operator==(value_array s1, value_array s2)
{
	bool result = true;

	for (size_t i = 0; i < s1.values.size() && i < s2.values.size(); i++)
		result = result && (s1.values[i] == s2.values[i]);

	return result;
}

bool operator!=(value_array s1, value_array s2)
{
	bool result = false;

	for (size_t i = 0; i < s1.values.size() && i < s2.values.size(); i++)
		result = result || (s1.values[i] != s2.values[i]);

	return result;
}

// This assumes that s1.size == s2.size
value_array operator||(value_array s1, value_array s2)
{
	for (size_t i = 0; i < s1.values.size(); i++)
		s1.values[i] |= s2.values[i];

	return s1;
}

// This assumes that s1.size == s2.size
value_array operator&&(value_array s1, value_array s2)
{
	for (size_t i = 0; i < s1.values.size(); i++)
		s1.values[i] &= s2.values[i];

	return s1;
}

value_array operator!(value_array s)
{
	size_t i;
	for (i = 0; i < s.values.size(); i++)
		s.values[i] = ~s.values[i];

	s.values[i-1] &= (0xFFFFFFFF << (32 - ((s.size&0x0000000F)<<1)));

	return s;
}


