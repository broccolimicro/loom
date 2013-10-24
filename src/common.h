/*

 * common.h
 *
 * Common is a collection of functions not specific to the compiler that
 * we found useful to define. Note that our #defines for user flags are
 * also stored in common.h as it is accessed by everyone.
 */

#ifndef common_h
#define common_h

#include <stdlib.h>
#include <stdio.h>

#include <string.h>
#include <stdarg.h>
#include <sstream>
#include <math.h>
#include <sys/time.h>

#include "stdifc.h"

bool ac(char c);
bool nc(char c);
bool oc(char c);
bool sc(char c);

sstring hex_to_bin(sstring str);
sstring dec_to_bin(sstring str);

unsigned int count_1bits(unsigned int x);
unsigned int count_0bits(unsigned int x);

int powi(int x, int y);

uint32_t bitwise_or(uint32_t a, uint32_t b);
uint32_t bitwise_and(uint32_t a, uint32_t b);
uint32_t bitwise_not(uint32_t a);

inline int hash_pair(int i, int j)
{
	return (i+j)*(i+j+1)/2 + i;
}

namespace std
{
	template<typename i, typename j>
	struct hash<pair<i, j> >
	{
		inline size_t operator()(const pair<i, j> &v) const
		{
			return hash_pair(v.first, v.second);
		}
	};
}


svector<int> first_combination(int s);
bool next_combination(int S, svector<int> *iter);

template <class type>
svector<type> sample(svector<type> full, svector<int> i)
{
	svector<type> result;
	for (int j = 0; j < i.size(); j++)
		result.push_back(full[i[j]]);
	return result;
}

#define logic canonical

#endif
