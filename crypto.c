#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "ec.c"

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

void eccp(ecc *e, char *xs)
{
	int psiz = (e->p)->size;
	bnum *x = bninit(psiz), *y = bninit(psiz), *t = bninit(psiz);
	
	bnum *xa = bninit(psiz * 4), *xx = bninit(psiz * 4);
	bnum *yy = bninit(psiz * 4), *vv = bninit(psiz * 4);
	
	if (xs != NULL) { t = bndec(xs); }
	else { t = bnrnd(psiz); }
	bncopy(t, x);
	
	while (1)
	{
		// yy = ((x * (x * x)) + (a * (x * x)) + x) % m
		bnzero(xx); bnmul(x, x, xx);
		bnzero(xa); bnmul(xx, e->a, xa);
		bnzero(vv); bnmul(x, xx, vv);
		
		bnzero(yy); bnadd(vv, xa, yy, 1); bnadd(yy, x, vv, 1);
		bndiv(vv, e->p, xx, t);
		// todo: divide t by b?
		
		bnout("x: ", x, "\n");
		bnout("y^2: ", t, "\n");
		
		if (sqrtmod(t, e->p, y) == 0) { break; }
		
		if (x != NULL) { bnfree(x); }
		x = bnrnd(psiz);
	}
	
	bnout("y: ", y, "\n");
	// note: another y point == p - y
	
	bnzero(yy); bncopy(t, yy);
	bnzero(vv); bnmul(y, y, vv);
	bnzero(xx); bndiv(vv, e->p, xa, xx);
	
	bnfree(xa); bnfree(vv); bnfree(t);
	
	if (bncmp(xx, yy) != 0)
	{
		bnout("y^2: ",xx," != "); bnout("y^2: ",yy,"\n");
		exit(0);
	}
	
	bnfree(xx); bnfree(yy);
}

/*
Curve25519: a=486662, b=1, p=2^255 - 19
ECC DH : a * (b * P) == abP == baP == b * (a * P)
*/

char *ecdh(ecc *e, char *n, int d)
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
	
	if (d == 1)
	{
		char v[2050];
		bzero(v, 2050 * sizeof(char));
		strncat(v, "    "       , max(0, 2048 - strlen(v)));
		strncat(v, r            , max(0, 2048 - strlen(v)));
		strncat(v, "\n      * P", max(0, 2048 - strlen(v)));
		printf("\n");
		ecout(1,v,e,"\n\n");
		ecout(0,"      = Q",f,"\n");
		printf("\n");
	}
	
	bnfree(e->x); e->x = bndup(f->x);
	bnfree(e->y); e->y = bndup(f->y);
	ecfree(f);
	
	return r;
}

/*
ECC - Private Sign / Public Verify

* initialize
- generate secret key          : d
- publish                      : P, dP

* private sign
- hash message                 : h = SHA256(m)
- choose secret random         : k
- calculate public multipliers : r = ((k * h * d) % n)
                               : s = ((k * h) + (r * d))
- publish                      : kP, r, s

* public verify
- hkP + rdP                         == sP
- ((h * k) + (r * d)) * P           == ((k * h) + (r * d)) * P
- ((h * k) + ((k * h * d) * d)) * P == ((k * h) + ((k * h * d) * d)) * P
*/

char *ecsig(ecc *e, char *dd, ecc *dp)
{
	bnum *d = bndec(dd);
	if (d == NULL) {  }
	return NULL;
}

/*
ECC ElGamal - Public Encrypt / Private Decrypt

* initialize
- generate secret integer    : d
- publish                    : P, dP

* public encrypt
- generate secret key        : t = (SHA256(pwd) || tmpkey)
- generate secret multiplier : k = rand()
- public key encrypt         : r = k * P        = kP
                             : s = t + (k * dP) = t + kdP = kdP(x + t, y + t)
- encrypt optional message   : i = rand()
                             : e = AES256CBC(m, i, t)
- publish                    : [ (r, s) , (i, e) ]

* private decrypt
- private key decrypt        : u = s - (d * r)
                                 = (t + kdP) - (d * kP)
                                 = kdP(x + t, y + t) - dkP(x, y)
                                 = kdP(x - x + t)
                                 = t
*/

void ecpub()
{
	
}

int main(int argc, char **argv)
{
	char *a = "486662", *b = "1", *p = "57896044618658097711785492504343953926634992332820282019728792003956564819949";
	char *x = "9", *y = "43114425171068552920764898935933967039370386198203806730763910166200978582548";
	char *n, *m;
	bnum *t, *u, *v, *w;
	ecc *e, *f;
	
	t = bndec(p);
	u = bninit(t->size); w = bndec(x); bncopy(w, u); bnfree(w);
	v = bninit(t->size); w = bndec(y); bncopy(w, v); bnfree(w);
	e = ecinit(bndec(a), bndec(b), t, u, v);
	
	if (strcmp(argv[1], "pgen") == 0)
	{
		if (argc > 2) { eccp(e, argv[2]); }
		else { eccp(e, NULL); }
	}
	
	if (strcmp(argv[1], "pmul") == 0)
	{
		ecdh(e, argv[2], 1);
	}
	
	if (strcmp(argv[1], "ecdh") == 0)
	{
		f = ecdup(e);
		
		n = ecdh(e, NULL, 1);
		m = ecdh(f, NULL, 1);
		
		t = e->x; u = e->y;
		e->x = f->x; e->y = f->y;
		f->x = t; f->y = u;
		
		ecdh(e, n, 1);
		ecdh(f, m, 1);
		
		ecfree(f);
	}
	
	if (strcmp(argv[1], "psig") == 0)
	{
		
	}
	
	ecfree(e);
	
	return 0;
}
