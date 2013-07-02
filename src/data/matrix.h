#include "ptr.h"
#include "../common.h"

#ifndef matrix_h
#define matrix_h

template <class t>
struct matrix
{
	matrix()
	{
		this->h = 0;
		this->v = 0;
	}

	matrix(int h)
	{
		this->h = h;
		this->v = 0;
		data.resize(h, vector<t>());
	}

	matrix(int h, int v)
	{
		this->h = h;
		this->v = v;
		data.resize(h, vector<t>(v, (t)0));
	}

	~matrix()
	{
	}

	vector<vector<t> > data;
	int h, v;

	template <class t2>
	matrix<t> &operator=(matrix<t2> m)
	{
		data.clear();
		data = m.data;
		h = m.h;
		v = m.v;
		return *this;
	}

	template <class t2>
	matrix<t> &operator+=(matrix<t2> m)
	{
		*this = *this + m;
		return *this;
	}

	template <class t2>
	matrix<t> &operator-=(matrix<t2> m)
	{
		*this = *this - m;
		return *this;
	}

	vector<t> &operator[](int index)
	{
		return data[index];
	}

	vector<ptr<t> > operator()(int index)
	{
		vector<ptr<t> > result(data.size());

		for (int i = 0; i < data.size(); i++)
			result[i].value = &(data[i][index]);

		return result;
	}

	matrix<ptr<t> > operator()(int a, int b)
	{
		matrix<ptr<t> > result(b-a+1);

		for (int i = a; i <= b; i++)
			result.data[i-a] = data[i];
		return result;
	}

	matrix<ptr<t> > operator()(int a, int b, int c, int d)
	{
		matrix<ptr<t> > result(b-a+1, c-d+1);

		for (int i = a; i <= b; i++)
			for (int j = c; j <= d; j++)
				result.data[i-a][j-c] = data[i][j];

		return result;
	}

	matrix<t> &swapr(int a, int b)
	{
		vector<t> temp = data[a];
		data[a] = data[b];
		data[b] = temp;
		return *this;
	}

	matrix<t> &remr(int r)
	{
		for (int k = 0; k < (int)data.size(); k++)
			data[k].erase(data[k].begin() + r);
		return *this;
	}

	matrix<t> &swapc(int a, int b)
	{
		t temp;
		for (int i = 0; i < data.size(); i++)
		{
			temp = data[i][a];
			data[i][a] = data[i][b];
			data[i][b] = temp;
		}
		return *this;
	}

	matrix<t> &remc(int c)
	{
		data.erase(data.begin() + c);
		return *this;
	}

	matrix<t> &addr()
	{
		v++;
		for (size_t i = 0; i < data.size(); i++)
			data[i].resize(v+1, (t)0);

		return *this;
	}

	matrix<t> &addc()
	{
		h++;
		data.resize(h+1, vector<t>(v, (t)0));
		return *this;
	}

	size_t size()
	{
		return data.size();
	}

	void print(FILE *file)
	{
		for (int i = 0; i < (int)data.size(); i++)
		{
			fprintf(file, "[");
			for (int j = 0; j < (int)data[i].size(); j++)
				fprintf(file, "%d ", data[i][j]);
			fprintf(file, "]\n");
		}
		fprintf(file, "\n");
	}

	void print()
	{
		print(stdout);
	}
};

template <class t>
matrix<t> operator-(matrix<t> m)
{
	for (int i = 0; i < m.data.size(); i++)
		for (int j = 0; j < m.data[i].size(); j++)
			m.data[i][j] = -m.data[i][j];

	return m;
}

template <class t1, class t2>
matrix<t1> operator+(matrix<t1> m1, matrix<t2> m2)
{
	for (int i = 0; i < m1.data.size() && i < m2.data.size(); i++)
		for (int j = 0; j < m1.data[i].size() && j < m2.data[i].size(); j++)
			m1.data[i][j] += m2.data[i][j];

	return m1;
}

template <class t1, class t2>
matrix<t1> operator-(matrix<t1> m1, matrix<t2> m2)
{
	for (int i = 0; i < m1.data.size() && i < m2.data.size(); i++)
		for (int j = 0; j < m1.data[i].size() && j < m2.data[i].size(); j++)
			m1.data[i][j] -= m2.data[i][j];

	return m1;
}

#endif
