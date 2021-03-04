#ifndef BEDWORD_H
#define BEDWORD_H

class bedword
{
public:
	bedword(void);
	bedword(const unsigned long y);

	// return the raw data from our data value
	void SetRawDword(unsigned long val) { m_Value.dword = val; }
	void SetRawByte(unsigned char val, int n) { if (n >= 0 && n <= 3) m_Value.bytes[n] = val; }
	unsigned long GetRawDword(void) const { return m_Value.dword; }
	unsigned char GetRawByte(int n) const { if (n >= 0 && n <= 3) return m_Value.bytes[n]; else return 0; }

	// handle the carry flag
	void SetCarry(void) { m_Carry = true; }
	void ClearCarry(void) { m_Carry = false; }
	bool GetCarryStatus(void) const { return m_Carry; }

	// shift and rotation operators
	bedword operator<<(const int bits);
	bedword operator>>(const int bits);
	bedword RotateLeft(const int bits);
	bedword RotateRight(const int bits);

	// operators between two big-endian numbers
	bedword& operator=(const bedword &y);
	bedword& operator+=(const bedword &y);
	bedword operator+(const bedword &y);
	bedword operator^(const bedword &y);
	bedword operator&(const bedword &y);
	bedword operator|(const bedword &y);
	bedword operator~();

	// operators between big-endian numbers and little endian numbers
	operator unsigned long const();
	bedword& operator=(const unsigned long &y);

protected:
	// we need to be able to acces individual bytes as well as the whole dword to work with big endian values on a little endian machine
	struct
	{
		unsigned long dword;
		unsigned char *bytes;
	} m_Value;

	// arithmetic flags
	bool m_Carry;
};

#endif
