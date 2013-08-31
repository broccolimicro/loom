/*
 * variable.h
 *
 * This structure describes a variable in the chp program. We need
 * to keep track of a variables name, its type (record, keyword, process),
 * and its bit width.
 *
 */


#ifndef variable_h
#define variable_h

#include "../common.h"
#include "../flag_space.h"

struct variable
{
	variable();
	variable(string name, string type, uint16_t width, bool arg, flag_space *flags);
	variable(string chp, bool arg, flag_space *flags);
	~variable();

	int			uid;
	string		chp;
	string		name;		// the name of the instantiated variable
	string		type;		// the name of the type of the instantiated variable
	uint16_t	width;		// the bit width of the instantiated variable
	bool		fixed;		// is the bit width of this variable fixed or variable?
	bool		driven;		// keep track of whether or not this variable is driven within this process
	bool		arg;		// Is this variable an argument variable into a process?
	vector<int>	reset;
	vector<int> pc;			// Program counter: This helps to determine whether a guard is monotonic or just completely unstable

	list<string> inputs;

	// For debugging purposes
	flag_space *flags;

	variable &operator=(variable v);
	void parse(string chp);
};

ostream &operator<<(ostream &os, map<string, variable> g);
ostream &operator<<(ostream &os, variable s);

#endif
