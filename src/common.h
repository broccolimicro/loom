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
#include <iostream>
#include <fstream>
#include <streambuf>
#include <string>
#include <string.h>
#include <stdarg.h>
#include <list>
#include <map>
#include <sstream>
#include <algorithm>
#include <vector>
#include <stack>
#include <math.h>
#include <sys/time.h>
#include <unordered_map>

using namespace std;

bool ac(char c);
bool nc(char c);
bool oc(char c);
bool sc(char c);

template <typename T>
string to_string(T n)
{
     ostringstream os;
     os << n;
     return os.str();
}

string hex_to_bin(string str);
string dec_to_bin(string str);

unsigned int count_1bits(unsigned int x);
unsigned int count_0bits(unsigned int x);

size_t find_first_of_l0(string subject, string search, size_t pos = 0);
size_t find_first_of_l0(string subject, list<string> search, size_t pos = 0, list<string> exclude = list<string>());
size_t find_last_of_l0(string subject, string search, size_t pos = string::npos);
size_t find_last_of_l0(string subject, list<string> search, size_t pos = string::npos, list<string> exclude = list<string>());

int powi(int x, int y);

uint32_t bitwise_or(uint32_t a, uint32_t b);
uint32_t bitwise_and(uint32_t a, uint32_t b);
uint32_t bitwise_not(uint32_t a);

template <class t>
vector<t> unique(vector<t> *v)
{
	sort(v->begin(), v->end());
	v->resize(unique(v->begin(), v->end()) - v->begin());
	return *v;
}

template <class t>
vector<t> intersect(vector<t> v1, vector<t> v2)
{
	vector<t> result;
	typename vector<t>::iterator i, j;
	for (i = v1.begin(), j = v2.begin(); i != v1.end() && j != v2.end();)
	{
		if (*j > *i)
			i++;
		else if (*i > *j)
			j++;
		else
		{
			result.push_back(*i);
			i++;
			j++;
		}
	}

	return result;
}

template <class t>
void unique(vector<t> *v1, vector<t> *v2)
{
	vector<t> result;
	typename vector<t>::iterator i, j;
	for (i = v1->begin(), j = v2->begin(); i != v1->end() && j != v2->end();)
	{
		if (*j > *i)
			i++;
		else if (*i > *j)
			j++;
		else
		{
			i = v1->erase(i);
			j = v2->erase(j);
		}
	}
}

template <class t>
void merge_vectors(vector<t> *v1, vector<t> v2)
{
	v1->insert(v1->end(), v2.begin(), v2.end());
}

template <class t>
vector<t> sample(vector<t> a, vector<t> b)
{
	vector<t> result;
	for (int i = 0; i < (int)b.size(); i++)
		result.push_back(a[b[i]]);
	return result;
}

template <class t, class u>
void intersect(map<t, u> x, map<t, u> y, map<t, u> *result)
{
	result->clear();
	typename map<t, u>::iterator i, j;

	for (i = x.begin(); i != x.end(); i++)
		for (j = y.begin(); j != y.end(); j++)
			if (i->first == j->first && i->second == j->second)
				result->insert(pair<t, u>(i->first, i->second));
}

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

#define logic canonical

#endif
