#include "graph.h"

graph::graph()
{

}

void graph::insert_edge(int from, int to, string chp)
{
	if(from >= (int)edges.size())
		edges.resize(from+1, vector<int>());
	edges[from].push_back(to);
	if(from >= (int)transitions.size())
		transitions.resize(from+1, vector<string>());
	transitions[from].push_back(chp);
	//cout << "Connecting " << from << " and " << to << endl;
	//cout << *this;
}

ostream &operator<<(ostream &os, graph g)
{
	int i, j;
	cout << "Connections:" << endl;
    for (i = 0; i < (int)g.edges.size(); i++)
    {
    	cout << i << ": ";
    	for (j = 0; j < (int)g.edges[i].size(); j++)
    		os << (g.edges[i])[j] << " ";
    	os << endl;
    }
    return os;
}

void graph::push_back(state s)
{
	states.push_back(s);

	vector<trace>::iterator i;
	vector<value>::iterator v;
	trace t;

	for (i = traces.begin(), v = s.begin(); i != traces.end() && v != s.end(); i++, v++)
		i->values.push_back(*v);

	for (; i != traces.end(); i++)
		i->push_back(value("X"));

	for (; v != s.end(); v++)
	{
		t.assign(states.size()-1, *v);
		traces.push_back(t);
	}
}

void graph::push_back(trace t)
{
	traces.push_back(t);

	vector<state>::iterator i;
	vector<value>::iterator v;
	state s;

	for (i = states.begin(), v = t.begin(); i != states.end() && v != t.end(); i++, v++)
		i->values.push_back(*v);

	for (; i != states.end(); i++)
		i->values.push_back(value("X"));

	for (; v != t.end(); v++)
	{
		s.assign(traces.size()-1, *v);
		states.push_back(s);
	}
}

int graph::size()
{
	return states.size();
}

int graph::width()
{
	return traces.size();
}
