/*
 * common.h
 *
 *  Created on: Oct 23, 2012
 *      Author: Ned Bingham
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

#endif
