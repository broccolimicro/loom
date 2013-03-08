/*
 * chp.cpp
 *
 *  Created on: Oct 23, 2012
 *      Author: Ned Bingham and Nicholas Kramer
 *
 *  DO NOT DISTRIBUTE
 *
 * 	Variable Definition
 * 	x1,x2,...,xn:=E1,E2,...,En
 *
 *  x+
 *  x-
 *
 *	Expression Operators
 *  | & ~ < > == ~=
 *
 *  Composition Operators
 *  ; || *
 *
 *	Order of Operations
 *	()
 *
 *	Loops
 * 	*[g1->s1[]g2->s2[]...[]gn->sn]
 *  *[g1->s1|g2->s2|...|gn->sn]
 *  *[s1]
 *
 *	Conditionals
 *   [g1->s1[]g2->s2[]...[]gn->sn]
 *   [g1->s1|g2->s2|...|gn->sn]
 *   [g1]
 *
 *  Replication
 *   <op i : n..m : s(i)>
 *
 *	Communication
 * 	 x!y
 * 	 x?y
 * 	 x@
 *
 *	Miscellaneous
 * 	 skip
 *
 *	Assertion
 * 	 {...}
 *
 *	Process Definition and Function Block Definitions
 * 	proc ...(...){...}
 *
 * 	Record Definition
 * 	record ...{...}
 *
 * 	channel Definition
 * 	channel ...{...}
 *
 *	Data Types
 * 	int<...> ...
 *
 *	Preprocessor
 * 	 #
 *
 */

#include "common.h"
#include "syntax.h"
#include "data.h"
#include "type.h"
#include "program.h"
#include "utility.h"

/* This structure describes a whole program. It contains a record of all
 * of the types described in this program and all of the global variables
 * defined in this program. It also contains a list of all of the errors
 * produced during the compilation and a list of all of the production rules
 * that result from this compilation.
 */
int main(int argc, char **argv)
{
	//FILE *log = fopen("log.txt", "w");
	//*stdout = *log;

	//Open the top level file
	ifstream t("main.chp");
	string prgm((istreambuf_iterator<char>(t)),
	             istreambuf_iterator<char>());

	size_t i;
	size_t open, close;

	//While there are and #includes (of the form #include "foo.chp") in the program
	while ((i = prgm.find("#include")) != prgm.npos)
	{
		//Find the file name
		open = prgm.find_first_of("\"", i+1);
		close = prgm.find_first_of("\"", open+1);

		ifstream s(prgm.substr(open+1, close-open-1).c_str());
		string f((istreambuf_iterator<char>(s)),
	             istreambuf_iterator<char>());

		//Place the contents of the file where the #include statement was
		prgm = prgm.substr(0, i) + f + prgm.substr(close+1);

	}

	program p(prgm, VERB_TRACE);

	//fclose(log);

}
