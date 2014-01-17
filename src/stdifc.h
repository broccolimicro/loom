/*
 * stdifc.h
 *
 *  Created on: Sep 18, 2013
 *      Author: nbingham
 */

#include <vector>
#include <list>
#include <map>
#include <stack>
#include <unordered_map>
#include <string>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <streambuf>
#include <iterator>

using namespace std;

#ifndef stdifc_h
#define stdifc_h

struct sstring : string
{
	sstring() {}

	sstring(int v)
	{
		ostringstream os;
		os << v;
		*this = os.str();
	}

	sstring(uint16_t v)
	{
		ostringstream os;
		os << v;
		*this = os.str();
	}

	sstring(uint32_t v)
	{
		ostringstream os;
		os << v;
		*this = os.str();
	}

	sstring(const sstring &str) : string(str) {}
	sstring(const string &str) : string(str) {}
	sstring(const sstring &str, int pos, int len = npos) : string(str, (size_t)pos, (size_t)len) {}
	sstring(const string &str, int pos, int len = npos) : string(str, (size_t)pos, (size_t)len) {}
	sstring(const char* s) : string(s) {}
	sstring(const char* s, int n) : string(s, (size_t)n) {}
	sstring(int n, char c) : string((size_t)n, c) {}
	sstring(istreambuf_iterator<char, std::char_traits<char> > first, istreambuf_iterator<char, std::char_traits<char> > last) : string(first, last) {}
	~sstring() {}

	static const int npos = -1;

	/*operator const char*()
	{
		return string::c_str();
	}*/

	int find(const sstring str, int pos = 0) const
	{
		return (int)string::find(str, pos);
	}

	int find(const char *str, int pos = 0) const
	{
		return (int)string::find(str, pos);
	}

	int find_first_of(sstring str, int pos = 0) const
	{
		return (int)string::find_first_of(str, pos);
	}

	int find_first_of(const char *str, int pos = 0) const
	{
		return (int)string::find_first_of(str, pos);
	}

	int find_last_of(sstring str, int pos = npos) const
	{
		return (int)string::find_last_of(str, pos);
	}

	int find_last_of(const char *str, int pos = npos) const
	{
		return (int)string::find_last_of(str, pos);
	}

	int find_first_of_l0(sstring search, int pos = 0)
	{
		sstring::iterator i, j;
		int depth[3] = {0, 0, 0};
		int ret;

		for (i = string::begin() + pos, ret = pos; i != string::end(); i++, ret++)
		{
			if (*i == ')')
				depth[0]--;
			else if (*i == ']')
				depth[1]--;
			else if (*i == '}')
				depth[2]--;

			for (j = search.begin(); j != search.end() && depth[0] == 0 && depth[1] == 0 && depth[2] == 0; j++)
				if (*i == *j)
				{
					if (i == string::end())
						return npos;
					else
						return ret;
				}

			if (*i == '(')
				depth[0]++;
			else if (*i == '[')
				depth[1]++;
			else if (*i == '{')
				depth[2]++;
		}

		return npos;
	}

	int find_first_of_l0(list<sstring> search, int pos = 0, list<sstring> exclude = list<sstring>())
	{
		bool found;
		sstring::iterator i;
		list<sstring>::iterator j;
		int depth[3] = {0, 0, 0};
		int ret;

		found = false;
		for (i = string::begin() + pos, ret = pos; i != string::end(); i++, ret++)
		{
			if (*i == ')')
				depth[0]--;
			else if (*i == ']')
				depth[1]--;
			else if (*i == '}')
				depth[2]--;

			for (j = search.begin(); j != search.end() && depth[0] == 0 && depth[1] == 0 && depth[2] == 0 && !found; j++)
				if (string::substr(ret, j->length()) == *j)
					found = true;
			for (j = exclude.begin(); j != exclude.end() && depth[0] == 0 && depth[1] == 0 && depth[2] == 0 && found; j++)
				if (string::substr(ret, j->length()) == *j)
					found = false;

			if (*i == '(')
				depth[0]++;
			else if (*i == '[')
				depth[1]++;
			else if (*i == '{')
				depth[2]++;

			if (found)
			{
				if (i == string::end())
					return npos;
				else
					return ret;
			}
		}

		return npos;
	}

	int find_last_of_l0(sstring search, int pos = 0)
	{
		sstring::reverse_iterator i, j;
		int depth[3] = {0, 0, 0};
		int ret;

		if (pos == sstring::npos)
			pos = 0;
		else
			pos = length() - pos;

		for (i = string::rbegin() + pos, ret = pos; i != string::rend(); i++, ret++)
		{
			if (*i == '(')
				depth[0]--;
			else if (*i == '[')
				depth[1]--;
			else if (*i == '{')
				depth[2]--;

			for (j = search.rbegin(); j != search.rend() && depth[0] == 0 && depth[1] == 0 && depth[2] == 0; j++)
				if (*i == *j)
				{
					if (i == string::rend())
						return npos;
					else
						return length() - ret - 1;
				}

			if (*i == ')')
				depth[0]++;
			else if (*i == ']')
				depth[1]++;
			else if (*i == '}')
				depth[2]++;
		}

		return npos;
	}

	int find_last_of_l0(list<sstring> search, int pos = 0, list<sstring> exclude = list<sstring>())
	{
		bool found;
		sstring::reverse_iterator i;
		list<sstring>::iterator j;
		int depth[3] = {0, 0, 0};
		int ret;

		if (pos == sstring::npos)
			pos = 0;
		else
			pos = length() - pos;

		found = false;
		for (i = string::rbegin() + pos, ret = pos; i != string::rend(); i++, ret++)
		{
			if (*i == '(')
				depth[0]--;
			else if (*i == '[')
				depth[1]--;
			else if (*i == '{')
				depth[2]--;

			for (j = search.begin(); j != search.end() && depth[0] == 0 && depth[1] == 0 && depth[2] == 0 && !found; j++)
				if (string::substr(length() - ret - 1 - j->length(), j->length()) == *j)
					found = true;
			for (j = exclude.begin(); j != exclude.end() && depth[0] == 0 && depth[1] == 0 && depth[2] == 0 && found; j++)
				if (string::substr(length() - ret - 1 - j->length(), j->length()) == *j)
					found = false;

			if (*i == ')')
				depth[0]++;
			else if (*i == ']')
				depth[1]++;
			else if (*i == '}')
				depth[2]++;

			if (found)
			{
				if (i == string::rend())
					return npos;
				else
					return length() - ret - 1;
			}
		}

		return npos;
	}

	int size() const
	{
		return (int)(string::size());
	}

	int length() const
	{
		return (int)(string::length());
	}
};

template <typename type>
struct svector : vector<type>
{
	svector() : vector<type>() {}
	svector(int n) : vector<type>(n) {}
	svector(int n, type v) : vector<type>(n, v) {}
	template <class InputIterator>
	svector(InputIterator first, InputIterator second) : vector<type>(first, second) {}
	~svector() {}

	int size() const
	{
		return (int)vector<type>::size();
	}

	svector<type> &unique()
	{
		std::sort(vector<type>::begin(), vector<type>::end());
		vector<type>::resize(std::unique(vector<type>::begin(), vector<type>::end()) - vector<type>::begin());
		return *this;
	}

	svector<type> &sort()
	{
		std::sort(vector<type>::begin(), vector<type>::end());
		return *this;
	}

	svector<type> &rsort()
	{
		std::sort(vector<type>::rbegin(), vector<type>::rend());
		return *this;
	}

	typename vector<type>::iterator find(type v)
	{
		return std::find(vector<type>::begin(), vector<type>::end(), v);
	}

	svector<type> &merge(vector<type> v)
	{
		vector<type>::insert(vector<type>::end(), v.begin(), v.end());
		return *this;
	}

	svector<type> set_intersection(vector<type> v)
	{
		svector<type> result(v.size() + size());
		typename vector<type>::iterator new_end = std::set_intersection(vector<type>::begin(), vector<type>::end(), v.begin(), v.end(), result.begin());
		result.resize(new_end - result.begin());
		return result;
	}

	svector<type> set_difference(vector<type> v)
	{
		svector<type> result(v.size() + size());
		typename vector<type>::iterator new_end = std::set_difference(vector<type>::begin(), vector<type>::end(), v.begin(), v.end(), result.begin());
		result.resize(new_end - result.begin());
		return result;
	}

	svector<type> set_symmetric_difference(svector<type> v)
	{
		vector<type> result(v.size() + size());
		typename vector<type>::iterator new_end = std::set_symmetric_difference(vector<type>::begin(), vector<type>::end(), v.begin(), v.end(), result.begin());
		result.resize(new_end - result.begin());
		return result;
	}

	svector<type&> sample(svector<int> v)
	{
		svector<type&> result;
		for (int i = 0; i < v.size(); i++)
			result.push_back(vector<type>::at(v[i]));
		return result;
	}

	bool contains(type v)
	{
		return (std::find(vector<type>::begin(), vector<type>::end(), v) != vector<type>::end());
	}
};

template <typename type>
void vector_symmetric_compliment(vector<type> *v1, vector<type> *v2)
{
	vector<type> result;
	typename vector<type>::iterator i, j;
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

template <typename key, typename val>
struct smap : map<key, val>
{
	smap<key, val> set_intersection(smap<key, val> v)
	{
		smap<key, val> result;
		for (typename map<key, val>::iterator i = map<key, val>::begin(), j = v.begin(); i != map<key, val>::end() && j != v.end();)
		{
			if (i->first == j->first)
			{
				if (i->second == j->second)
					result.insert(*i);
				i++;
				j++;
			}
			else if (i->first < j->first)
				i++;
			else if (i->first > j->first)
				j++;
		}
		return result;
	}

	smap<key, val> &merge(smap<key, val> m)
	{
		map<key, val>::insert(m.begin(), m.end());
		return *this;
	}

	int size() const
	{
		return (int)(map<key, val>::size());
	}
};

#endif
