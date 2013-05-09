#include <stdlib.h>

#ifndef ref_h
#define ref_h

template <class t>
struct ref
{
	ref()
	{
		value = NULL;
	}
	ref(t &v)
	{
		value = &v;
	}
	ref(t *v)
	{
		value = v;
	}
	~ref()
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

	ref<t> &operator=(ref<t> r)
	{
		if (value != NULL && r.value != NULL)
			*value = *r.value;
		else
			value = r.value;
		return *this;
	}

	template <class t2>
	ref<t> &operator=(t2 r)
	{
		if (value != NULL)
			*value = r;
		return *this;
	}

	template <class t2>
	ref<t> &operator+=(t2 r)
	{
		*this = *this + r;
		return *this;
	}

	template <class t2>
	ref<t> &operator-=(t2 r)
	{
		*this = *this - r;
		return *this;
	}

	template <class t2>
	ref<t> &operator*=(t2 r)
	{
		*this = *this * r;
		return *this;
	}

	template <class t2>
	ref<t> &operator/=(t2 r)
	{
		*this = *this / r;
		return *this;
	}

	template <class t2>
	ref<t> &operator%=(t2 r)
	{
		*this = *this % r;
		return *this;
	}

	template <class t2>
	ref<t> &operator&=(t2 r)
	{
		*this = *this & r;
		return *this;
	}

	template <class t2>
	ref<t> &operator|=(t2 r)
	{
		*this = *this | r;
		return *this;
	}

	template <class t2>
	ref<t> &operator^=(t2 r)
	{
		*this = *this ^ r;
		return *this;
	}

	template <class t2>
	ref<t> &operator<<=(t2 r)
	{
		*this = *this << r;
		return *this;
	}

	template <class t2>
	ref<t> &operator>>=(t2 r)
	{
		*this = *this >> r;
		return *this;
	}
};

template <class t>
inline t operator-(ref<t> r)
{
	return -*r.value;
}

template <class t>
inline t operator+(ref<t> r1, ref<t> r2)
{
	return (*r1.value + *r2.value);
}

template <class t>
inline t operator+(ref<t> r1, t r2)
{
	return (*r1.value + r2);
}

template <class t>
inline t operator+(t r1, ref<t> r2)
{
	return (r1 + *r2.value);
}

template <class t>
inline t operator-(ref<t> r1, ref<t> r2)
{
	return (*r1.value - *r2.value);
}

template <class t>
inline t operator-(ref<t> r1, t r2)
{
	return (*r1.value - r2);
}

template <class t>
inline t operator-(t r1, ref<t> r2)
{
	return (r1 - *r2.value);
}

template <class t>
inline t operator*(ref<t> r1, ref<t> r2)
{
	return (*r1.value * *r2.value);
}

template <class t>
inline t operator*(ref<t> r1, t r2)
{
	return (*r1.value * r2);
}

template <class t>
inline t operator*(t r1, ref<t> r2)
{
	return (r1 * *r2.value);
}

template <class t>
inline t operator/(ref<t> r1, ref<t> r2)
{
	return (*r1.value / *r2.value);
}

template <class t>
inline t operator/(ref<t> r1, t r2)
{
	return (*r1.value / r2);
}

template <class t>
inline t operator/(t r1, ref<t> r2)
{
	return (r1 / *r2.value);
}

template <class t>
inline t operator%(ref<t> r1, ref<t> r2)
{
	return (*r1.value % *r2.value);
}

template <class t>
inline t operator%(ref<t> r1, t r2)
{
	return (*r1.value % r2);
}

template <class t>
inline t operator%(t r1, ref<t> r2)
{
	return (r1 % *r2.value);
}

template <class t>
inline bool operator==(ref<t> r1, ref<t> r2)
{
	return (*r1.value == *r2.value);
}

template <class t>
inline bool operator==(ref<t> r1, t r2)
{
	return (*r1.value == r2);
}

template <class t>
inline bool operator==(t r1, ref<t> r2)
{
	return (r1 == *r2.value);
}

template <class t>
inline bool operator!=(ref<t> r1, ref<t> r2)
{
	return (*r1.value != *r2.value);
}

template <class t>
inline bool operator!=(ref<t> r1, t r2)
{
	return (*r1.value != r2);
}

template <class t>
inline bool operator!=(t r1, ref<t> r2)
{
	return (r1 != *r2.value);
}

template <class t>
inline bool operator>(ref<t> r1, ref<t> r2)
{
	return (*r1.value > *r2.value);
}

template <class t>
inline bool operator>(ref<t> r1, t r2)
{
	return (*r1.value > r2);
}

template <class t>
inline bool operator>(t r1, ref<t> r2)
{
	return (r1 > *r2.value);
}

template <class t>
inline bool operator<(ref<t> r1, ref<t> r2)
{
	return (*r1.value < *r2.value);
}

template <class t>
inline bool operator<(ref<t> r1, t r2)
{
	return (*r1.value < r2);
}

template <class t>
inline bool operator<(t r1, ref<t> r2)
{
	return (r1 < *r2.value);
}

template <class t>
inline bool operator>=(ref<t> r1, ref<t> r2)
{
	return (*r1.value >= *r2.value);
}

template <class t>
inline bool operator>=(ref<t> r1, t r2)
{
	return (*r1.value >= r2);
}

template <class t>
inline bool operator>=(t r1, ref<t> r2)
{
	return (r1 >= *r2.value);
}

template <class t>
inline bool operator<=(ref<t> r1, ref<t> r2)
{
	return (*r1.value <= *r2.value);
}

template <class t>
inline bool operator<=(ref<t> r1, t r2)
{
	return (*r1.value <= r2);
}

template <class t>
inline bool operator<=(t r1, ref<t> r2)
{
	return (r1 <= *r2.value);
}

template <class t>
inline bool operator&(ref<t> r1, ref<t> r2)
{
	return (*r1.value & *r2.value);
}

template <class t>
inline bool operator&(ref<t> r1, t r2)
{
	return (*r1.value & r2);
}

template <class t>
inline bool operator&(t r1, ref<t> r2)
{
	return (r1 & *r2.value);
}

template <class t>
inline bool operator|(ref<t> r1, ref<t> r2)
{
	return (*r1.value | *r2.value);
}

template <class t>
inline bool operator|(ref<t> r1, t r2)
{
	return (*r1.value | r2);
}

template <class t>
inline bool operator|(t r1, ref<t> r2)
{
	return (r1 | *r2.value);
}

template <class t>
inline bool operator^(ref<t> r1, ref<t> r2)
{
	return (*r1.value ^ *r2.value);
}

template <class t>
inline bool operator^(ref<t> r1, t r2)
{
	return (*r1.value ^ r2);
}

template <class t>
inline bool operator^(t r1, ref<t> r2)
{
	return (r1 ^ *r2.value);
}

template <class t>
inline bool operator<<(ref<t> r1, ref<t> r2)
{
	return (*r1.value << *r2.value);
}

template <class t>
inline bool operator<<(ref<t> r1, t r2)
{
	return (*r1.value << r2);
}

template <class t>
inline bool operator<<(t r1, ref<t> r2)
{
	return (r1 << *r2.value);
}

template <class t>
inline bool operator>>(ref<t> r1, ref<t> r2)
{
	return (*r1.value >> *r2.value);
}

template <class t>
inline bool operator>>(ref<t> r1, t r2)
{
	return (*r1.value >> r2);
}

template <class t>
inline bool operator>>(t r1, ref<t> r2)
{
	return (r1 >> *r2.value);
}

#endif
