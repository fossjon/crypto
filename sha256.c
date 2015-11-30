#include <stdio.h>
#include <string.h>
int b = 0;
unsigned int a[8], h[8], l[2];
unsigned int hh[8] = {
	0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
	0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
};
unsigned int k[64], w[64];
unsigned int kk[64] = {
	0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
	0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
	0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
	0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
	0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
	0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
	0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
	0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

unsigned int rr(unsigned int a, int b) { return ((a << (32 - b)) | (a >> b)); }
unsigned int rs(unsigned int a, int b) { return (a >> b); }

void sha256init()
{
	int x;
	b = 0; l[0] = 0; l[1] = 0;
	for (x = 0; x < 8; ++x) { h[x] = hh[x]; a[x] = 0; }
	for (x = 0; x < 64; ++x) { k[x] = kk[x]; w[x] = 0; }
}

void sha2core()
{
	int y;
	for (y = 0; y < 16; ++y) { printf("%08x", w[y]); } printf("\n");
	unsigned int s0, s1, t1, t2, ch, ma;
	for (y = 16; y < 64; ++y)
	{
		s0 = (rr(w[y-15],7) ^ rr(w[y-15],18) ^ rs(w[y-15],3));
		s1 = (rr(w[y-2],17) ^ rr(w[y-2],19) ^ rs(w[y-2],10));
		w[y] = (w[y-16] + s0 + w[y-7] + s1);
	}
	for (y = 0; y < 8; ++y) { a[y] = h[y]; }
	for (y = 0; y < 64; ++y)
	{
		s1 = (rr(h[4],6) ^ rr(h[4],11) ^ rr(h[4],25));
		ch = ((h[4] & h[5]) ^ ((~h[4]) & h[6]));
		t1 = (h[7] + s1 + ch + k[y] + w[y]);
		s0 = (rr(h[0],2) ^ rr(h[0],13) ^ rr(h[0],22));
		ma = ((h[0] & h[1]) ^ (h[0] & h[2]) ^ (h[1] & h[2]));
		t2 = (s0 + ma);
	}
	for (y = 7; y >= 0; --y)
	{
		if (y == 4) { h[y] = (h[y-1] + t1); }
		else if (y == 0) { h[y] = (t1 + t2); }
		else { h[y] = h[y-1]; }
	}
	for (y = 0; y < 8; ++y) { h[y] = (h[y] + a[y]); }
	b = 0;
	for (y = 0; y < 16; ++y) { w[y] = 0; }
}

void sha256update(char *s, unsigned int m)
{
	unsigned int x = 0;
	for (x = 0; x < m; ++x)
	{
		int i = (b / 32), j = (24 - (b % 32));
		w[i] = (w[i] | (s[x] << j));
		b += 8;
		if (b == 512) { sha2core(); }
	}
	unsigned int t = l[0];
	l[0] += (m * 8);
	if (l[0] <= t) { l[1] += 1; }
}

void sha256final()
{
	if (b >= (512 - 64)) { sha2core(); }
	int i = (b / 32), j = (24 - (b % 32));
	w[i] = (w[i] | (0x80 << j));
	w[14] = l[1]; w[15] = l[0];
	sha2core();
}

int main(int argc, char **argv)
{
	int x;
	sha256init();
	sha256update(argv[1], strlen(argv[1]));
	sha256final();
	for (x = 0; x < 8; ++x) { printf("%08x", h[x]); }
	printf("\n");
	return 0;
}
