#include "ref.h"
#include <vector>

using namespace std;

#ifndef matrix_h
#define matrix_h

template <class t>
struct matrix
{
	matrix()
	{
	}

	matrix(int h)
	{
		data.resize(h);
	}

	matrix(int h, int v)
	{
		data.resize(h);
		for (int i = 0; i < h; i++)
			data[i].resize(v);
	}

	~matrix()
	{

	}

	vector<vector<t> > data;

	template <class t2>
	matrix<t> &operator=(matrix<t2> m)
	{
		data.clear();
		data = m.data;
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

	vector<ref<t> > operator()(int index)
	{
		vector<ref<t> > result(data.size());

		for (int i = 0; i < data.size(); i++)
			result[i].value = &(data[i][index]);

		return result;
	}

	matrix<ref<t> > operator()(int a, int b)
	{
		matrix<ref<t> > result(b-a+1);

		for (int i = a; i <= b; i++)
			result.data[i-a] = data[i];
		return result;
	}

	matrix<ref<t> > operator()(int a, int b, int c, int d)
	{
		matrix<ref<t> > result(b-a+1, c-d+1);

		for (int i = a; i <= b; i++)
			for (int j = c; j <= d; j++)
				result.data[i-a][j-c] = data[i][j];

		return result;
	}

	matrix<t> remove(int y, int x)
	{
		matrix<t> result(data.size(), data[0].size());
		int i, j, a, b;

		for (i = 0, a = 0; i < data.size(); i == y ? i+=2 : i++, a++)
			for (j = 0, b = 0; j < data[i].size(); j == x ? j+=2 : i++, b++)
				result[a][b] = data[i][j];

		return result;
	}

	matrix<t> &swapr(int a, int b)
	{
		vector<t> temp = data[a];
		data[a] = data[b];
		data[b] = temp;
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

	void print(FILE *file)
	{
		for (int i = 0; i < data.size(); i++)
			data[i].print(file);
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
