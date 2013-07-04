/*
 * minterm.cpp
 *
 *  Created on: Apr 5, 2013
 *      Author: nbingham
 */

#include "minterm.h"
#include "../common.h"

minterm::minterm()
{
	values.clear();
	size = 0;
}

minterm::~minterm()
{
	values.clear();
	size = 0;
}

minterm::minterm(string str)
{
	size = 0;
	uint32_t val = 0;
	for (string::iterator ch = str.begin(); ch != str.end(); ch++)
	{
		val = (val << 2) | (~(uint32_t)(*ch) & 0x00000001) | ((((uint32_t)(*ch) >> 5) ^ ((uint32_t)(*ch) << 1)) & 0x00000002);
		size++;

		if (size%16 == 0)
		{
			values.push_back(val);
			val = 0;
		}
	}

	if (size%16 > 0)
	{
		val <<= (16 - (size%16))*2;
		values.push_back(val);
	}
}

minterm::minterm(int s, uint32_t v)
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

minterm::minterm(int s, int i, uint32_t v)
{
	int w = s >> 4,
		j, k = i>>4,
		m = (0x03 << (30 - ((i & 0x0F)<<1)));
	for (j = 0; j < w; j++)
		values.push_back(i == k ? ((vX&~m) | (v&m)) : vX);
}

void minterm::clear()
{
	values.clear();
}

vector<uint32_t>::iterator minterm::begin()
{
	return values.begin();
}

vector<uint32_t>::iterator minterm::end()
{
	return values.end();
}

// assumes i < size
void minterm::inelastic_set(int uid, uint32_t v)
{
	int w = uid >> 4;
	uint32_t mask = (0x00000003 << (30 - ((uid & 0x0000000F)<<1)));
	values[w] = (values[w] & ~mask) | (v & mask);
}

void minterm::elastic_set(int uid, uint32_t v, uint32_t r)
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

void minterm::sv_union(int uid, uint32_t v)
{
	values[uid >> 4] |= (v & (0x00000003 << (30 - ((uid & 0x0000000F)<<1))));
}

void minterm::sv_intersect(int uid, uint32_t v)
{
	values[uid >> 4] &= (v | ~(0x00000003 << (30 - ((uid & 0x0000000F)<<1))));
}

void minterm::sv_invert(int uid)
{
	values[uid >> 4] ^= (0x00000003 << (30 - ((uid & 0x0000000F)<<1)));
}

void minterm::sv_or(int uid, uint32_t v)
{

}

void minterm::sv_and(int uid, uint32_t v)
{

}

void minterm::sv_not(int uid)
{

}

bool minterm::always_0()
{
	return all_(*this);
}

bool minterm::always_1()
{
	return allX(*this);
}

vector<int> minterm::variable_list()
{
	vector<int> result;
	for (int i = 0; i < size; i++)
		if ((*this)[i] != vX)
			result.push_back(i);

	return result;
}

void minterm::push_back(uint32_t v)
{
	int i = size >> 4;
	uint32_t j = size&0x0F;
	if (j == 0)
		values.push_back(v_);

	uint32_t mask = (0x00000003 << (30 - 2*j));

	values[i] = (values[i] & ~mask) | (v & mask);
	size++;
}

string minterm::print_expr()
{
	ostringstream res;
	bool first = false;

	string tbl[4] = {"!", "~", "", ""};
	uint32_t v;
	for (size_t i = 0; i < values.size(); i++)
	{
		v = values[i];
		for (int j = 15; j > 0; j--)
		{
			if (((v>>(2*j))&3) != 3 && ((int)i*16 + (15-j)) < size)
			{
				if (first)
					res << "&";
				res << tbl[(v>>(2*j))&3] << "x" << i*16 + (15-j);
				first = true;
			}
		}
		if (((v>>30)&3) != 3 && ((int)i*16 + 15) < size)
		{
			if (first)
				res << "&";
			res << tbl[(v>>30)&3] << "x" << i*16 + 15;
			first = true;
		}
	}

	return res.str();
}

string minterm::print_expr(vector<string> vars)
{
	ostringstream res;
	bool first = false;

	string tbl[4] = {"!", "~", "", ""};
	uint32_t v;
	for (size_t i = 0; i < values.size(); i++)
	{
		v = values[i];
		for (int j = 15; j > 0; j--)
		{
			if (((v>>(2*j))&3) != 3 && ((int)i*16 + (15-j)) < size)
			{
				if (first)
					res << "&";
				res << tbl[(v>>(2*j))&3] << vars[i*16 + (15-j)];
				first = true;
			}
		}
		if (((v>>30)&3) != 3 && ((int)i*16 + 15) < size)
		{
			if (first)
				res << "&";
			res << tbl[(v>>30)&3] << vars[i*16 + 15];
			first = true;
		}
	}

	return res.str();
}

string minterm::print_trace()
{
	ostringstream res;

	string tbl[4] = {"_", "0", "1", "X"};
	uint32_t v;
	for (size_t i = 0; i < values.size(); i++)
	{
		v = values[i];
		for (int j = 15; j >= 0; j--)
			if (((v>>(2*j))&3) != 3 && ((int)i*16 + (15-j)) < size)
				res << tbl[(v>>(2*j))&3];
	}

	return res.str();
}

minterm nullv(int s)
{
	return minterm(s, 0x00000000);
}

minterm fullv(int s)
{
	return minterm(s, 0xFFFFFFFF);
}

bool all_(minterm s)
{
	if (s.values.size() == 0)
		return false;

	bool result = true;
	size_t i;
	for (i = 0; i < s.values.size()-1; i++)
		result = result && ((((s.values[i]>>1) | (s.values[i])) & 0x55555555) == 0);
	result = result && ((((s.values[i]>>1) | (s.values[i])) & (0x55555555 << (32 - ((s.size&0x0000000F) << 1)))) == 0);
	return result;
}

bool all0(minterm s)
{
	if (s.values.size() == 0)
		return false;

	bool result = true;
	size_t i;
	for (i = 0; i < s.values.size()-1; i++)
		result = result && ((((s.values[i]>>1) | (~s.values[i])) & 0x55555555) == 0);
	result = result && ((((s.values[i]>>1) | (~s.values[i])) & (0x55555555 << (32 - ((s.size&0x0000000F) << 1)))) == 0);
	return result;
}

bool all1(minterm s)
{
	if (s.values.size() == 0)
		return false;

	bool result = true;
	size_t i;
	for (i = 0; i < s.values.size()-1; i++)
		result = result && ((((~s.values[i]>>1) | (s.values[i])) & 0x55555555) == 0);
	result = result && ((((~s.values[i]>>1) | (s.values[i])) & (0x55555555 << (32 - ((s.size&0x0000000F) << 1)))) == 0);
	return result;
}

bool allX(minterm s)
{
	if (s.values.size() == 0)
		return true;

	bool result = true;
	size_t i;
	for (i = 0; i < s.values.size()-1; i++)
		result = result && ((((~s.values[i]>>1) | (~s.values[i])) & 0x55555555) == 0);
	result = result && ((((~s.values[i]>>1) | (~s.values[i])) & (0x55555555 << (32 - ((s.size&0x0000000F) << 1)))) == 0);
	return result;
}

bool has_(minterm s)
{
	bool result = false;
	size_t i;
	for (i = 0; i < s.values.size()-1; i++)
		result = result || (((s.values[i]>>1) | s.values[i] | 0xAAAAAAAA) < 0xFFFFFFFF);
	result = result || (((s.values[i]>>1) | s.values[i] | 0xAAAAAAAA | ~(0xFFFFFFFF << (32 - ((s.size&0x0000000F) << 1)))) != 0xFFFFFFFF);
	return result;
}

// Returns true if s2 is a subset of s1
// Returns false otherwise
// assumes that s1.size == s2.size
bool subset(minterm s1, minterm s2)
{
	bool result = true;
	for (size_t i = 0; i < s1.values.size() && i < s2.values.size(); i++)
		result = result && ((s1.values[i] & s2.values[i]) == s2.values[i]);
	return result;
}

// assumes that s1.size == s2.size
bool conflict(minterm s1, minterm s2)
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
bool up_conflict(minterm s1, minterm s2)
{
	bool result = true;
	size_t i;
	for (i = 0; i < s1.values.size() - 1; i++)
		result = result && ((~(s1.values[i] >> 1) & s1.values[i] & (s2.values[i] >> 1) & ~s2.values[i] & 0x55555555) == 0);
	result = result && ((~(s1.values[i] >> 1) & s1.values[i] & (s2.values[i] >> 1) & ~s2.values[i] & (0x55555555 << (32 - ((s1.size&0x0000000F)<<1)))) == 0);
	return result;
}

// assumes that s1.size == s2.size
bool down_conflict(minterm s1, minterm s2)
{
	bool result = true;
	size_t i;
	for (i = 0; i < s1.values.size() - 1; i++)
		result = result && (((s1.values[i] >> 1) & ~s1.values[i] & ~(s2.values[i] >> 1) & s2.values[i] & 0x55555555) == 0);
	result = result && (((s1.values[i] >> 1) & ~s1.values[i] & ~(s2.values[i] >> 1) & s2.values[i] & (0x55555555 << (32 - ((s1.size&0x0000000F)<<1)))) == 0);
	return result;
}

int diff_count(minterm s1, minterm s2)
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

uint32_t minterm::operator[](int i)
{
	uint32_t v = (values[i>>4] >> (2*(15 - (i & 0x0000000F))))&0x00000003;
	v = (v << 0 ) | (v << 2 ) | (v << 4 ) | (v << 6 ) |
		(v << 8 ) | (v << 10) | (v << 12) | (v << 14) |
		(v << 16) | (v << 18) | (v << 20) | (v << 22) |
		(v << 24) | (v << 26) | (v << 28) | (v << 30);
	return v;
}

minterm minterm::operator()(int i, uint32_t v)
{
	cout << i << " " << size << endl;
	minterm result = *this;
	result.sv_intersect(i, v);
	if (!has_(result))
		result.sv_union(i, vX);

	return result;
}

minterm &minterm::operator=(minterm s)
{
	values = s.values;
	size = s.size;
	return *this;
}

minterm &minterm::operator&=(minterm s)
{
	*this = *this & s;
	return *this;
}

minterm &minterm::operator|=(minterm s)
{
	*this = *this | s;
	return *this;
}

// This assumes that s1.size == s2.size
minterm operator&(minterm s1, minterm s2)
{
	for (size_t i = 0; i < s1.values.size(); i++)
		s1.values[i] = (s1.values[i] & s2.values[i] & 0xAAAAAAAA) | (((s1.values[i]&(s2.values[i] >> 1)) | (s1.values[i]&s2.values[i]) | (s2.values[i]&(s1.values[i] >> 1))) & 0x55555555);

	return s1;
}

// This assumes that s1.size == s2.size
minterm operator|(minterm s1, minterm s2)
{
	for (size_t i = 0; i < s1.values.size(); i++)
		s1.values[i] = (((s1.values[i]&(s2.values[i] << 1)) | (s1.values[i]&s2.values[i]) | (s2.values[i]&(s1.values[i] << 1))) & 0xAAAAAAAA) | (s1.values[i] & s2.values[i] & 0x55555555);

	return s1;
}

minterm operator~(minterm s)
{
	for (size_t i = 0; i < s.values.size(); i++)
		s.values[i] = (((s.values[i] << 1) & 0xAAAAAAAA) | ((s.values[i] >> 1) & 0x55555555));

	return s;
}

bool operator==(minterm s1, minterm s2)
{
	bool result = true;

	for (size_t i = 0; i < s1.values.size() && i < s2.values.size(); i++)
		result = result && (s1.values[i] == s2.values[i]);

	return result;
}

bool operator!=(minterm s1, minterm s2)
{
	bool result = false;

	for (size_t i = 0; i < s1.values.size() && i < s2.values.size(); i++)
		result = result || (s1.values[i] != s2.values[i]);

	return result;
}

// This assumes that s1.size == s2.size
minterm operator||(minterm s1, minterm s2)
{
	for (size_t i = 0; i < s1.values.size(); i++)
		s1.values[i] |= s2.values[i];

	return s1;
}

// This assumes that s1.size == s2.size
minterm operator&&(minterm s1, minterm s2)
{
	for (size_t i = 0; i < s1.values.size(); i++)
		s1.values[i] &= s2.values[i];

	return s1;
}

minterm operator!(minterm s)
{
	size_t i;
	for (i = 0; i < s.values.size(); i++)
		s.values[i] = ~s.values[i];

	s.values[i-1] &= (0xFFFFFFFF << (32 - ((s.size&0x0000000F)<<1)));

	return s;
}

// This assumes m0.size == m1.size
minterm project(minterm m0, minterm m1)
{
	minterm result;
	uint32_t mask;
	for (int i = 0; i < (int)m1.values.size(); i++)
	{
		mask = ~((m1.values[i]>>1) | m1.values[i]) & 0x55555555;
		mask = mask | (mask<<1);
		result.values.push_back(((m0.values[i] & mask) | m1.values[i]));
	}

	return result;
}
