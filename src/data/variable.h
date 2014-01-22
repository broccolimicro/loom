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
	variable(sstring name, sstring type, uint16_t width, int reset, bool arg, flag_space *flags);
	variable(sstring chp, bool arg, flag_space *flags);
	~variable();

	int			uid;
	sstring		chp;
	sstring		name;		// the name of the instantiated variable
	sstring		type;		// the name of the type of the instantiated variable
	uint16_t	width;		// the bit width of the instantiated variable
	bool		fixed;		// is the bit width of this variable fixed or variable?
	bool		driven;		// keep track of whether or not this variable is driven within this process
	bool		arg;		// Is this variable an argument variable into a process?
	svector<int>	reset;
	svector<int> pc;			// Program counter: This helps to determine whether a guard is monotonic or just completely unstable

	list<sstring> inputs;

	// For debugging purposes
	flag_space *flags;

	variable &operator=(variable v);
	void parse(sstring chp);
};

ostream &operator<<(ostream &os, smap<sstring, variable> g);
ostream &operator<<(ostream &os, variable s);

#endif
