/* c helper swift functions */

bnum *getx(ecc *e) { return e->x; }
bnum *gety(ecc *e) { return e->y; }

ecc *eccryp()
{
	char *a = "486662", *b = "1", *p = "57896044618658097711785492504343953926634992332820282019728792003956564819949";
	char *x = "9", *y = "43114425171068552920764898935933967039370386198203806730763910166200978582548";
	bnum *t, *u, *v, *w;
	ecc *e;
	
	t = bndec(p);
	u = bninit(t->size); w = bndec(x); bncopy(w, u); bnfree(w);
	v = bninit(t->size); w = bndec(y); bncopy(w, v); bnfree(w);
	e = ecinit(bndec(a), bndec(b), t, u, v);
	
	return e;
}

bnum *bnrnd(int size)
{
	FILE *f;
	bnum *r;
	
	r = bninit(size);
	f = fopen("/dev/urandom", "r");
	fread(r->nums, sizeof(unsigned int), size - 1, f);
	fclose(f);
	r->leng = size;
	while ((r->leng > 1) && (r->nums[r->leng - 1] == 0)) { r->leng -= 1; }
	
	return r;
}

/*
 Curve25519: a=486662, b=1, p=2^255 - 19
 ECC DH: o * (n * P) == onP == noP == n * (o * P)
 */

char *ecdh(ecc *e, const char *n)
{
	char *r = (char *)n;
	bnum *m;
	ecc *f;
	
	if (r == NULL)
	{
		m = bnrnd((e->p)->size);
		r = bnstr(m);
	}
	
	else
	{
		m = bndec((char *)n);
	}
	
	f = ecdup(e);
	pmul(m, e, f);
	
	(f->x)->leng = min((f->x)->leng, (e->p)->size); bncopy(f->x, e->x);
	(f->y)->leng = min((f->y)->leng, (e->p)->size); bncopy(f->y, e->y);
	bnfree(m);
	ecfree(f);
	
	return r;
}

void setexy(ecc *e, const char *x, const char *y)
{
	bnum *t;
	t = bndec((char *)x); t->leng = min(t->leng, (e->p)->size);
	bncopy(t, e->x); bnfree(t);
	t = bndec((char *)y); t->leng = min(t->leng, (e->p)->size);
	bncopy(t, e->y); bnfree(t);
}

char *shash(const char *m)
{
	char *o = malloc(256);
	sha256 hobj;
	sha256init(&hobj);
	sha256update(&hobj, (char *)m, (unsigned int)strlen(m));
	sha256final(&hobj, o);
	return o;
}

char *sencr(const char *i, const char *m, const char *k)
{
	int x, y, z = 0;
	unsigned long ilen = strlen(i), mlen = strlen(m), klen = strlen(k);
	unsigned char ivn[16], msg[16], key[256];
	char *hex = "0123456789abcdef";
	char *enc = malloc(3 * strlen(m));
	
	bzero(key, 256);
	for (x = 0; ((x + 1) < 32) && ((x + 1) < klen); x += 2)
	{
		for (y = 0; y < 16; ++y)
		{
			if (k[x] == hex[y]) { key[x / 2] |= (y << 4); }
			if (k[x + 1] == hex[y]) { key[x / 2] |= y; }
		}
	}
	aes256keys(key);
	
	for (x = 0; x < 16; ++x)
	{
		ivn[x] = 0;
		if (x < ilen) { ivn[x] = i[x]; }
	}
	
	for (x = 0; x < mlen; x += 16)
	{
		for (y = 0; y < 16; ++y)
		{
			msg[y] = 0;
			if ((x + y) < mlen) { msg[y] = m[x + y]; }
			msg[y] ^= ivn[y];
		}
		aes256core(msg, key, 0);
		for (y = 0; y < 16; ++y)
		{
			enc[z] = hex[(msg[y] >> 4) & 0xf]; ++z;
			enc[z] = hex[msg[y] & 0xf]; ++z;
			ivn[y] = msg[y];
		}
	}
	enc[z] = 0;
	
	return enc;
}
