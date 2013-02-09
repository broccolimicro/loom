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
