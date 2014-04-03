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

uint32_t itom(int v)
{
	v = (v&3) + 1;
	return	(v << 0 ) | (v << 2 ) | (v << 4 ) | (v << 6 ) |
			(v << 8 ) | (v << 10) | (v << 12) | (v << 14) |
			(v << 16) | (v << 18) | (v << 20) | (v << 22) |
			(v << 24) | (v << 26) | (v << 28) | (v << 30);
}

int mtoi(uint32_t v)
{
	return (v&3) - 1;
}

uint32_t vidx(int v)
{
	return (30 - ((v & 0x0000000F)<<1));
}

uint32_t vmsk(int v)
{
	return (0x00000003 << (30 - ((v & 0x0000000F)<<1)));
}

minterm::minterm()
{
	values.clear();
	default_value = vX;
	size = 0;
}

minterm::minterm(const minterm &m)
{
	values = m.values;
	default_value = m.default_value;
	size = m.size;
}

minterm::minterm(sstring str)
{
	size = 0;
	default_value = vX;
	uint32_t val = 0;
	for (sstring::iterator ch = str.begin(); ch != str.end(); ch++)
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

	values.back() &= (0xFFFFFFFF << (32 - ((size & 0xF)<<1)));
}

minterm::~minterm()
{
}

uint32_t minterm::get(int uid)
{
	if (uid >= size)
		resize(uid+1, default_value);

	uint32_t v = (values[uid>>4] >> vidx(uid)) & 3;
	return	(v << 0 ) | (v << 2 ) | (v << 4 ) | (v << 6 ) |
			(v << 8 ) | (v << 10) | (v << 12) | (v << 14) |
			(v << 16) | (v << 18) | (v << 20) | (v << 22) |
			(v << 24) | (v << 26) | (v << 28) | (v << 30);
}

uint32_t minterm::val(int uid)
{
	if (uid >= size)
		resize(uid+1, default_value);

	return mtoi(values[uid>>4] >> vidx(uid));
}

void minterm::set(int uid, uint32_t v)
{
	if (uid >= size)
		resize(uid+1, default_value);

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

		if (w >= values.size())
			values.resize(w+1, r);

		values[size >> 4] |= r & (0x3FFFFFFF >> (((size&0x0000000F)<<1) - 2));
		values[w] = (values[w] >> l) << l;

		size = s+1;
	}

	values.back() &= (0xFFFFFFFF << (32 - ((size & 0xF)<<1)));
}

void minterm::clear()
{
	values.clear();
}

void minterm::sv_union(int uid, uint32_t v)
{
	if (uid >= size)
		resize(uid+1, default_value);

	values[uid >> 4] |= (v & vmsk(uid));
}

void minterm::sv_intersect(int uid, uint32_t v)
{
	if (uid >= size)
		resize(uid+1, default_value);

	values[uid >> 4] &= (v | ~vmsk(uid));
}

void minterm::sv_invert(int uid)
{
	if (uid >= size)
		resize(uid+1, default_value);

	values[uid >> 4] ^= vmsk(uid);
}

void minterm::sv_or(int uid, uint32_t v)
{
	if (uid >= size)
		resize(uid+1, default_value);

	uint32_t m = vmsk(uid);
	v = (v & m) | (v0 & ~m);
	values[uid >> 4] = (((values[uid >> 4]&(v << 1)) | (values[uid >> 4]&v) | (v&(values[uid >> 4] << 1))) & v1) | (values[uid >> 4] & v & v0);
}

void minterm::sv_and(int uid, uint32_t v)
{
	if (uid >= size)
		resize(uid+1, default_value);

	uint32_t m = vmsk(uid);
	v = (v & m) | (v1 & ~m);
	values[uid >> 4] = (values[uid >> 4] & v & v1) | (((values[uid >> 4]&(v >> 1)) | (values[uid >> 4]&v) | (v&(values[uid >> 4] >> 1))) & v0);
}

void minterm::sv_not(int uid)
{
	if (uid >= size)
		resize(uid+1, default_value);

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
		resize(s.size, default_value);
	else if (size > s.size)
		s.resize(size, s.default_value);

	bool result = true;
	for (int i = 0; i < values.size(); i++)
		result = result && ((values[i] & s.values[i]) == s.values[i]);
	return result;
}

// assumes that s1.size == s2.size
bool minterm::conflict(minterm s)
{
	if (s.size > size)
		resize(s.size, default_value);
	else if (size > s.size)
		s.resize(size, s.default_value);

	bool result = true;
	uint32_t C;
	int i;
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
		resize(s.size, default_value);
	else if (size > s.size)
		s.resize(size, s.default_value);

	uint32_t a;
	int count = 0;
	for (int i = 0; i < values.size(); i++)
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

pair<int, int> minterm::xdiff_count(minterm s)
{
	if (s.size > size)
		resize(s.size, default_value);
	else if (size > s.size)
		s.resize(size, s.default_value);

	uint32_t a, b, c, d;
	int xcount0 = 0;
	int xcount1 = 0;
	for (int i = 0; i < values.size(); i++)
	{
		a = values[i] & 0x55555555 & (values[i] >> 1);

		b = s.values[i] & 0x55555555 & (s.values[i] >> 1);

		c = a & ~b;
		d = b & ~a;

		c = (c & 0x33333333) + ((c >> 2) & 0x33333333);
		c = (c & 0x0F0F0F0F) + ((c >> 4) & 0x0F0F0F0F);
		c = c + (c >> 8);
		c = c + (c >> 16);
		xcount0 += c & 0x0000003F;

		d = (d & 0x33333333) + ((d >> 2) & 0x33333333);
		d = (d & 0x0F0F0F0F) + ((d >> 4) & 0x0F0F0F0F);
		d = d + (d >> 8);
		d = d + (d >> 16);
		xcount1 += d & 0x0000003F;
	}

	return pair<int, int>(xcount0, xcount1);
}

minterm minterm::xoutnulls()
{
	minterm result;
	result.size = size;
	uint32_t a;
	for (int i = 0; i < (int)values.size(); i++)
	{
		a = ~values[i] & (~values[i] >> 1) & 0x55555555;
		result.values.push_back(values[i] | a | (a << 1));
	}

	result.values.back() &= (0xFFFFFFFF << (32 - ((result.size & 0xF)<<1)));
	return result;
}

minterm minterm::mask()
{
	minterm result;
	int i;
	if (values.size() > 0)
	{
		for (i = 0; i < (int)values.size(); i++)
			result.values.push_back(((((values[i]>>1)&v0)^(values[i]&v0)) | ((values[i]&v1)^((values[i]<<1)&v1))));
		result.size = size;
		result.values.back() &= (0xFFFFFFFF << (32 - ((result.size & 0xF)<<1)));
		return result;
	}
	else
		return minterm(v_);
}

minterm minterm::inverse()
{
	minterm result;
	for (int i = 0; i < values.size(); i++)
		result.values.push_back((((values[i] << 1) & v1) | ((values[i] >> 1) & v0)));
	result.size = size;
	if (result.values.size() > 0)
		result.values.back() &= (0xFFFFFFFF << (32 - ((result.size & 0xF)<<1)));
	else
		result.default_value = 0;
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

svector<minterm> minterm::expand(svector<int> uids)
{
	svector<minterm> r1;

	r1.push_back(*this);
	for (int i = 0; i < (int)uids.size(); i++)
	{
		for (int j = r1.size()-1; j >= 0; j--)
		{
			if (r1[j].val(uids[i]) == 2)
			{
				r1.push_back(r1[j] & minterm(uids[i], 0));
				r1[j] &= minterm(uids[i], 1);
			}
		}
	}
	return r1;
}

minterm::minterm(uint32_t val)
{
	default_value = vX;
	if (val == 0)
	{
		values = svector<uint32_t>(1, 0);
		size = 1;
	}
	else if (val == 1)
	{
		values = svector<uint32_t>();
		size = 0;
	}
}

minterm::minterm(int var, uint32_t val)
{
	default_value = vX;
	uint32_t w		= var >> 4;
	uint32_t l		= vidx(var);
	uint32_t v		= itom(val);
	uint32_t m		= 0x00000003 << l;

	size = var+1;
	values.resize(w+1, default_value);
	values[w] = (values[w] >> l) << l;
	values[w] = (values[w] & ~m) | (v & m);
}

minterm::minterm(smap<int, uint32_t> vals)
{
	default_value = vX;
	size = vals.rbegin()->first;
	smap<int, uint32_t>::iterator i;
	uint32_t w		= size >> 4;
	uint32_t l		= vidx(size);
	uint32_t m;

	values.resize(w+1, default_value);
	values[w] = (values[w] >> l) << l;
	for (i = vals.begin(); i != vals.end(); i++)
	{
		w		= i->first >> 4;
		m		= vmsk(i->first);
		values[w] = (values[w] & ~m) | (itom(i->second) & m);
	}
	size++;
}

minterm::minterm(sstring exp, variable_space *vars)
{
	default_value = vX;
	sstring var;
	uint32_t value;
	int uid;
	int k, l;
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
				cerr << "Error: Undefined variable \"" << var << "\"." << endl;

			l = k+1;
		}
	}
}

svector<int> minterm::vars()
{
	svector<int> result;
	for (int i = 0; i < size; i++)
		if (get(i) != vX)
			result.push_back(i);

	return result;
}

void minterm::vars(svector<int> *var_list)
{
	for (int i = 0; i < size; i++)
		if (get(i) != vX)
			var_list->push_back(i);
}

minterm minterm::refactor(svector<int> ids)
{
	minterm result;
	for (int i = 0; i < (int)ids.size(); i++)
		result.set(ids[i], get(i));
	return result;
}

minterm minterm::refactor(svector<pair<int, int> > ids)
{
	minterm result;
	for (int i = 0; i < (int)ids.size(); i++)
		result.set(ids[i].second, get(ids[i].first));
	return result;
}

minterm minterm::hide(int var)
{
	minterm result = *this;
	if (var < size)
		result.values[var >> 4] |= vmsk(var);
	else
		result.resize(var+1, result.default_value);
	return result;
}

minterm minterm::hide(svector<int> vars)
{
	minterm result = *this;
	for (int i = 0; i < (int)vars.size(); i++)
	{
		if (vars[i] < size)
			result.values[vars[i] >> 4] |= vmsk(vars[i]);
		else
			result.resize(vars[i]+1, result.default_value);
	}
	return result;
}

void minterm::extract(smap<int, minterm> *result)
{
	uint32_t v;
	for (int i = 0; i < size; i++)
	{
		v = get(i);
		if (v != vX && v != v_)
			result->insert(pair<int, minterm>(i, (*this)[i]));
	}
}

smap<int, minterm> minterm::extract()
{
	smap<int, minterm> result;
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

smap<int, uint32_t> minterm::anysat()
{
	smap<int, uint32_t> result;
	uint32_t v;
	for (int i = 0; i < size; i++)
	{
		v = get(i);
		if (v != vX && v != v_)
			result.insert(pair<int, uint32_t>(i, (v == v1)));
	}
	return result;
}

svector<smap<int, uint32_t> > minterm::allsat()
{
	return svector<smap<int, uint32_t> >(1, anysat());
}

minterm &minterm::operator=(minterm s)
{
	values.clear();
	values = s.values;
	size = s.size;
	default_value = s.default_value;
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

minterm minterm::operator&(minterm s)
{
	if (s.size > size)
		resize(s.size, default_value);
	else if (size > s.size)
		s.resize(size, s.default_value);

	minterm result;
	for (int i = 0; i < values.size(); i++)
		result.values.push_back(values[i] & s.values[i]);
	result.size = size;

	return result;
}

minterm minterm::operator|(minterm s)
{
	if (s.size > size)
		resize(s.size, default_value);
	else if (size > s.size)
		s.resize(size, s.default_value);

	minterm result;
	for (int i = 0; i < values.size(); i++)
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
		resize(t.size, default_value);
	else if (size > t.size)
		t.resize(size, t.default_value);

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

sstring minterm::print(variable_space *vars, sstring prefix)
{
	sstring res;
	bool first = false;

	sstring tbl[4] = {"!", "~", "", ""};
	uint32_t v;
	for (int i = 0; i < values.size(); i++)
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
					res += tbl[(v>>(2*j))&3] + prefix + "x" + sstring(i*16 + (15-j));
				first = true;
			}
		}

		if ((v&3) != 3 && ((int)i*16 + 15) < size)
		{
			if (first)
				res += "&";
			if (vars != NULL)
				res += tbl[v&3] + prefix + vars->get_name(i*16 + 15);
			else
				res += tbl[v&3] + prefix + "x" + sstring(i*16 + 15);
			first = true;
		}
	}

	if (res == "")
		res = "1";

	return res;
}

sstring minterm::print_assign(variable_space *vars, sstring prefix)
{
	sstring res;
	sstring res2;
	bool first = false;

	sstring tbl[4] = {"", "0", "1", ""};
	uint32_t v;
	for (int i = 0; i < values.size(); i++)
	{
		v = values[i];
		for (int j = 15; j > 0; j--)
		{
			if (((v>>(2*j))&3) != 3 && ((v>>(2*j))&3) != 0 && ((int)i*16 + (15-j)) < size)
			{
				if (first)
				{
					res += ",";
					res2 += ",";
				}

				if (vars != NULL)
					res += prefix + vars->get_name(i*16 + (15-j));
				else
					res += prefix + "x" + sstring(i*16 + (15-j));
				res2 += tbl[(v>>(2*j))&3];

				first = true;
			}
		}
		if ((v&3) != 3 && (v&3) != 0 && ((int)i*16 + 15) < size)
		{
			if (first)
			{
				res += ",";
				res2 += ",";
			}

			if (vars != NULL)
				res += prefix + vars->get_name(i*16 + 15);
			else
				res += prefix + "x" + sstring(i*16 + 15);
			res2 += tbl[v&3];

			first = true;
		}
	}

	res += ":=" + res2;

	if (res2 == "")
		return "skip";

	return res;
}

sstring minterm::print_with_quotes(variable_space *vars, sstring prefix)
{
        sstring res;
        bool first = false;

        sstring tbl[4] = {"!", "~", "", ""};
        uint32_t v;
        for (int i = 0; i < values.size(); i++)
        {
                v = values[i];
                for (int j = 15; j > 0; j--)
                {
                        if (((v>>(2*j))&3) != 3 && ((int)i*16 + (15-j)) < size)
                        {
                                if (first)
                                        res += "&";
                                if (vars != NULL)
                                        res += tbl[(v>>(2*j))&3] + "\"" + prefix + vars->get_name(i*16 + (15-j)) + "\"";
                                else
                                        res += tbl[(v>>(2*j))&3] + "\"" + prefix + "x" + sstring(i*16 + (15-j)) + "\"";
                                first = true;
                        }
                }

                if ((v&3) != 3 && ((int)i*16 + 15) < size)
                {
                        if (first)
                                res += "&";
                        if (vars != NULL)
                                res += tbl[v&3] + "\"" + prefix + vars->get_name(i*16 + 15) + "\"";
                        else
                                res += tbl[v&3] + "\"" + prefix + "x" + sstring(i*16 + 15) + "\"";
                        first = true;
                }
        }

        if (res == "")
                res = "1";

        return res;
}


bool operator==(minterm s1, minterm s2)
{
	if (s2.size > s1.size)
		s1.resize(s2.size, s1.default_value);
	else if (s1.size > s2.size)
		s2.resize(s1.size, s2.default_value);

	bool result = true;

	int i = 0;
	for (i = 0; i < s1.values.size() && i < s2.values.size(); i++)
		result = result && (s1.values[i] == s2.values[i]);

	return result;
}

bool operator!=(minterm s1, minterm s2)
{
	if (s2.size > s1.size)
		s1.resize(s2.size, s1.default_value);
	else if (s1.size > s2.size)
		s2.resize(s1.size, s2.default_value);

	bool result = false;

	int i = 0;
	for (i = 0; i < s1.values.size(); i++)
		result = result || (s1.values[i] != s2.values[i]);

	return result;
}

bool operator==(minterm s1, uint32_t s2)
{
	bool zero = false, one = true;
	int i;
	for (i = 0; i < (int)s1.values.size()-1; i++)
	{
		zero = zero || (((s1.values[i]>>1) | s1.values[i] | v1) != vX);
		one = one && ((((~s1.values[i]>>1) | (~s1.values[i])) & v0) == 0);
	}
	if ((int)s1.values.size() > 0)
	{
		zero = zero || (((s1.values[i]>>1) | s1.values[i] | v1 | ~(vX << (32 - ((s1.size&0x0000000F) << 1)))) != vX);
		one = one && ((((~s1.values[i]>>1) | (~s1.values[i])) & (v0 << (32 - ((s1.size&0x0000000F) << 1)))) == 0);
	}

	if (((s2 == 0) && zero) || ((s2 == 1) && one))
		return true;
	else
		return false;
}

bool operator!=(minterm s1, uint32_t s2)
{
	bool zero = false, one = true;
	int i = 0;
	for (i = 0; i < (int)s1.values.size()-1; i++)
	{
		zero = zero || (((s1.values[i]>>1) | s1.values[i] | v1) != vX);
		one = one && ((((~s1.values[i]>>1) | (~s1.values[i])) & v0) == 0);
	}
	if ((int)s1.values.size() > 0)
	{
		zero = zero || (((s1.values[i]>>1) | s1.values[i] | v1 | ~(vX << (32 - ((s1.size&0x0000000F) << 1)))) != vX);
		one = one && ((((~s1.values[i]>>1) | (~s1.values[i])) & (v0 << (32 - ((s1.size&0x0000000F) << 1)))) == 0);
	}

	if (((s2 == 0) && zero) || ((s2 == 1) && one))
		return false;
	else
		return true;
}

bool operator<(minterm s1, minterm s2)
{
	if (s2.size > s1.size)
		s1.resize(s2.size, s1.default_value);
	else if (s1.size > s2.size)
		s2.resize(s1.size, s2.default_value);

	for (int i = 0; i < (int)s1.values.size(); i++)
	{
		if (s1.values[i] < s2.values[i])
			return true;
		else if (s1.values[i] > s2.values[i])
			return false;
	}

	return false;
}

bool operator>(minterm s1, minterm s2)
{
	if (s2.size > s1.size)
		s1.resize(s2.size, s1.default_value);
	else if (s1.size > s2.size)
		s2.resize(s1.size, s2.default_value);

	for (int i = 0; i < (int)s1.values.size(); i++)
	{
		if (s1.values[i] > s2.values[i])
			return true;
		else if (s1.values[i] < s2.values[i])
			return false;
	}

	return false;
}

bool operator<=(minterm s1, minterm s2)
{
	if (s2.size > s1.size)
		s1.resize(s2.size, s1.default_value);
	else if (s1.size > s2.size)
		s2.resize(s1.size, s2.default_value);

	for (int i = 0; i < (int)s1.values.size(); i++)
	{
		if (s1.values[i] < s2.values[i])
			return true;
		else if (s1.values[i] > s2.values[i])
			return false;
	}

	return true;
}

bool operator>=(minterm s1, minterm s2)
{
	if (s2.size > s1.size)
		s1.resize(s2.size, s1.default_value);
	else if (s1.size > s2.size)
		s2.resize(s1.size, s2.default_value);

	for (int i = 0; i < (int)s1.values.size(); i++)
	{
		if (s1.values[i] > s2.values[i])
			return true;
		else if (s1.values[i] < s2.values[i])
			return false;
	}

	return true;
}
