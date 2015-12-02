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
	bnum *x = bninit(psiz), *y = bninit(psiz), *t;
	
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
		
		printf("\n");
		bnout("x: ", x, "\n");
		bnout("y^2: ", t, "\n");
		
		if (sqrtmod(t, e->p, y) == 0) { break; }
		
		if (x != NULL) { bnfree(x); }
		x = bnrnd(psiz);
	}
	
	bnout("y: ", y, "\n\n");
	// note: another y point == p - y
	
	bnzero(yy); bncopy(t, yy);
	bnzero(vv); bnmul(y, y, vv);
	bnzero(xx); bndiv(vv, e->p, xa, xx);
	
	bnfree(xa); bnfree(vv); bnfree(t);
	
	if (bncmp(xx, yy) != 0)
	{
		bnout("y^2: ",xx," != "); bnout("y^2: ",yy,"\n");
	}
	
	bnfree(xx); bnfree(yy);
	bnfree(x); bnfree(y);
}

/*
Curve25519: a=486662, b=1, p=2^255 - 19
ECC DH : a * (b * P) == abP == baP == b * (a * P)
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
	
	if (o == 1)
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
	bnfree(m);
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
- secret unique multiplier     : k
- calculate public multipliers : (x, y) = kP(x, y)
                               : r = (((k * h) + (x * d)) % n)
                               : s = (h + k + d)
                               : while (r < s) do r = (r + s)
                               : k = (k + (r - s))
- publish                      : kP, r

* public verify
- hP + kP + dP == rP
*/

int ecsig(ecc *e, bnum *d, ecc *dp, char *hh, ecc *kp, bnum *r, int o)
{
	int a = 0, psiz = (e->p)->size;
	bnum *k, *h;
	bnum *t = bninit(psiz * 5), *u = bninit(psiz * 5), *v = bninit(psiz * 5), *w = bninit(psiz * 5);
	ecc *hp = ecdup(e), *hkp = ecdup(e), *hkdp = ecdup(e), *rp = ecdup(e);
	ect *tadd = etinit(e);
	
	if (hh != NULL)
	{
		if (((kp->x)->leng == 1) && ((kp->x)->nums[0] == 0))
		{
			if ((d->leng == 1) && (d->nums[0] == 0))
			{
				k = bnrnd(psiz); bncopy(k, d); bnfree(k);
				pmul(d, e, dp);
				
				if (o == 1)
				{
					bnout("d=", d, "\n");
					ecout(0, "P=", e, "\n");
					ecout(0, "dP=", dp, "\n\n");
				}
			}
		}
		
		h = bndec(hh);
		
		if (o == 1) { bnout("h=", h, "\n\n"); }
		
		if (((kp->x)->leng == 1) && ((kp->x)->nums[0] == 0))
		{
			k = bnrnd(psiz);
			pmul(k, e, kp);
			
			if (o == 1) { bnout("k=", k, "\n"); }
			
			bnzero(t); bnmul(k, h, t);
			bnzero(u); bnmul(kp->x, d, u);
			bnadd(t, u, v, 1);
			bndiv(v, e->p, w, r);
			
			bnadd(h, k, t, 1); bnadd(t, d, u, 1);
			while (bncmp(r, u) < 0) { bnadd(r, u, r, 1); }
			
			bnsub(r, u, v, 1);
			bnadd(k, v, k, 1);
			pmul(k, e, kp);
			
			bnfree(k);
		}
		
		if (o == 1)
		{
			ecout(0, "kP=", kp, "\n");
			bnout("r=", r, "\n\n");
		}
		
		pmul(h, e, hp);
		padd(hp, kp, hkp, tadd);
		padd(hkp, dp, hkdp, tadd);
		pmul(r, e, rp);
		
		if ((d->leng == 1) && (d->nums[0] == 0))
		{
			if (o == 1)
			{
				//ecout(0, "hP=", hp, "\n");
				//ecout(0, "hkP=", hkp, "\n");
				ecout(0, "(h+k+d)P=", hkdp, "\n==\n");
				ecout(0, "rP=", rp, "\n\n");
			}
		}
		
		if (bncmp(hkdp->x, rp->x) == 0)
		{
			if (bncmp(hkdp->y, rp->y) == 0)
			{
				a = 1;
			}
		}
		
		bnfree(h);
	}
	
	bnfree(t); bnfree(u); bnfree(v); bnfree(w);
	ecfree(hp); ecfree(hkp); ecfree(hkdp); ecfree(rp);
	etfree(tadd);
	
	return a;
}

/*
ECC ElGamal - Public Encrypt / Private Decrypt

* initialize
- generate secret integer  : d
- publish                  : P, dP

* public encrypt
- generate secret key      : t = (SHA256(pwd) || tmpkey)
- secret unique multiplier : k = rand()
- public key encrypt       : r = k * P        = kP
                           : s = t + (k * dP) = t + kdP = kdP(x + t, y + t)
- encrypt optional message : i = rand()
                           : e = AES256CBC(m, i, t)
- publish                  : [ (r, s) , (i, e) ]

* private decrypt
- private key decrypt      : u = s - (d * r)
                               = (t + kdP) - (d * kP)
                               = kdP(x + t, y + t) - dkP(x, y)
                               = kdP(x - x + t)
                               = t
*/

void ecenc(ecc *e, bnum *d, ecc *dp, char *hh, ecc *kp, ecc *kdp, int o)
{
	int psiz = (e->p)->size;
	bnum *k, *h, *t;
	ecc *dkp;
	
	if (hh != NULL)
	{
		if ((d->leng == 1) && (d->nums[0] == 0))
		{
			k = bnrnd(psiz); bncopy(k, d); bnfree(k); k = NULL;
			pmul(d, e, dp);
		}
		
		if (o == 1)
		{
			ecout(0, "P=", e, "\n");
			ecout(0, "dP=", dp, "\n\n");
		}
		
		h = bndec(hh);
		
		if (((kp->x)->leng == 1) && ((kp->x)->nums[0] == 0))
		{
			if (o == 1) { bnout("h=", h, "\n\n"); }
			
			k = bnrnd(psiz);
			pmul(k, e, kp);
			
			pmul(k, dp, kdp);
			t = kdp->x; kdp->x = bninit(psiz + 1); bncopy(t, kdp->x); bnfree(t);
			bnadd(kdp->x, h, kdp->x, 1);
			
			if (o == 1)
			{
				bnout("k=", k, "\n");
				ecout(0, "kP=", kp, "\n");
				ecout(0, "kdP=", kdp, "\n\n");
			}
			
			bnfree(k);
		}
		
		else
		{
			dkp = ecdup(e);
			pmul(d, kp, dkp);
			t = bninit(psiz); bnsub(kdp->x, dkp->x, t, 1);
			
			if (o == 1)
			{
				bnout("d=", d, "\n");
				ecout(0, "dkP=", dkp, "\n\n");
				bnout("t=", t, "\n\n");
			}
			
			bnfree(t);
			ecfree(dkp);
		}
		
		bnfree(h);
	}
}

int main(int argc, char **argv)
{
	char *a = "486662", *b = "1", *p = "57896044618658097711785492504343953926634992332820282019728792003956564819949";
	char *x = "9", *y = "43114425171068552920764898935933967039370386198203806730763910166200978582548";
	char *n, *m, *z = NULL;
	bnum *d, *r, *s;
	bnum *t, *u, *v, *w;
	ecc *e, *f, *dp, *kp, *kdp;
	
	if (argc > 2) { z = argv[2]; }
	
	t = bndec(p);
	u = bninit(t->size); w = bndec(x); bncopy(w, u); bnfree(w);
	v = bninit(t->size); w = bndec(y); bncopy(w, v); bnfree(w);
	e = ecinit(bndec(a), bndec(b), t, u, v);
	
	if (strcmp(argv[1], "pgen") == 0)
	{
		eccp(e, z);
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
		
		free(n); free(m);
		ecfree(f);
	}
	
	if (strcmp(argv[1], "psig") == 0)
	{
		d = bninit((e->p)->size);
		dp = ecdup(e); kp = ecdup(e);
		r = bninit((e->p)->size * 5);
		s = bninit((e->p)->size * 5);
		
		printf("Sign:\n\n");
		(kp->x)->leng = 1; (kp->x)->nums[0] = 0;
		(kp->y)->leng = 1; (kp->y)->nums[0] = 0;
		ecsig(e, d, dp, z, kp, r, 1);
		
		printf("Verify:\n\n");
		bnzero(d);
		if (ecsig(e, d, dp, z, kp, r, 1) != 0) { printf("[GOOD]\n"); }
		else { printf("\n[FAILED]\n"); }
		
		bnfree(d); bnfree(r); bnfree(s);
		ecfree(dp); ecfree(kp);
	}
	
	if (strcmp(argv[1], "penc") == 0)
	{
		d = bninit((e->p)->size);
		dp = ecdup(e); kp = ecdup(e); kdp = ecdup(e);
		
		printf("Encrypt:\n\n");
		(kp->x)->leng = 1; (kp->x)->nums[0] = 0;
		(kp->y)->leng = 1; (kp->y)->nums[0] = 0;
		ecenc(e, d, dp, z, kp, kdp, 1);
		
		printf("Decrypt:\n\n");
		ecenc(e, d, dp, z, kp, kdp, 1);
		
		bnfree(d);
		ecfree(dp); ecfree(kp); ecfree(kdp);
	}
	
	ecfree(e);
	
	return 0;
}
