#include "bn.c"

struct ecurve {
	bnum *a, *b, *p, *x, *y;
};

#define ecc struct ecurve

struct ectemp {
	bnum *i, *s, *xr, *yr;
	bnum *t, *u, *v;
	bnum *w, *h, *g;
};

#define ect struct ectemp

ecc *ecinit(bnum *a, bnum *b, bnum *p, bnum *x, bnum *y)
{
	ecc *r = malloc(1 * sizeof(ecc));
	r->a = a; r->b = b; r->p = p;
	r->x = x; r->y = y;
	return r;
}

ecc *ecdup(ecc *e)
{
	ecc *r = malloc(1 * sizeof(ecc));
	r->a = bndup(e->a); r->b = bndup(e->b); r->p = bndup(e->p);
	r->x = bndup(e->x); r->y = bndup(e->y);
	return r;
}

void ecfree(ecc *e)
{
	bnfree(e->a); bnfree(e->b); bnfree(e->p);
	bnfree(e->x); bnfree(e->y);
	free(e);
}

void ecout(int d, char *s, ecc *e, char *t)
{
	char *a = bnstr(e->a), *b = bnstr(e->b), *p = bnstr(e->p);
	char *x = bnstr(e->x), *y = bnstr(e->y);
	char as[2], bs[2];
	as[0] = '+'; as[1] = '\0';
	bs[0] = '\0'; bs[1] = '\0';
	if ((e->a)->sign == 1) { as[0] = '-'; }
	if ((e->b)->sign == 1) { bs[0] = '-'; }
	if (d == 1) { printf("  %s%s*(y^2) = x^3 %s %s*(x^2) + x (mod %s)\n", bs, b, as, a, p); }
	printf("%s", s);
	printf("(x, y) = (%s, %s)", x, y);
	printf("%s", t);
	free(a); free(b); free(p);
	free(x); free(y);
}

ect *etinit(ecc *e)
{
	ect *t = malloc(1 * sizeof(ect));
	int ss = max(1, (e->b)->size);
	ss = max(((e->a)->size * 2) + 2, ((e->p)->size * 2) + 2);
	ss = max(((e->x)->size * 2) + 2, ((e->y)->size * 2) + 2);
	t->i = bninit(ss); t->s = bninit(ss); t->xr = bninit(ss); t->yr = bninit(ss);
	t->t = bninit(ss); t->u = bninit(ss); t->v = bninit(ss);
	int tt = ((ss * 2) + 2);
	t->w = bninit(tt); t->h = bninit(tt); t->g = bninit(tt + 4);
	return t;
}

void etfree(ect *t)
{
	bnfree(t->i); bnfree(t->s); bnfree(t->xr); bnfree(t->yr);
	bnfree(t->t); bnfree(t->u); bnfree(t->v);
	bnfree(t->w); bnfree(t->h); bnfree(t->g);
	free(t);
}

// modular multiplicative inverse

void egcd(bnum *a, bnum *b, bnum *g)
{
	int size = ((a->size + b->size) * 3);
	// s = 0; news = 1
	bnum *s = bninit(size);
	bnum *news = bninit(size); news->nums[0] = 1;
	// r = b; newr = a
	bnum *r = bninit(size); bncopy(b, r);
	bnum *newr = bninit(size); bncopy(a, newr);
	// init some temp vars
	bnum *prev = bninit(size), *quot = bninit(size), *temp = bninit(size);
	while ((r->leng > 1) || (r->nums[0] > 0))
	{
		// quot = (newr / r)
		if ((r->leng == 1) && (r->nums[0] < 3))
		{
			bncopy(newr, quot);
			if (r->nums[0] > 1) { bnrshift(quot, 1); }
		}
		else { bndiv(newr, r, quot, temp); }
		// prev = s
		bncopy(s, prev);
		// s = (news - (quot * prev))
		bnzero(temp); bnmul(quot, prev, temp);
		bnsub(news, temp, s, 0);
		// news = prev
		bncopy(prev, news);
		// prev = r
		bncopy(r, prev);
		// r = (newr - (quot * prev))
		bnzero(temp); bnmul(quot, prev, temp);
		bnsub(newr, temp, r, 0);
		// newr = prev
		bncopy(prev, newr);
	}
	if (news->sign == 1)
	{
		// news = news + b
		bnadd(news, b, news, 0);
	}
	bncopy(news, g);
	bnfree(s); bnfree(news);
	bnfree(r); bnfree(newr);
	bnfree(prev); bnfree(quot); bnfree(temp);
}

// modular square root

int sqrtmod(bnum *a, bnum *p, bnum *r)
{
	bnum *o = bninit(1);
	o->nums[0] = 1; o->leng = 1; o->sign = 0;
	
	// legendre symbol
	// define if a is a quadratic residue modulo odd prime
	
	// g = (p - 1) / 2
	
	// p - 1
	bnum *qq = bndup(p);
	bnsub(qq, o, qq, 1);
	// (p - 1) / 2
	bnum *g = bndup(qq);
	bnrshift(g, 1);
	
	// l = pow(a, g, p)
	
	// pow(a, g, p)
	bnum *l = bninit(max(max(a->size, g->size), p->size) * 3);
	bnpowmod(a, g, p, l);
	if (bncmp(l, qq) == 0)
	{
		bnfree(o); bnfree(qq); bnfree(g); bnfree(l);
		return -1;
	}
	
	// factor p - 1 on the form q * (2 ^ s) (with Q odd)
	// q = p - 1; s = 0
	bnum *q = bndup(qq);
	bnum *s = bninit(p->size);
	while ((q->nums[0] % 2) == 0)
	{
		// s += 1; q /= 2
		bnadd(s, o, s, 1);
		bnrshift(q, 1);
	}
	
	// select a z which is a quadratic non resudue modulo p
	// z = 1
	bnum *z = bninit(p->size);
	z->nums[0] = 1; z->leng = 1; z->sign = 0;
	while (1)
	{
		// while (lsym(z, p) != -1)
		bnpowmod(z, g, p, l);
		if (bncmp(l, qq) == 0) { break; }
		// z += 1
		bnadd(z, o, z, 1);
	}
	// c = pow(z, q, p)
	bnum *c = bninit(max(max(z->size, q->size), p->size) * 3);
	bnpowmod(z, q, p, c);
	
	// search for a solution
	// f = ((q + 1) / 2)
	bnum *f = bndup(q);
	bnadd(f, o, f, 1); bnrshift(f, 1);
	// x = pow(a, f, p)
	bnpowmod(a, f, p, r);
	// t = pow(a, q, p)
	bnum *t = bninit(max(max(a->size, q->size), p->size) * 3);
	bnpowmod(a, q, p, t);
	// m = s
	bnum *m = bninit(p->size), *i = bninit(p->size), *e = bninit(p->size);
	bncopy(s, m);
	// u = 2
	bnum *u = bninit(1);
	u->nums[0] = 2; u->leng = 1; u->sign = 0;
	bnum *b = bninit(p->size * 4), *v = bninit(p->size * 4), *w = bninit(p->size * 4);
	while ((t->leng > 1) || (t->nums[0] != 1))
	{
		// find the lowest i such that t ^ (2 ^ i) = 1
		// i = 1; e = 2
		i->nums[0] = 1; i->leng = 1; i->sign = 0;
		e->nums[0] = 2; e->leng = 1; e->sign = 0;
		while (bncmp(i, m) < 0)
		{
			bnpowmod(t, e, p, l);
			if ((l->leng == 1) && (l->nums[0] == 1)) { break; }
			bnlshift(e, 1);
			bnadd(i, o, i, 1);
		}
		// update next value to iterate
		// (m - i - 1)
		bnsub(m, i, v, 0);
		bnsub(v, o, v, 0);
		// 2 ^ (m - i - 1)
		bnpowmod(u, v, p, l);
		// b = (c ^ (2 ^ (m - i - 1))) % p
		bnpowmod(c, l, p, b);
		// x = ((x * b) % p)
		bnzero(v); bnmul(r, b, v);
		bndiv(v, p, w, r);
		// b = (b * b) % p
		bnzero(v); bnmul(b, b, v);
		bndiv(v, p, w, b);
		// t = ((t * b) % p)
		bnzero(v); bnmul(t, b, v);
		bndiv(v, p, w, t);
		// c = b; m = i
		bncopy(b, c);
		bncopy(i, m);
	}
	
	bnfree(o); bnfree(qq); bnfree(g); bnfree(l);
	bnfree(q); bnfree(s); bnfree(z); bnfree(c);
	bnfree(f); bnfree(t); bnfree(m);
	bnfree(i); bnfree(e); bnfree(b);
	bnfree(u); bnfree(v); bnfree(w);
	
	// r = [x, p - x]
	
	return 0;
}

// montgomery curve arithmetic

void nmod(bnum *a, bnum *b)
{
	if (a->sign == 1) { bnadd(b, a, a, 0); }
}

void pdub(ecc *p, ecc *r, ect *t)
{
	// printf("2P=\n");
	
	// l = 3*x^2 + 2*a*x + 1 / 2*b*y
	
	// x^2
	bnzero(t->w); bnmul(p->x, p->x, t->w); bndiv(t->w, p->p, t->t, t->v);
	// 3*x^2
	bnadd(t->v, t->v, t->w, 1); bnadd(t->w, t->v, t->w, 1);
	// 2*a*x
	bnzero(t->h); bnmul(p->a, p->x, t->h); bnlshift(t->h, 1);
	bndiv(t->h, p->p, t->t, t->u); nmod(t->u, p->p);
	// 3*x^2 + 2*a*x + 1
	bnadd(t->w, t->u, t->g, 1);
	int x, o = 1;
	for (x = 0; (x < (t->g)->leng) && (o == 1); ++x)
	{
		o = 0; if ((t->g)->nums[x] == 0xffffffff) { o = 1; } (t->g)->nums[x] += 1;
	}
	if (o == 1) { (t->g)->nums[x] = 1; (t->g)->leng += 1; }
	bndiv(t->g, p->p, t->t, t->yr);
	// 1 / 2*b*y
	bnzero(t->w); bnmul(p->b, p->y, t->w); bnlshift(t->w, 1);
	bndiv(t->w, p->p, t->t, t->u); nmod(t->u, p->p); egcd(t->u, p->p, t->i);
	// 3*x^2 + 2*a*x + 1 / 2*b*y
	bnzero(t->w); bnmul(t->yr, t->i, t->w); bndiv(t->w, p->p, t->t, t->s);
	
	// xr = b*l^2 - a - 2*x
	
	// l^2
	bnzero(t->g); bnmul(t->s, t->s, t->g);
	// b*l^2 - a
	bnzero(t->w); bnmul(p->b, t->g, t->w);
	bnsub(t->w, p->a, t->w, 0);
	// 2*x
	bnzero(t->h); bncopy(p->x, t->h); bnlshift(t->h, 1);
	// b*l^2 - a - 2*x
	bnsub(t->w, t->h, t->w, 0);
	bndiv(t->w, p->p, t->t, t->xr); nmod(t->xr, p->p);
	
	// yr = ((3*x + a) * l) - b*l^3 - y
	
	// (3*x + a) * l
	bnadd(t->h, p->x, t->w, 1); bnadd(t->w, p->a, t->w, 0);
	bndiv(t->w, p->p, t->t, t->u); nmod(t->u, p->p);
	bnzero(t->w); bnmul(t->u, t->s, t->w);
	// l^3
	bncopy(t->g, t->h);
	bnzero(t->g); bnmul(t->h, t->s, t->g); bndiv(t->g, p->p, t->t, t->u);
	// b*l^3
	bnzero(t->h); bnmul(p->b, t->u, t->h);
	bndiv(t->h, p->p, t->t, t->u); nmod(t->u, p->p);
	// ((3*x + a) * l) - b*l^3 - y
	bnsub(t->w, t->u, t->w, 0); bnsub(t->w, p->y, t->w, 0);
	bndiv(t->w, p->p, t->t, t->yr); nmod(t->yr, p->p);
	
	(t->xr)->leng = (p->p)->size; bncopy(t->xr, r->x);
	(t->yr)->leng = (p->p)->size; bncopy(t->yr, r->y);
}

void padd(ecc *p, ecc *q, ecc *r, ect *t)
{
	// printf("P+Q=\n");
	
	// l = (Qy - Py) / (Qx - Px)
	
	// Qy - Py
	bnsub(q->y, p->y, t->yr, 0);
	// Qx - Px
	bnsub(q->x, p->x, t->xr, 0);
	bndiv(t->xr, p->p, t->t, t->u); nmod(t->u, p->p);
	// 1 / (Qx - Px)
	egcd(t->u, p->p, t->i);
	// (Qy - Py) / (Qx - Px)
	bnzero(t->w); bnmul(t->yr, t->i, t->w);
	bndiv(t->w, p->p, t->t, t->s);
	
	// xr = b*l^2 - a - Px - Qx
	
	// b*l^2 - a - Px - Qx
	bnzero(t->w); bnmul(t->s, t->s, t->w);
	bnzero(t->g); bnmul(p->b, t->w, t->g);
	bnsub(t->g, p->a, t->g, 0);
	bnsub(t->g, p->x, t->g, 0);
	bnsub(t->g, q->x, t->g, 0);
	bndiv(t->g, p->p, t->t, t->xr); nmod(t->xr, p->p);
	
	// yr = ((2*Px + Qx + a) * l) - b*l^3 - Py
	
	// 2*Px + Qx + a
	bnadd(p->x, p->x, t->t, 1);
	bnadd(t->t, q->x, t->t, 1);
	bnadd(t->t, p->a, t->t, 0);
	bndiv(t->t, p->p, t->u, t->v); nmod(t->v, p->p);
	// (2*Px + Qx + a) * l
	bnzero(t->w); bnmul(t->v, t->s, t->w); bndiv(t->w, p->p, t->t, t->u);
	// b*l^3
	bnzero(t->w); bnmul(t->s, t->s, t->w);
	bnzero(t->g); bnmul(t->w, t->s, t->g); bndiv(t->g, p->p, t->t, t->v);
	bnzero(t->w); bnmul(t->v, p->b, t->w);
	// ((2*Px + Qx + a) * l) - b*l^3 - Py
	bnsub(t->u, t->w, t->v, 0);
	bnsub(t->v, p->y, t->v, 0);
	bndiv(t->v, p->p, t->t, t->yr); nmod(t->yr, p->p);
	
	(t->xr)->leng = (p->p)->size; bncopy(t->xr, r->x);
	(t->yr)->leng = (p->p)->size; bncopy(t->yr, r->y);
}

void pmul(bnum *m, ecc *p, ecc *r)
{
	int init = 0;
	bnum *mul = bndup(m);
	ecc *b = ecdup(p);
	ect *t = etinit(p);
	while ((mul->leng > 1) || (mul->nums[0] > 0))
	{
		if ((mul->nums[0] % 2) == 1)
		{
			if (init == 0)
			{
				bnfree(r->x); r->x = bndup(b->x);
				bnfree(r->y); r->y = bndup(b->y);
			}
			else
			{
				padd(r, b, r, t);
			}
			init = 1;
		}
		pdub(b, b, t);
		bnrshift(mul, 1);
	}
	bnfree(mul);
	ecfree(b);
	etfree(t);
}
