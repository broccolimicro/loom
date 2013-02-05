#include "graph.h"
#include "space.h"
#include "common.h"

graph::graph()
{

}

graph::graph(state_space *spaces)
{
	//I'd like to do something here. Not sure what.
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


void graph::print_line(int from)
{
	int i;
	if(from >= (int)edges.size())
		edges.resize(from+1, vector<int>());
	cout << from << ": ";
	for (i = 0; i < (int)edges[from].size(); i++)
		cout << (edges[from])[i] << " ";
	cout << endl;
}
void graph::print_line_with_trans(int from)
{
	int i, j;
	if(from >= (int)edges.size())
		edges.resize(from+1, vector<int>());
	cout << from << ":";
	for (i = 0; i < 4 - (int)log10((double)max(from, 1)); i++)
		cout << " ";
	j = 0;
	for (i = 0; i < (int)edges[from].size(); i++)
	{
		cout << edges[from][i] << " ";
		j += (int)log10((double)max(edges[from][i], 1)) + 2;
	}
	for (i = 0; i < 10 - j; i++)
		cout << " ";
	for (i = 0; from < (int)transitions.size() && i < (int)transitions[from].size(); i++)
		cout << (transitions[from])[i] << " ";

	cout << endl;
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
