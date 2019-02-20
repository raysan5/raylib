#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

// macros for packing floats and doubles:
#define pack754_16(f) (pack754((f), 16, 5))
#define pack754_32(f) (pack754((f), 32, 8))
#define pack754_64(f) (pack754((f), 64, 11))
#define unpack754_16(i) (unpack754((i), 16, 5))
#define unpack754_32(i) (unpack754((i), 32, 8))
#define unpack754_64(i) (unpack754((i), 64, 11))

/*
** pack754() -- pack a floating point number into IEEE-754 format
*/
unsigned long long int pack754(long double f, unsigned bits, unsigned expbits)
{
	long double fnorm;
	int         shift;
	long long   sign, exp, significand;
	unsigned    significandbits = bits - expbits - 1; // -1 for sign bit

	if (f == 0.0)
		return 0; // get this special case out of the way

	// check sign and begin normalization
	if (f < 0)
	{
		sign  = 1;
		fnorm = -f;
	}
	else
	{
		sign  = 0;
		fnorm = f;
	}

	// get the normalized form of f and track the exponent
	shift = 0;
	while (fnorm >= 2.0)
	{
		fnorm /= 2.0;
		shift++;
	}
	while (fnorm < 1.0)
	{
		fnorm *= 2.0;
		shift--;
	}
	fnorm = fnorm - 1.0;

	// calculate the binary form (non-float) of the significand data
	significand = fnorm * ((1LL << significandbits) + 0.5f);

	// get the biased exponent
	exp = shift + ((1 << (expbits - 1)) - 1); // shift + bias

	// return the final answer
	return (sign << (bits - 1)) | (exp << (bits - expbits - 1)) | significand;
}

/*
** unpack754() -- unpack a floating point number from IEEE-754 format
*/
long double unpack754(unsigned long long int i, unsigned bits, unsigned expbits)
{
	long double result;
	long long   shift;
	unsigned    bias;
	unsigned    significandbits = bits - expbits - 1; // -1 for sign bit

	if (i == 0)
		return 0.0;

	// pull the significand
	result = (i & ((1LL << significandbits) - 1)); // mask
	result /= (1LL << significandbits); // convert back to float
	result += 1.0f; // add the one back on

	// deal with the exponent
	bias  = (1 << (expbits - 1)) - 1;
	shift = ((i >> significandbits) & ((1LL << expbits) - 1)) - bias;
	while (shift > 0)
	{
		result *= 2.0;
		shift--;
	}
	while (shift < 0)
	{
		result /= 2.0;
		shift++;
	}

	// sign it
	result *= (i >> (bits - 1)) & 1 ? -1.0 : 1.0;

	return result;
}

/*
** packi16() -- store a 16-bit int into a char buffer (like htons())
*/
void packi16(unsigned char* buf, unsigned int i)
{
	*buf++ = i >> 8;
	*buf++ = i;
}

/*
** packi32() -- store a 32-bit int into a char buffer (like htonl())
*/
void packi32(unsigned char* buf, unsigned long int i)
{
	*buf++ = i >> 24;
	*buf++ = i >> 16;
	*buf++ = i >> 8;
	*buf++ = i;
}

/*
** packi64() -- store a 64-bit int into a char buffer (like htonl())
*/
void packi64(unsigned char* buf, unsigned long long int i)
{
	*buf++ = i >> 56;
	*buf++ = i >> 48;
	*buf++ = i >> 40;
	*buf++ = i >> 32;
	*buf++ = i >> 24;
	*buf++ = i >> 16;
	*buf++ = i >> 8;
	*buf++ = i;
}

/*
** unpacki16() -- unpack a 16-bit int from a char buffer (like ntohs())
*/
int unpacki16(unsigned char* buf)
{
	unsigned int i2 = ((unsigned int) buf[0] << 8) | buf[1];
	int          i;

	// change unsigned numbers to signed
	if (i2 <= 0x7fffu)
	{
		i = i2;
	}
	else
	{
		i = -1 - (unsigned int) (0xffffu - i2);
	}

	return i;
}

/*
** unpacku16() -- unpack a 16-bit unsigned from a char buffer (like ntohs())
*/
unsigned int unpacku16(unsigned char* buf)
{
	return ((unsigned int) buf[0] << 8) | buf[1];
}

/*
** unpacki32() -- unpack a 32-bit int from a char buffer (like ntohl())
*/
long int unpacki32(unsigned char* buf)
{
	unsigned long int i2 = ((unsigned long int) buf[0] << 24) | ((unsigned long int) buf[1] << 16) | ((unsigned long int) buf[2] << 8) | buf[3];
	long int          i;

	// change unsigned numbers to signed
	if (i2 <= 0x7fffffffu)
	{
		i = i2;
	}
	else
	{
		i = -1 - (long int) (0xffffffffu - i2);
	}

	return i;
}

/*
** unpacku32() -- unpack a 32-bit unsigned from a char buffer (like ntohl())
*/
unsigned long int unpacku32(unsigned char* buf)
{
	return ((unsigned long int) buf[0] << 24) | ((unsigned long int) buf[1] << 16) | ((unsigned long int) buf[2] << 8) | buf[3];
}

/*
** unpacki64() -- unpack a 64-bit int from a char buffer (like ntohl())
*/
long long int unpacki64(unsigned char* buf)
{
	unsigned long long int i2 = ((unsigned long long int) buf[0] << 56) | ((unsigned long long int) buf[1] << 48) | ((unsigned long long int) buf[2] << 40) | ((unsigned long long int) buf[3] << 32) | ((unsigned long long int) buf[4] << 24) | ((unsigned long long int) buf[5] << 16) | ((unsigned long long int) buf[6] << 8) | buf[7];
	long long int          i;

	// change unsigned numbers to signed
	if (i2 <= 0x7fffffffffffffffu)
	{
		i = i2;
	}
	else
	{
		i = -1 - (long long int) (0xffffffffffffffffu - i2);
	}

	return i;
}

/*
** unpacku64() -- unpack a 64-bit unsigned from a char buffer (like ntohl())
*/
unsigned long long int unpacku64(unsigned char* buf)
{
	return ((unsigned long long int) buf[0] << 56) | ((unsigned long long int) buf[1] << 48) | ((unsigned long long int) buf[2] << 40) | ((unsigned long long int) buf[3] << 32) | ((unsigned long long int) buf[4] << 24) | ((unsigned long long int) buf[5] << 16) | ((unsigned long long int) buf[6] << 8) | buf[7];
}