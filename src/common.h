/*
 * common.h
 *
 *  Created on: Oct 23, 2012
 *      Author: Ned Bingham and Nicholas Kramer
 *
 *  DO NOT DISTRIBUTE
 */

#ifndef common_h
#define common_h

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <streambuf>
#include <string>
#include <list>
#include <map>
#include <sstream>
#include <algorithm>
#include <vector>
#include <math.h>

using namespace std;

bool ac(char c);
bool nc(char c);
bool oc(char c);
bool sc(char c);


template <typename T>
string to_string(T n)
{
     ostringstream os;
     os << n;
     return os.str();
}

string hex_to_bin(string str);
string dec_to_bin(string str);

size_t find_first_of_l0(string subject, string search, size_t pos = 0);
size_t find_first_of_l0(string subject, list<string> search, size_t pos = 0, list<string> exclude = list<string>());

size_t find_name(string subject, string search, size_t pos = 0);

#define VERB_SUPPRESS	-1
#define VERB_PRS		0
#define VERB_STATES		1
#define VERB_PRSALG		2
#define VERB_STATEVAR	3
#define VERB_PARSE		4
#define VERB_TRACE		5

//State space output customization flags
#define STATESP_CO 1		//Output the statespace to the console
#define STATESP_GR 0		//Output the .dot formatted graph of the statespace
#define CHP_EDGE 1			// If 1, output CHP on graph edges. If 0, output functional labels (loop, conditional, etc)
#define GRAPH_VERT 0		//If 1, the graph will be high to low. Else, it will be left to right
#define GRAPH_DPI 450		//DPI of the output graph

#endif
