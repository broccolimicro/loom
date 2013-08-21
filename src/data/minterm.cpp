/*
 * minterm.cpp
 *
 *  Created on: Apr 5, 2013
 *      Author: nbingham
 */

#include "minterm.h"
#include "canonical.h"
#include "variable_space.h"
#include "../common.h"

#define v_ 0x00000000
#define v0 0x55555555
#define v1 0xAAAAAAAA
#define vX 0xFFFFFFFF

inline uint32_t itom(int v)
{
	v = (v&3) + 1;
	return	(v << 0 ) | (v << 2 ) | (v << 4 ) | (v << 6 ) |
			(v << 8 ) | (v << 10) | (v << 12) | (v << 14) |
			(v << 16) | (v << 18) | (v << 20) | (v << 22) |
			(v << 24) | (v << 26) | (v << 28) | (v << 30);
}

inline int mtoi(uint32_t v)
{
	return (v&3) - 1;
}

inline uint32_t vidx(int v)
{
	return (30 - ((v & 0x0000000F)<<1));
}

inline uint32_t vmsk(int v)
{
	return (0x00000003 << (30 - ((v & 0x0000000F)<<1)));
}

minterm::minterm()
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

minterm::~minterm()
{
	values.clear();
	size = 0;
}

uint32_t minterm::get(int uid)
{
	if (uid >= size)
		resize(uid+1, vX);

	uint32_t v = (values[uid>>4] >> vidx(uid)) & 3;
	return	(v << 0 ) | (v << 2 ) | (v << 4 ) | (v << 6 ) |
			(v << 8 ) | (v << 10) | (v << 12) | (v << 14) |
			(v << 16) | (v << 18) | (v << 20) | (v << 22) |
			(v << 24) | (v << 26) | (v << 28) | (v << 30);
}

uint32_t minterm::val(int uid)
{
	if (uid >= size)
		resize(uid+1, vX);

	return mtoi(values[uid>>4] >> vidx(uid));
}

void minterm::set(int uid, uint32_t v)
{
	if (uid >= size)
		resize(uid+1, vX);

	int w = uid >> 4;
	uint32_t m = vmsk(uid);
	values[w] = (values[w] & ~m) | (v & m);
}

void minterm::resize(int s, uint32_t r)
{
	s--;
	int w, l;
	if (s >= size)
	{
		w = s >> 4;
		l = 30 - ((s & 0x0000000F)<<1);

		if (w >= (int)values.size())
			values.resize(w+1, r);

		values[size >> 4] |= r & (0x3FFFFFFF >> (((size&0x0000000F)<<1) - 2));
		values[w] = (values[w] >> l) << l;

		size = s+1;
	}
}

void minterm::clear()
{
	values.clear();
}

void minterm::sv_union(int uid, uint32_t v)
{
	if (uid > size)
		resize(uid+1, vX);

	values[uid >> 4] |= (v & vmsk(uid));
}

void minterm::sv_intersect(int uid, uint32_t v)
{
	if (uid > size)
		resize(uid+1, vX);

	values[uid >> 4] &= (v | ~vmsk(uid));
}

void minterm::sv_invert(int uid)
{
	if (uid > size)
		resize(uid+1, vX);

	values[uid >> 4] ^= vmsk(uid);
}

void minterm::sv_or(int uid, uint32_t v)
{
	if (uid > size)
		resize(uid+1, vX);

	uint32_t m = vmsk(uid);
	v = (v & m) | (v0 & ~m);
	values[uid >> 4] = (((values[uid >> 4]&(v << 1)) | (values[uid >> 4]&v) | (v&(values[uid >> 4] << 1))) & v1) | (values[uid >> 4] & v & v0);
}

void minterm::sv_and(int uid, uint32_t v)
{
	if (uid > size)
		resize(uid+1, vX);

	uint32_t m = vmsk(uid);
	v = (v & m) | (v1 & ~m);
	values[uid >> 4] = (values[uid >> 4] & v & v1) | (((values[uid >> 4]&(v >> 1)) | (values[uid >> 4]&v) | (v&(values[uid >> 4] >> 1))) & v0);
}

void minterm::sv_not(int uid)
{
	if (uid > size)
		resize(uid+1, vX);

	uint32_t m0 = (0x00000001 << vidx(uid));
	uint32_t m1 = (0x00000002 << vidx(uid));
	values[uid >> 4] = (values[uid >> 4] & ~(m0 | m1)) | (((values[uid >> 4] & m0) << 1) & 0xFFFFFFFE) | (((values[uid >> 4] & m1) >> 1) & 0x7FFFFFFF);
}

// Returns true if s2 is a subset of s1
// Returns false otherwise
// assumes that s1.size == s2.size
bool minterm::subset(minterm s)
{
	if (s.size > size)
		resize(s.size, vX);
	else if (size > s.size)
		s.resize(size, vX);

	bool result = true;
	for (size_t i = 0; i < values.size(); i++)
		result = result && ((values[i] & s.values[i]) == s.values[i]);
	return result;
}

// assumes that s1.size == s2.size
bool minterm::conflict(minterm s)
{
	if (s.size > size)
		resize(s.size, vX);
	else if (size > s.size)
		s.resize(size, vX);

	bool result = true;
	uint32_t C;
	size_t i;
	if (values.size() > 0)
	{
		for (i = 0; i < values.size() - 1; i++)
		{
			C = ~(values[i] & s.values[i]);
			result = result && (((C >> 1) & C & v0) == 0);
		}

		C = ~(values[i] & s.values[i]);
		result = result && (((C >> 1) & C & (v0 << (32 - ((size&0x0000000F)<<1)))) == 0);
	}

	return result;
}

int minterm::diff_count(minterm s)
{
	if (s.size > size)
		resize(s.size, vX);
	else if (size > s.size)
		s.resize(size, vX);

	uint32_t a;
	int count = 0;
	for (size_t i = 0; i < values.size(); i++)
	{
		// XOR to see what bits are different
		a = values[i] ^ s.values[i];
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

minterm minterm::mask()
{
	minterm result;
	int i;
	if (values.size() > 0)
	{
		for (i = 0; i < (int)values.size(); i++)
		{
			result.values.push_back(((((values[i]>>1)&v0)^(values[i]&v0)) | ((values[i]&v1)^((values[i]<<1)&v1))));
			printf("%08X->%08X\n", values[i], result.values.back());
		}
		result.size = size;
		return result;
	}
	else
		return minterm(v_);
}

minterm minterm::inverse()
{
	minterm result;
	for (size_t i = 0; i < values.size(); i++)
		result.values.push_back((((values[i] << 1) & v1) | ((values[i] >> 1) & v0)));
	result.size = size;
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

minterm::minterm(uint32_t val)
{
	values.push_back(itom(val));
	size = 16;
}

minterm::minterm(int var, uint32_t val)
{
	uint32_t w		= var >> 4;
	uint32_t l		= vidx(var);
	uint32_t v		= itom(val);
	uint32_t m		= 0x00000003 << l;

	size = var+1;
	values.resize(w+1, vX);
	values[w] = (values[w] >> l) << l;
	values[w] = (values[w] & ~m) | (v & m);
}

minterm::minterm(map<int, uint32_t> vals)
{
	size = vals.rbegin()->first;
	map<int, uint32_t>::iterator i;
	uint32_t w		= size >> 4;
	uint32_t l		= vidx(size);
	uint32_t m;

	values.resize(w+1, vX);
	values[w] = (values[w] >> l) << l;
	for (i = vals.begin(); i != vals.end(); i++)
	{
		w		= i->first >> 4;
		m		= vmsk(i->first);
		values[w] = (values[w] & ~m) | (itom(i->second) & m);
	}
	size++;
}

minterm::minterm(string exp, variable_space *vars)
{
	string var;
	uint32_t value;
	int uid;
	size_t k, l;
	uint32_t s = vars->size();
	uint32_t w = s >> 4, i;

	for (i = 0; i < w; i++)
		values.push_back(vX);
	if (((16 - (s & 0x000000F)) << 1) < 32)
		values.push_back(vX << ((16 - (s & 0x000000F)) << 1));
	size = s;

	for (k = 0, l = 0; k <= exp.size(); k++)
	{
		if (k == exp.size() || exp[k] == '&')
		{
			if (exp[l] == '~')
			{
				value = v0;
				var = exp.substr(l+1, k-l-1);
			}
			else
			{
				value = v1;
				var = exp.substr(l, k-l);
			}

			uid = vars->get_uid(var);

			if (uid >= 0)
				sv_intersect(uid, value);
			else if ((var == "0" && value == v1) || (var == "1" && value == v0))
			{
				for (i = 0; i < w+1; i++)
					values[i] = v_;
				return;
			}
			else if (var != "1" && var != "0")
				cout << "Error: Undefined variable \"" << var << "\"." << endl;

			l = k+1;
		}
	}
}

vector<int> minterm::vars()
{
	vector<int> result;
	for (int i = 0; i < size; i++)
		if (get(i) != vX)
			result.push_back(i);

	return result;
}

void minterm::vars(vector<int> *var_list)
{
	for (int i = 0; i < size; i++)
		if (get(i) != vX)
			var_list->push_back(i);
}

minterm minterm::refactor(vector<int> ids)
{
	minterm result;
	for (int i = 0; i < (int)ids.size(); i++)
		result.set(ids[i], get(i));
	return result;
}

minterm minterm::smooth(int var)
{
	minterm result = *this;
	if (var < size)
		result.values[var >> 4] |= vmsk(var);
	else
		result.resize(var+1, vX);
	return result;
}

minterm minterm::smooth(vector<int> vars)
{
	minterm result = *this;
	for (int i = 0; i < (int)vars.size(); i++)
	{
		if (vars[i] < size)
			result.values[vars[i] >> 4] |= vmsk(vars[i]);
		else
			result.resize(vars[i]+1, vX);
	}
	return result;
}

void minterm::extract(map<int, minterm> *result)
{
	uint32_t v;
	for (int i = 0; i < size; i++)
	{
		v = get(i);
		if (v != vX && v != v_)
			result->insert(pair<int, minterm>(i, (*this)[i]));
	}
}

map<int, minterm> minterm::extract()
{
	map<int, minterm> result;
	uint32_t v;
	for (int i = 0; i < size; i++)
	{
		v = get(i);
		if (v != vX && v != v_)
			result.insert(pair<int, minterm>(i, (*this)[i]));
	}
	return result;
}

minterm minterm::pabs()
{
	minterm result;
	int i;
	for (i = 0; i < (int)values.size(); i++)
		result.values.push_back(values[i] | v1);
	result.size = size;
	if (i > 0)
		result.values[i-1] &= (vX << (32 - ((size&0x0000000F)<<1)));
	return result;
}

minterm minterm::nabs()
{
	minterm result;
	int i;
	for (i = 0; i < (int)values.size(); i++)
		result.values.push_back(values[i] | v0);
	result.size = size;
	if (i > 0)
		result.values[i-1] &= (vX << (32 - ((size&0x0000000F)<<1)));
	return result;
}

int minterm::satcount()
{
	return 1;
}

map<int, uint32_t> minterm::anysat()
{
	map<int, uint32_t> result;
	uint32_t v;
	for (int i = 0; i < size; i++)
	{
		v = get(i);
		if (v != vX && v != v_)
			result.insert(pair<int, uint32_t>(i, (v == v1)));
	}
	return result;
}

vector<map<int, uint32_t> > minterm::allsat()
{
	return vector<map<int, uint32_t> >(1, anysat());
}

minterm &minterm::operator=(minterm s)
{
	values = s.values;
	size = s.size;
	return *this;
}

minterm &minterm::operator=(uint32_t s)
{
	(*this) = minterm(s);
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

minterm &minterm::operator&=(uint32_t s)
{
	*this = *this & s;
	return *this;
}

minterm &minterm::operator|=(uint32_t s)
{
	*this = *this | s;
	return *this;
}

minterm minterm::operator[](int i)
{
	return minterm(i, val(i));
}

minterm minterm::operator()(int i, uint32_t v)
{
	minterm result = *this;
	result.sv_intersect(i, itom(v));
	if (result != 0)
		result.sv_union(i, vX);

	return result;
}

// This assumes that s1.size == s2.size
// TODO
minterm minterm::operator&(minterm s)
{
	if (s.size > size)
		resize(s.size, vX);
	else if (size > s.size)
		s.resize(size, vX);

	minterm result;
	for (size_t i = 0; i < values.size(); i++)
		result.values.push_back(values[i] & s.values[i]);
	result.size = size;

	return result;
}

// This assumes that s1.size == s2.size
minterm minterm::operator|(minterm s)
{
	if (s.size > size)
		resize(s.size, vX);
	else if (size > s.size)
		s.resize(size, vX);

	minterm result;
	for (size_t i = 0; i < values.size(); i++)
		result.values.push_back(values[i] | s.values[i]);
	result.size = size;

	return result;
}

canonical minterm::operator~()
{
	canonical result;
	int i, j;
	for (i = 0; i < size; i++)
	{
		j = val(i);
		if (j == 0 || j == 1)
			result.push_back(minterm(i, 1-j));
	}
	result.mccluskey();
	return result;
}

// This assumes that s1.size == s2.size
minterm minterm::operator&(uint32_t s)
{
	if (s == 0)
		return minterm(0);
	else
		return (*this);
}

// This assumes that s1.size == s2.size
minterm minterm::operator|(uint32_t s)
{
	if (s == 0)
		return (*this);
	else
		return minterm(1);
}

bool minterm::operator==(minterm s)
{
	if (s.size > size)
		resize(s.size, vX);
	else if (size > s.size)
		s.resize(size, vX);

	bool result = true;

	for (size_t i = 0; i < values.size() && i < s.values.size(); i++)
		result = result && (values[i] == s.values[i]);

	return result;
}

bool minterm::operator!=(minterm s)
{
	if (s.size > size)
		resize(s.size, vX);
	else if (size > s.size)
		s.resize(size, vX);

	bool result = false;

	for (size_t i = 0; i < values.size(); i++)
		result = result || (values[i] != s.values[i]);

	return result;
}

bool minterm::operator==(uint32_t s)
{
	bool zero = false, one = true;
	int i;
	for (i = 0; i < (int)values.size()-1; i++)
	{
		zero = zero || (((values[i]>>1) | values[i] | v1) != vX);
		one = one && ((((~values[i]>>1) | (~values[i])) & v0) == 0);
	}
	if ((int)values.size() > 0)
	{
		zero = zero || (((values[i]>>1) | values[i] | v1 | ~(vX << (32 - ((size&0x0000000F) << 1)))) != vX);
		one = one && ((((~values[i]>>1) | (~values[i])) & (v0 << (32 - ((size&0x0000000F) << 1)))) == 0);
	}

	if (((s == 0) && zero) || ((s == 1) && one))
		return true;
	else
		return false;
}

bool minterm::operator!=(uint32_t s)
{
	bool zero = false, one = true;
	int i = 0;
	for (i = 0; i < (int)values.size()-1; i++)
	{
		zero = zero || (((values[i]>>1) | values[i] | v1) != vX);
		one = one && ((((~values[i]>>1) | (~values[i])) & v0) == 0);
	}
	if ((int)values.size() > 0)
	{
		zero = zero || (((values[i]>>1) | values[i] | v1 | ~(vX << (32 - ((size&0x0000000F) << 1)))) != vX);
		one = one && ((((~values[i]>>1) | (~values[i])) & (v0 << (32 - ((size&0x0000000F) << 1)))) == 0);
	}

	if (((s == 0) && zero) || ((s == 1) && one))
		return false;
	else
		return true;
}

bool minterm::constant()
{
	bool zero = false, one = true;
	int i;

	for (i = 0; i < (int)values.size()-1; i++)
	{
		zero = zero || (((values[i]>>1) | values[i] | v1) != vX);
		one = one && ((((~values[i]>>1) | (~values[i])) & v0) == 0);
	}
	if (values.size() > 0)
	{
		zero = zero || (((values[i]>>1) | values[i] | v1 | ~(vX << (32 - ((size&0x0000000F) << 1)))) != vX);
		one = one && ((((~values[i]>>1) | (~values[i])) & (v0 << (32 - ((size&0x0000000F) << 1)))) == 0);
	}

	return (zero || one);
}

minterm minterm::operator>>(minterm t)
{
	if (t.size > size)
		resize(t.size, vX);
	else if (size > t.size)
		t.resize(size, vX);

	uint32_t v;
	minterm result;
	for (int i = 0; i < (int)values.size(); i++)
	{
		v = t.values[i] & (t.values[i] >> 1) & v0;
		v = v | (v<<1);
		result.values.push_back((values[i] & v) | (t.values[i] & ~v));
	}
	result.size = size;
	return result;
}

string minterm::print(variable_space *vars, string prefix)
{
	string res;
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
					res += "&";
				if (vars != NULL)
					res += tbl[(v>>(2*j))&3] + prefix + vars->get_name(i*16 + (15-j));
				else
					res += tbl[(v>>(2*j))&3] + prefix + "x" + to_string(i*16 + (15-j));
				first = true;
			}
		}
		if (((v>>30)&3) != 3 && ((int)i*16 + 15) < size)
		{
			if (first)
				res += "&";
			if (vars != NULL)
				res += tbl[(v>>30)&3] + prefix + vars->get_name(i*16 + 15);
			else
				res += tbl[(v>>30)&3] + prefix + "x" + to_string(i*16 + 15);
			first = true;
		}
	}

	if (res == "")
		res = "1";

	return res;
}
