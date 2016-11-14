struct sha256ctx {
	int b;
	unsigned int a[8], h[8], l[2];
	unsigned int k[64], w[64];
};

#define sha256 struct sha256ctx

unsigned int rr(unsigned int a, int b) { return (((a << (32 - b)) | (a >> b)) & 0xffffffff); }
unsigned int rs(unsigned int a, int b) { return ((a >> b) & 0xffffffff); }
unsigned int add(unsigned int a, unsigned int b) { return ((a + b) & 0xffffffff); }

void sha256init(sha256 *sctx)
{
	unsigned int hh[8] = {
		0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
		0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
	};
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
	int x;
	sctx->b = 0; sctx->l[0] = 0; sctx->l[1] = 0;
	for (x = 0; x < 8; ++x) { sctx->h[x] = hh[x]; sctx->a[x] = 0; }
	for (x = 0; x < 64; ++x) { sctx->k[x] = kk[x]; sctx->w[x] = 0; }
}

void sha2core(sha256 *sctx)
{
	int y, z;
	unsigned int s0, s1, t1, t2, ch, ma;
	for (y = 16; y < 64; ++y)
	{
		s0 = (rr(sctx->w[y-15],7) ^ rr(sctx->w[y-15],18) ^ rs(sctx->w[y-15],3));
		s1 = (rr(sctx->w[y-2],17) ^ rr(sctx->w[y-2],19) ^ rs(sctx->w[y-2],10));
		sctx->w[y] = add(add(sctx->w[y-16], s0), add(sctx->w[y-7], s1));
	}
	for (y = 0; y < 8; ++y) { sctx->a[y] = sctx->h[y]; }
	for (y = 0; y < 64; ++y)
	{
		s1 = (rr(sctx->h[4],6) ^ rr(sctx->h[4],11) ^ rr(sctx->h[4],25));
		ch = ((sctx->h[4] & sctx->h[5]) ^ ((~(sctx->h[4])) & sctx->h[6]));
		t1 = add(add(add(sctx->h[7], s1), add(ch, sctx->k[y])), sctx->w[y]);
		s0 = (rr(sctx->h[0],2) ^ rr(sctx->h[0],13) ^ rr(sctx->h[0],22));
		ma = ((sctx->h[0] & sctx->h[1]) ^ (sctx->h[0] & sctx->h[2]) ^ (sctx->h[1] & sctx->h[2]));
		t2 = add(s0, ma);
		for (z = 7; z >= 0; --z)
		{
			if (z == 4) { sctx->h[z] = add(sctx->h[z-1], t1); }
			else if (z == 0) { sctx->h[z] = add(t1, t2); }
			else { sctx->h[z] = sctx->h[z-1]; }
		}
	}
	for (y = 0; y < 8; ++y) { sctx->h[y] = add(sctx->h[y], sctx->a[y]); }
	sctx->b = 0;
	for (y = 0; y < 16; ++y) { sctx->w[y] = 0; }
}

void sha256update(sha256 *sctx, unsigned char *s, unsigned int m)
{
	unsigned int x = 0;
	for (x = 0; x < m; ++x)
	{
		int i = (sctx->b / 32), j = (24 - (sctx->b % 32));
		sctx->w[i] = (sctx->w[i] | (s[x] << j));
		sctx->b += 8;
		if (sctx->b == 512) { sha2core(sctx); }
	}
	unsigned int t = sctx->l[0];
	sctx->l[0] += (m * 8);
	if (sctx->l[0] <= t) { sctx->l[1] += 1; }
}

void sha256final(sha256 *sctx, char *h)
{
	int x, y, i, j;
	char *c = "0123456789abcdef";
	
	if (sctx->b >= (512 - 64)) { sha2core(sctx); }
	
	i = (sctx->b / 32); j = (24 - (sctx->b % 32));
	
	sctx->w[i] = (sctx->w[i] | (0x80 << j));
	sctx->w[14] = sctx->l[1]; sctx->w[15] = sctx->l[0];
	sha2core(sctx);
	
	i = 0;
	for (x = 0; x < 8; ++x)
	{
		for (y = 0; y <= 28; y += 4)
		{
			h[i] = c[(sctx->h[x] >> (28 - y)) & 0xf];
			++i;
		}
	}
	h[i] = '\0';
}
