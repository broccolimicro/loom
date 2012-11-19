/*
 * common.cpp
 *
 *  Created on: Oct 23, 2012
 *      Author: Ned Bingham and Nicholas Kramer
 *
 *  DO NOT DISTRIBUTE
 */

#include "common.h"

/*Is this character a legal name starter character?
 */
bool ac(char c)
{
	return ((c >= 'a' && c <= 'z') ||
			(c >= 'A' && c <= 'Z') );
}


/* Is this character a character that is legal to have
 * in a type name or variable name? a-z A-Z 0-9 _
 */
bool nc(char c)
{
	return ((c >= 'a' && c <= 'z') ||
			(c >= 'A' && c <= 'Z') ||
			(c >= '0' && c <= '9') ||
			(c == '_'));
}

/* Is this character an operator?
 *
 */
bool oc(char c)
{
	return (c == ':' ||
			c == '=' ||
			c == '|' ||
			c == '&' ||
			c == '~' ||
			c == '>' ||
			c == '<' ||
			c == ';' ||
			c == '*' ||
			c == '[' ||
			c == ']' ||
			c == '(' ||
			c == ')' ||
			c == '{' ||
			c == '}' ||
			c == '+' ||
			c == '-' ||
			c == '!' ||
			c == '?' ||
			c == '@' ||
			c == '#');
}

/* Is this character whitespace?
 *
 */
bool sc(char c)
{
	return (c == ' '  ||
			c == '\t' ||
			c == '\n' ||
			c == '\r');
}

// BIG ENDIAN

int hex_to_int(string str)
{
	int result = 0;
	int mul = 1;
	string::reverse_iterator i;

	for (i = str.rbegin(), mul = 1; i != str.rend(); i++, mul *= 16)
	{
		switch (*i)
		{
		case '0': result += 0; break;
		case '1': result += mul; break;
		case '2': result += 2*mul; break;
		case '3': result += 3*mul; break;
		case '4': result += 4*mul; break;
		case '5': result += 5*mul; break;
		case '6': result += 6*mul; break;
		case '7': result += 7*mul; break;
		case '8': result += 8*mul; break;
		case '9': result += 9*mul; break;
		case 'a': result += 10*mul; break;
		case 'b': result += 11*mul; break;
		case 'c': result += 12*mul; break;
		case 'd': result += 13*mul; break;
		case 'e': result += 14*mul; break;
		case 'f': result += 15*mul; break;
		case 'A': result += 10*mul; break;
		case 'B': result += 11*mul; break;
		case 'C': result += 12*mul; break;
		case 'D': result += 13*mul; break;
		case 'E': result += 14*mul; break;
		case 'F': result += 15*mul; break;
		default:  return 0;
		}
	}

	return result;
}

string hex_to_bin(string str)
{
	string result = "";
	string::iterator i;

	for (i = str.begin(); i != str.end(); i++)
	{
		switch (*i)
		{
		case '0': result += "0000"; break;
		case '1': result += "0001"; break;
		case '2': result += "0010"; break;
		case '3': result += "0011"; break;
		case '4': result += "0100"; break;
		case '5': result += "0101"; break;
		case '6': result += "0110"; break;
		case '7': result += "0111"; break;
		case '8': result += "1000"; break;
		case '9': result += "1001"; break;
		case 'a': result += "1010"; break;
		case 'b': result += "1011"; break;
		case 'c': result += "1100"; break;
		case 'd': result += "1101"; break;
		case 'e': result += "1110"; break;
		case 'f': result += "1111"; break;
		case 'A': result += "1010"; break;
		case 'B': result += "1011"; break;
		case 'C': result += "1100"; break;
		case 'D': result += "1101"; break;
		case 'E': result += "1110"; break;
		case 'F': result += "1111"; break;
		default:  return "";
		}
	}

	return result;
}

int dec_to_int(string str)
{
	int result = 0;
	int mul = 1;
	string::reverse_iterator i;

	for (i = str.rbegin(), mul = 1; i != str.rend(); i++, mul *= 10)
	{
		switch (*i)
		{
		case '0': result += 0; break;
		case '1': result += mul; break;
		case '2': result += 2*mul; break;
		case '3': result += 3*mul; break;
		case '4': result += 4*mul; break;
		case '5': result += 5*mul; break;
		case '6': result += 6*mul; break;
		case '7': result += 7*mul; break;
		case '8': result += 8*mul; break;
		case '9': result += 9*mul; break;
		default:  return 0;
		}
	}

	return result;
}

string int_to_bin(int dec)
{
	string result = "";
	int i = 0;

	if (dec == 0)
		return "0";

	while ((dec&0x80000000) == 0)
	{
		i++;
		dec <<= 1;
	}

	for (; i < 32; i++)
	{
		result += char('0' + ((dec&0x80000000) > 1));
		dec <<= 1;
	}

	return result;
}

string dec_to_bin(string str)
{
	return int_to_bin(dec_to_int(str));
}


