/*
 * triple.h
 *
 *  Created on: Jul 23, 2013
 *      Author: nbingham
 */

#ifndef triple_h
#define triple_h

struct triple
{
	triple();
	triple(int i, int l, int h);
	~triple();

	int i;
	int l;
	int h;

	triple &operator=(triple t);
};

bool operator==(triple t1, triple t2);

namespace std
{
	template<>
	struct hash<triple>
	{
		inline size_t operator()(const triple &v) const
		{
			return hash_pair(v.i, hash_pair(v.l, v.h)) % 15485863;
		}
	};
}

#endif
