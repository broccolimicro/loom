#include "graph.h"


graph::graph()
{

}

graph::graph(state_space *spaces)
{
	//I'd like to do something here. Not sure what.
}

void graph::insert_edge(int from, int to)
{

	if(from >= (int)edges.size())
		edges.resize(from+1, vector<int>());
	edges[from].push_back(to);
	cout << "Connecting " << from << " and " << to << endl;
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
