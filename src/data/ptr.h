#include "../common.h"

#ifndef ptr_h
#define ptr_h

template <class t>
struct ptr
{
	ptr()
	{
		value = NULL;
	}
	ptr(t &v)
	{
		value = &v;
	}
	ptr(t *v)
	{
		value = v;
	}
	~ptr()
	{
		value = NULL;
	}

	t *value;

	template <class t2>
	inline operator t2()
	{
		if (value != NULL)
			return ((t2)*value);
		else
			return (t2)0;
	}

	ptr<t> &operator=(ptr<t> r)
	{
		if (value != NULL && r.value != NULL)
			*value = *r.value;
		else
			value = r.value;
		return *this;
	}

	template <class t2>
	ptr<t> &operator=(t2 r)
	{
		if (value != NULL)
			*value = r;
		return *this;
	}

	template <class t2>
	ptr<t> &operator+=(t2 r)
	{
		*this = *this + r;
		return *this;
	}

	template <class t2>
	ptr<t> &operator-=(t2 r)
	{
		*this = *this - r;
		return *this;
	}

	template <class t2>
	ptr<t> &operator*=(t2 r)
	{
		*this = *this * r;
		return *this;
	}

	template <class t2>
	ptr<t> &operator/=(t2 r)
	{
		*this = *this / r;
		return *this;
	}

	template <class t2>
	ptr<t> &operator%=(t2 r)
	{
		*this = *this % r;
		return *this;
	}

	template <class t2>
	ptr<t> &operator&=(t2 r)
	{
		*this = *this & r;
		return *this;
	}

	template <class t2>
	ptr<t> &operator|=(t2 r)
	{
		*this = *this | r;
		return *this;
	}

	template <class t2>
	ptr<t> &operator^=(t2 r)
	{
		*this = *this ^ r;
		return *this;
	}

	template <class t2>
	ptr<t> &operator<<=(t2 r)
	{
		*this = *this << r;
		return *this;
	}

	template <class t2>
	ptr<t> &operator>>=(t2 r)
	{
		*this = *this >> r;
		return *this;
	}
};

template <class t>
inline t operator-(ptr<t> r)
{
	return -*r.value;
}

template <class t>
inline t operator+(ptr<t> r1, ptr<t> r2)
{
	return (*r1.value + *r2.value);
}

template <class t>
inline t operator+(ptr<t> r1, t r2)
{
	return (*r1.value + r2);
}

template <class t>
inline t operator+(t r1, ptr<t> r2)
{
	return (r1 + *r2.value);
}

template <class t>
inline t operator-(ptr<t> r1, ptr<t> r2)
{
	return (*r1.value - *r2.value);
}

template <class t>
inline t operator-(ptr<t> r1, t r2)
{
	return (*r1.value - r2);
}

template <class t>
inline t operator-(t r1, ptr<t> r2)
{
	return (r1 - *r2.value);
}

template <class t>
inline t operator*(ptr<t> r1, ptr<t> r2)
{
	return (*r1.value * *r2.value);
}

template <class t>
inline t operator*(ptr<t> r1, t r2)
{
	return (*r1.value * r2);
}

template <class t>
inline t operator*(t r1, ptr<t> r2)
{
	return (r1 * *r2.value);
}

template <class t>
inline t operator/(ptr<t> r1, ptr<t> r2)
{
	return (*r1.value / *r2.value);
}

template <class t>
inline t operator/(ptr<t> r1, t r2)
{
	return (*r1.value / r2);
}

template <class t>
inline t operator/(t r1, ptr<t> r2)
{
	return (r1 / *r2.value);
}

template <class t>
inline t operator%(ptr<t> r1, ptr<t> r2)
{
	return (*r1.value % *r2.value);
}

template <class t>
inline t operator%(ptr<t> r1, t r2)
{
	return (*r1.value % r2);
}

template <class t>
inline t operator%(t r1, ptr<t> r2)
{
	return (r1 % *r2.value);
}

template <class t>
inline bool operator==(ptr<t> r1, ptr<t> r2)
{
	return (*r1.value == *r2.value);
}

template <class t>
inline bool operator==(ptr<t> r1, t r2)
{
	return (*r1.value == r2);
}

template <class t>
inline bool operator==(t r1, ptr<t> r2)
{
	return (r1 == *r2.value);
}

template <class t>
inline bool operator!=(ptr<t> r1, ptr<t> r2)
{
	return (*r1.value != *r2.value);
}

template <class t>
inline bool operator!=(ptr<t> r1, t r2)
{
	return (*r1.value != r2);
}

template <class t>
inline bool operator!=(t r1, ptr<t> r2)
{
	return (r1 != *r2.value);
}

template <class t>
inline bool operator>(ptr<t> r1, ptr<t> r2)
{
	return (*r1.value > *r2.value);
}

template <class t>
inline bool operator>(ptr<t> r1, t r2)
{
	return (*r1.value > r2);
}

template <class t>
inline bool operator>(t r1, ptr<t> r2)
{
	return (r1 > *r2.value);
}

template <class t>
inline bool operator<(ptr<t> r1, ptr<t> r2)
{
	return (*r1.value < *r2.value);
}

template <class t>
inline bool operator<(ptr<t> r1, t r2)
{
	return (*r1.value < r2);
}

template <class t>
inline bool operator<(t r1, ptr<t> r2)
{
	return (r1 < *r2.value);
}

template <class t>
inline bool operator>=(ptr<t> r1, ptr<t> r2)
{
	return (*r1.value >= *r2.value);
}

template <class t>
inline bool operator>=(ptr<t> r1, t r2)
{
	return (*r1.value >= r2);
}

template <class t>
inline bool operator>=(t r1, ptr<t> r2)
{
	return (r1 >= *r2.value);
}

template <class t>
inline bool operator<=(ptr<t> r1, ptr<t> r2)
{
	return (*r1.value <= *r2.value);
}

template <class t>
inline bool operator<=(ptr<t> r1, t r2)
{
	return (*r1.value <= r2);
}

template <class t>
inline bool operator<=(t r1, ptr<t> r2)
{
	return (r1 <= *r2.value);
}

template <class t>
inline bool operator&(ptr<t> r1, ptr<t> r2)
{
	return (*r1.value & *r2.value);
}

template <class t>
inline bool operator&(ptr<t> r1, t r2)
{
	return (*r1.value & r2);
}

template <class t>
inline bool operator&(t r1, ptr<t> r2)
{
	return (r1 & *r2.value);
}

template <class t>
inline bool operator|(ptr<t> r1, ptr<t> r2)
{
	return (*r1.value | *r2.value);
}

template <class t>
inline bool operator|(ptr<t> r1, t r2)
{
	return (*r1.value | r2);
}

template <class t>
inline bool operator|(t r1, ptr<t> r2)
{
	return (r1 | *r2.value);
}

template <class t>
inline bool operator^(ptr<t> r1, ptr<t> r2)
{
	return (*r1.value ^ *r2.value);
}

template <class t>
inline bool operator^(ptr<t> r1, t r2)
{
	return (*r1.value ^ r2);
}

template <class t>
inline bool operator^(t r1, ptr<t> r2)
{
	return (r1 ^ *r2.value);
}

template <class t>
inline bool operator<<(ptr<t> r1, ptr<t> r2)
{
	return (*r1.value << *r2.value);
}

template <class t>
inline bool operator<<(ptr<t> r1, t r2)
{
	return (*r1.value << r2);
}

template <class t>
inline bool operator<<(t r1, ptr<t> r2)
{
	return (r1 << *r2.value);
}

template <class t>
inline bool operator>>(ptr<t> r1, ptr<t> r2)
{
	return (*r1.value >> *r2.value);
}

template <class t>
inline bool operator>>(ptr<t> r1, t r2)
{
	return (*r1.value >> r2);
}

template <class t>
inline bool operator>>(t r1, ptr<t> r2)
{
	return (r1 >> *r2.value);
}

#endif
