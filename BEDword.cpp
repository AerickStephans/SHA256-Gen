#include "BEDword.h"

bedword::bedword(void)
{
	m_Value.dword = 0;
	m_Value.bytes = (unsigned char *)&m_Value.dword;
	m_Carry = false;
}

bedword::bedword(const unsigned long y)
{
	m_Value.bytes = (unsigned char *)&m_Value.dword;
	m_Value.bytes[0] = (unsigned char)((y & 0xFF000000) >> 24);
	m_Value.bytes[1] = (unsigned char)((y & 0x00FF0000) >> 16);
	m_Value.bytes[2] = (unsigned char)((y & 0x0000FF00) >> 8);
	m_Value.bytes[3] = (unsigned char)(y & 0x000000FF);
	m_Carry = false;
}

// Return the little-endian form of the big-endian value
bedword::operator unsigned long const()
{
	return m_Value.bytes[3] | (m_Value.bytes[2] << 8) | (m_Value.bytes[1] << 16) | (m_Value.bytes[0] << 24);
}

/*
bedword::operator unsigned int const()
{
	return m_Value.bytes[3] | (m_Value.bytes[2] << 8) | (m_Value.bytes[1] << 16) | (m_Value.bytes[0] << 24);
}

bedword::operator int const()
{
	return m_Value.bytes[3] | (m_Value.bytes[2] << 8) | (m_Value.bytes[1] << 16) | (m_Value.bytes[0] << 24);
}
*/

// Sets the big-endian value using a little-endian value
bedword& bedword::operator=(const unsigned long &y)
{
	m_Value.bytes = (unsigned char *)&m_Value.dword;
	m_Value.bytes[0] = (unsigned char)((y & 0xFF000000) >> 24);
	m_Value.bytes[1] = (unsigned char)((y & 0x00FF0000) >> 16);
	m_Value.bytes[2] = (unsigned char)((y & 0x0000FF00) >> 8);
	m_Value.bytes[3] = (unsigned char)(y & 0x000000FF);
	m_Carry = false;

	return *this;
}

// Sets the big-endian value to another big-endian value
bedword& bedword::operator=(const bedword &y)
{
	m_Value.dword = y.GetRawDword();
	m_Carry = y.GetCarryStatus();

	return *this;
}

// Adds two big endian numbers together; x is the current object (this pointer).
// If the sum exceeds the maximum value of a 32-bit unsigned integer, the internal
// carry flag is set.
bedword bedword::operator+(const bedword &y)
{
	// by making the temporary storage 5 bytes, handling an overflow is cleaner
	bedword result;
	int temp[5] = { 0, 0, 0, 0, 0 };
	int i;

	// add the bytes individually
	for (i = 0; i < 4; i++)
	{
		temp[4 - i] += m_Value.bytes[3 - i] + (int)y.GetRawByte(3 - i);

		// did we overflow? if we did, carry the MSB of the result to the next byte
		if (temp[4 - i] > 0xFF)
		{
			temp[4 - i] -= 256;
			temp[3 - i] += 1;
		}
	}

	// store the result and set the carry flag
	for (i = 0; i < 4; i++)
		result.SetRawByte((unsigned char)temp[4 - i], 3 - i);

	if (temp[0] != 0)
		result.SetCarry();
	else
		result.ClearCarry();

	// return the final result
	return result;
}

// Adds two big endian numbers together; x is the current object (this pointer).
// If the sum exceeds the maximum value of a 32-bit unsigned integer, the internal
// carry flag is set.
bedword& bedword::operator+=(const bedword &y)
{
	// by making the temporary storage 5 bytes, handling an overflow is cleaner
	int temp[5] = { 0, 0, 0, 0, 0 };
	int i;

	// add the bytes individually
	for (i = 0; i < 4; i++)
	{
		temp[4 - i] += m_Value.bytes[3 - i] + (int)y.GetRawByte(3 - i);

		// did we overflow? if we did, carry the MSB of the result to the next byte
		if (temp[4 - i] > 0xFF)
		{
			temp[4 - i] -= 256;
			temp[3 - i] += 1;
		}
	}

	// store the result and set the carry flag
	for (i = 0; i < 4; i++)
		m_Value.bytes[3 - i] = temp[4 - i];
	m_Carry = (temp[0] != 0) ? true : false;

	// return the final result
	return *this;
}

// Performs a bitwise exclusive-or between two big-endian number
bedword bedword::operator^(const bedword &y)
{
	bedword result;
	result.SetRawDword(m_Value.dword ^ y.GetRawDword());
	return result;
}

// Performs a bitwise and between two big-endian number
bedword bedword::operator&(const bedword &y)
{
	bedword result;
	result.SetRawDword(m_Value.dword & y.GetRawDword());
	return result;
}

// Performs a bitwise or between two big-endian number
bedword bedword::operator|(const bedword &y)
{
	bedword result;
	result.SetRawDword(m_Value.dword | y.GetRawDword());
	return result;
}

// Performs a bitwise not of a big-endian number
bedword bedword::operator~()
{
	bedword result;
	result.SetRawDword(~m_Value.dword);
	return result;
}

// Performs a right shift between two big-endian number
bedword bedword::operator>>(const int bits)
{
	bedword result;
	result.SetRawDword(m_Value.dword >> bits);
	return result;
}

// Performs a left shift between two big-endian number
bedword bedword::operator<<(const int bits)
{
	bedword result;
	result.SetRawDword(m_Value.dword << bits);
	return result;
}

// Performs a right rotation between two big-endian number
bedword bedword::RotateRight(const int bits)
{
	bedword result = *this << (32 - bits);
	result = result | (*this >> bits);
	return result;
}

// Performs a left shift between two big-endian number
bedword bedword::RotateLeft(const int bits)
{
	bedword result = *this >> (32 - bits);
	result = result | (*this << bits);
	return result;
}
