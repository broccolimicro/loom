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

int bitwise_or(int a, int b);
int bitwise_and(int a, int b);
int bitwise_not(int a);

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

//In chp.cpp, there is a variable valled verbosity. Set it to one of the following settings to control exactly what is output.
//(bitmasks)
// Predefined verbosity sets
#define VERB_ALL							0xFFFFFFFF
#define VERB_NONE							0x00000000
#define VERB_DEFAULT						0x00000001
#define VERB_RESULT							0x00000421
#define VERB_HSE							0x0001FC00
#define VERB_STATES							0x000003F0
#define VERB_PRS							0x0000000F
#define VERB_DATA							0x0001FFFF
#define VERB_EXEC							0x1001FFFF


#define VERB_DEBUG							0x80000000
#define VERB_PRECOMPILED_CHP				0x00010000
#define VERB_BASE_HSE						0x00008000
#define VERB_MERGED_HSE						0x00004000
#define VERB_PROJECTED_HSE					0x00002000
#define VERB_DECOMPOSED_HSE					0x00001000
#define VERB_RESHUFFLED_HSE					0x00000800
#define VERB_STATE_VAR_HSE					0x00000400
#define VERB_BASE_STATE_SPACE				0x00000200
#define VERB_BASE_STATE_SPACE_DOT			0x00000100
#define VERB_SCRIBEVAR_STATE_SPACE			0x00000080
#define VERB_SCRIBEVAR_STATE_SPACE_DOT		0x00000040
#define VERB_STATEVAR_STATE					0x00000020
#define VERB_STATEVAR_STATE_DOT				0x00000010
#define VERB_BASE_PRS						0x00000008
#define VERB_FACTORED_PRS					0x00000004	// TODO
#define VERB_PLACEHOLDER_PRS				0x00000002	// There is space here more another production rule optimization
#define VERB_BUBBLE_RESHUFFLED_PRS			0x00000001	// TODO




#define TOP_DOWN 0	//This should be a 1 if we are using top down PRS building, and 0 if bottom up.
#define BUBBLELESS 0 //Whether we are doing the new fangled 'bubbleless reshuffling'

//State space output customization flags
#define STATE_LONG_NAME 1		//If 1, have the actual state
#define CHP_EDGE 1				// If 1, output CHP on graph edges. If 0, output functional labels (loop, condition, etc)
#define GRAPH_VERT 1			//If 1, the graph will be high to low. Else, it will be left to right
#define GRAPH_DPI 300			//DPI of the output graph
#define SHOW_ALL_DIFF_STATES 0 	// If 0, only the 'effective' edges of the diff graph are shown.

#define METHOD_NORMAL
//#define METHOD_PETRIFY_SIMPLE
//#define METHOD_PETRIFY_EXP

#endif
