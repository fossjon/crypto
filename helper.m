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

char *ecdh(ecc *e, char *n, int o)
{
	char *r = n;
	bnum *m;
	ecc *f;
	
	if (r == NULL)
	{
		m = bnrnd((e->p)->size);
		r = bnstr(m);
	}
	
	else
	{
		m = bndec(n);
	}
	
	f = ecdup(e);
	pmul(m, e, f);
	
	bnfree(e->x); e->x = bndup(f->x);
	bnfree(e->y); e->y = bndup(f->y);
	bnfree(m);
	ecfree(f);
	
	return r;
}
