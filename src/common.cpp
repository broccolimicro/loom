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

int hex_to_int(sstring str)
{
	int result = 0;
	int mul = 1;
	sstring::reverse_iterator i;

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

sstring hex_to_bin(sstring str)
{
	sstring result = "";
	sstring::iterator i;

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

int dec_to_int(sstring str)
{
	int result = 0;
	int mul = 1;
	sstring::reverse_iterator i;

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

sstring int_to_bin(int dec)
{
	sstring result = "";
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

sstring dec_to_bin(sstring str)
{
	return int_to_bin(dec_to_int(str));
}

unsigned int count_1bits(unsigned int x)
{
    x = x - ((x >> 1) & 0x55555555);
    x = (x & 0x33333333) + ((x >> 2) & 0x33333333);
    x = (x & 0x0F0F0F0F) + ((x >> 4) & 0x0F0F0F0F);
    x = x + (x >> 8);
    x = x + (x >> 16);
    return x & 0x0000003F;
}

unsigned int count_0bits(unsigned int x)
{
	x = x - ((x >> 1) & 0x55555555);
	x = (x & 0x33333333) + ((x >> 2) & 0x33333333);
	x = (x & 0x0F0F0F0F) + ((x >> 4) & 0x0F0F0F0F);
	x = x + (x >> 8);
	x = x + (x >> 16);
    return 32 - (x & 0x0000003F);
}

int powi(int base, int exp)
{
    int result = 1;
    while (exp)
    {
        if (exp & 1)
            result *= base;
        exp >>= 1;
        base *= base;
    }

    return result;
}

uint32_t bitwise_or(uint32_t a, uint32_t b)
{
	return a||b;
}

uint32_t bitwise_and(uint32_t a, uint32_t b)
{
	return a&&b;
}

uint32_t bitwise_not(uint32_t a)
{
	return !a;
}

svector<int> first_combination(int s)
{
	svector<int> result;
	for (int i = 0; i < s; i++)
		result.push_back(i);
	return result;
}

bool next_combination(int S, svector<int> *iter)
{
	for (int i = 1; i <= iter->size(); i++)
	{
		if ((*iter)[iter->size()-i] < S-i)
		{
			(*iter)[iter->size()-i]++;
			for (int j = iter->size()-i+1; j < iter->size(); j++)
				(*iter)[j] = (*iter)[j-1]+1;
			return true;
		}
	}
	return false;
}
