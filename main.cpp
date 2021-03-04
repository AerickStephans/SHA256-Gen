#include <iostream>
using namespace std;

const unsigned long g_Karray[64] = {
	0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
	0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
	0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
	0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
	0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
	0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
	0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
	0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

// the eight state variables for generating the hash
const unsigned long gle_Harray[8] = {
	0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
	0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
};

unsigned long g_Harray[8];

// initialize the arrays
void InitArrays(void)
{
	int i;

	for (i = 0; i < 8; i++)
	{
		g_Harray[i] = gle_Harray[i];
	}
}

// rotates A to the right by n bits
unsigned long RotR(unsigned long A, int n)
{
	unsigned long temp = A << (32 - n);
	return (A >> n) | temp;
}

// implements the Ch(X,Y,Z) function
unsigned long Ch(unsigned long X, unsigned long Y, unsigned long Z)
{
	return (X & Y) ^ (~X & Z);
}

// implements the Maj(X,Y,Z) function
unsigned long Maj(unsigned long X, unsigned long Y, unsigned long Z)
{
	return (X & Y) ^ (X & Z) ^ (Y & Z);
}

// implements the big-sigma-0(X) function
unsigned long BigSigma0(unsigned long X)
{
	return RotR(X, 2) ^ RotR(X, 13) ^ RotR(X, 22);
}

// implements the big-sigma-1(X) function
unsigned long BigSigma1(unsigned long X)
{
	return RotR(X, 6) ^ RotR(X, 11) ^ RotR(X, 25);
}

// implements the small-sigma-0(X) function
unsigned long SmallSigma0(unsigned long X)
{
	return RotR(X, 7) ^ RotR(X, 18) ^ (X >> 3);
}

// implements the small-sigma-1(X) function
unsigned long SmallSigma1(unsigned long X)
{
	return RotR(X, 17) ^ RotR(X, 19) ^ (X >> 10);
}

// extends the message to a multiple of 64 bytes. WARNING: msg will be a dangling pointer unless
// you reassign the pointer used before using again
unsigned char *ExtendMessage(unsigned char *msg, int length)
{
	unsigned char *new_msg = NULL;
	// if the message is not a multiple of 64 bytes, extend it to a multiple of
	// 64; otherwise add a 64 byte block to it
	int new_length = (length + 9) + (64 - ((length + 9) % 64));
	int i;

	// allocate some memory for the new message
	new_msg = new unsigned char[new_length];
	if (!new_msg) return NULL;

	// copy the old message to the new one
	for (i = 0; i < length; i++)
	{
		new_msg[i] = msg[i];
	}
	
	// add the extra 1 bit and 0 bits to the end of the message
	// note that the upper 4 bytes of the length are assumed to be 0,
	// so only 32-bit message lengths are supported
	new_msg[i++] = 0x80;
	while (i < new_length - 4)
	{
		new_msg[i] = 0;
		i++;
	}

	// append the length of the message as a 64-bit big endian number
	new_msg[i++] = ((length * 8) & 0xFF000000) >> 24;
	new_msg[i++] = ((length * 8) & 0xFF0000) >> 16;
	new_msg[i++] = ((length * 8) & 0xFF00) >> 8;
	new_msg[i++] = (length * 8) & 0xFF;

	// free the old message and set it to the new one
	delete msg;
	return new_msg;
}

// the current working array for decomposing each block
unsigned long g_Warray[64];

// initialize the W blocks with the given data; assumes that the block
// pointer is pointing to a correctly size block
void InitWBlocks(unsigned char *block)
{
	int i;
	unsigned long b0, b1, b2, b3;

	// copy the 16 bytes in the block to the W array
	for (i = 0; i < 16; i++)
	{
		b3 = block[i * 4];
		b2 = block[(i * 4) + 1];
		b1 = block[(i * 4) + 2];
		b0 = block[(i * 4) + 3];
		g_Warray[i] = b0 | (b1 << 8) | (b2 << 16) | (b3 << 24);
	}

	// calculate the other values
	for (i = 16; i < 64; i++)
	{
		g_Warray[i] = SmallSigma1(g_Warray[i - 2]) + g_Warray[i - 7] + SmallSigma0(g_Warray[i - 15]) + g_Warray[i - 16];
	}
}

// hash the specified block
void SHA256Hash(unsigned char *data, int length)
{
	// the current working set of data
	unsigned long a = 0, b = 0, c = 0, d = 0, e = 0, f = 0, g = 0, h = 0;
	unsigned long T1 = 0, T2 = 0, temp = 0;
	int t = 0, i = 0, N = length / 64;

	// process every block in the message
	for (t = 0; t < N; t++)
	{
		// set up the W blocks and the working data set
		InitWBlocks(data + (t * 64));
		a = g_Harray[0];
		b = g_Harray[1];
		c = g_Harray[2];
		d = g_Harray[3];
		e = g_Harray[4];
		f = g_Harray[5];
		g = g_Harray[6];
		h = g_Harray[7];

		// 64 rounds of SHA-256
		for (i = 0; i < 64; i++)
		{
			T1 = h + BigSigma1(e) + Ch(e, f, g) + g_Karray[i] + g_Warray[i];
			T2 = BigSigma0(a) + Maj(a, b, c);
			h = g;
			g = f;
			f = e;
			e = d + T1;
			d = c;
			c = b;
			b = a;
			a = T1 + T2;
		}

		// update the H block values
		g_Harray[0] += a;
		g_Harray[1] += b;
		g_Harray[2] += c;
		g_Harray[3] += d;
		g_Harray[4] += e;
		g_Harray[5] += f;
		g_Harray[6] += g;
		g_Harray[7] += h;
	}
}

int main(int argc, char *argv[])
{
	const char *test_msg = "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq";
	int i, length = strlen(test_msg);
	unsigned char *msg = new unsigned char[length];
	unsigned long *new_msg = NULL;

	InitArrays();

	for(i = 0; i < length; i++)
		msg[i] = test_msg[i];

	for (i = 0; i < length; i++)
	{
		cout << hex << (int)msg[i] << " ";
	}
	cout << dec << endl;

	msg = ExtendMessage(msg, length);
	new_msg = (unsigned long *)msg;

	for (i = 0; i < (length + 9) + (64 - ((length + 9) % 64)); i++)
	{
		cout << hex << (int)msg[i] << " ";
	}
	cout << dec << endl << endl;

	SHA256Hash((unsigned char *)new_msg, (length + 9) + (64 - ((length + 9) % 64)));

	for (i = 0; i < 8; i++)
	{
		cout << hex << (int)g_Harray[i] << " ";
	}
	cout << dec << endl;
	
	new_msg = NULL;
	delete msg;

	cout << "Press enter to continue...";
	cin.get();

	return 0;
}
