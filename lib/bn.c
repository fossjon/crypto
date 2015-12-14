struct bigint {
	int sign, leng, size;
	unsigned int *nums;
};

#define bnum struct bigint

int min(int a, int b)
{
	if (a < b) { return a; }
	return b;
}

int max(int a, int b)
{
	if (a > b) { return a; }
	return b;
}

int oshift(char **a, int b, char c)
{
	// right shift string a ("234") and insert overflow char c ('1')
	int x, n = (b + 1);
	char *s = *a;
	s = realloc(s, n * sizeof(char));
	for (x = (n - 2); x > 0; --x) { s[x] = s[x - 1]; }
	s[0] = c; s[n - 1] = '\0';
	*a = s;
	return n;
}

char *bnstr(bnum *bint)
{
	int x, y, w, z, o, n;
	int r = 2, t = 2;
	char *result = malloc(r * sizeof(char));
	char *twos = malloc(t * sizeof(char));
	// result = 0
	result[0] = '0'; result[1] = '\0';
	// twos = 1
	twos[0] = '1'; twos[1] = '\0';
	// loop through the bn binary
	for (x = 0; x < bint->leng; ++x)
	{
		for (y = 0; y < 32; ++y)
		{
			// if bn binary[n] == 1 then add the 2^n to result
			if (((bint->nums[x] >> y) & 0x1) == 1)
			{
				o = 0; w = (t - 2); z = (r - 2);
				// if the length of twos is bigger than length of result then realloc result
				if (t > r)
				{
					result = realloc(result, t * sizeof(char));
					result[t - 1] = '\0';
					r = t;
				}
				// add twos to result
				while ((w > -1) || (z > -1))
				{
					int a = 0; if (w > -1) { a = (twos[w] - '0'); }
					int b = 0; if (z > -1) { b = (result[z] - '0'); }
					n = (a + b + o);
					result[w] = ('0' + (n % 10));
					o = (n / 10);
					--w; --z;
				}
				if (o > 0) { r = oshift(&result, r, '0' + o); }
			}
			// multiply twos * 2
			o = 0; w = (t - 2);
			while (w > -1)
			{
				n = (o + ((twos[w] - '0') * 2));
				twos[w] = ('0' + (n % 10));
				o = (n / 10);
				--w;
			}
			if (o > 0) { t = oshift(&twos, t, '0' + o); }
		}
	}
	free(twos);
	return result;
}

void bnout(char *s, bnum *p, char *t)
{
	char *o = bnstr(p);
	printf("%s[%d][%d][%d]=[%s]%s", s, p->sign, p->leng, p->size, o, t);
	free(o);
}

void bnzero(bnum *a)
{
	a->sign = 0;
	bzero(a->nums, a->size * sizeof(unsigned int));
	a->leng = 1;
}

void bncopy(bnum *src, bnum *dst)
{
	dst->sign = src->sign;
	bcopy(src->nums, dst->nums, src->leng * sizeof(unsigned int));
	dst->leng = src->leng;
}

void bnfree(bnum *bint)
{
	//todo:zero out all values
	free(bint->nums);
	free(bint);
}

void bndfree(bnum **bint)
{
	bnfree(bint[0]);
	bnfree(bint[1]);
	free(bint);
}

int bncmp(bnum *a, bnum *b)
{
	int x = (a->leng - 1), y = (b->leng - 1);
	while ((x > -1) && (a->nums[x] < 1)) { --x; }
	while ((y > -1) && (b->nums[y] < 1)) { --y; }
	if (x > y) { return 1; }
	if (y > x) { return -1; }
	while (x > -1)
	{
		if (a->nums[x] > b->nums[x]) { return 1; }
		if (b->nums[x] > a->nums[x]) { return -1; }
		--x;
	}
	return 0;
}

int bnhigh(bnum *a)
{
	int x, y, f = 0, hib = -1;
	for (x = (a->leng - 1); (x > -1) && (f == 0); --x)
	{
		for (y = 31; (y > -1) && (f == 0); --y)
		{
			if (((a->nums[x] >> y) & 0x1) == 1)
			{
				hib = ((x * 32) + y);
				f = 1;
			}
		}
	}
	return hib;
}

void bnrshift(bnum *a, int s)
{
	int x, leng = 0, i = (s / 32);
	s = (s % 32);
	for (x = 0; x < (a->leng - i); ++x)
	{
		if (i > 0)
		{
			a->nums[x] = a->nums[x + i];
		}
		if (s > 0)
		{
			if (x > 0)
			{
				a->nums[x - 1] = ((a->nums[x] << (32 - s)) | a->nums[x - 1]);
				if (a->nums[x - 1] > 0) { leng = x; }
			}
			a->nums[x] = (a->nums[x] >> s);
		}
		if (a->nums[x] > 0) { leng = (x + 1); }
	}
	if (leng < 1)
	{
		a->nums[0] = 0;
		leng = 1;
	}
	a->leng = leng;
}

void bnlshift(bnum *a, int s)
{
	int x, o = 0, i = (s / 32);
	unsigned int over = (a->nums[a->leng - 1] >> (32 - (s % 32)));
	s = (s % 32);
	// check to see if the left shift will have any carry over bits
	if ((s > 0) && (over > 0)) { o = 1; }
	// pre adjust the size of the bn to hold the left shift bits
	if (a->size < (a->leng + o + i))
	{
		a->size = (a->leng + o + i);
		a->nums = realloc(a->nums, a->size * sizeof(unsigned int));
	}
	a->leng += (o + i);
	// shift the bits from left to right (last to first) minus the overflow spot
	for (x = (a->leng - o - 1); x > -1; --x)
	{
		if (i > 0)
		{
			if ((x - i) > -1) { a->nums[x] = a->nums[x - i]; }
			else { a->nums[x] = 0; }
		}
		if (s > 0)
		{
			if ((x + 1) < (a->leng - o)) { a->nums[x + 1] = (a->nums[x + 1] | (a->nums[x] >> (32 - s))); }
			a->nums[x] = (a->nums[x] << s);
		}
	}
	// if we have some overflow bits then set them last now
	if (o == 1) { a->nums[a->leng - 1] = over; }
}

bnum *bninit(int ssiz)
{
	int x;
	bnum *a = malloc(1 * sizeof(bnum));
	a->sign = 0; a->leng = 1, a->size = ssiz;
	a->nums = malloc(ssiz * sizeof(unsigned int));
	for (x = 0; x < ssiz; ++x) { a->nums[x] = 0; }
	return a;
}

bnum *bndup(bnum *a)
{
	int x;
	bnum *r = malloc(1 * sizeof(bnum));
	r->sign = a->sign; r->leng = a->leng; r->size = a->size;
	r->nums = malloc(a->size * sizeof(unsigned int));
	for (x = 0; x < a->size; ++x) { r->nums[x] = a->nums[x]; }
	return r;
}

bnum *bndec(char *decstr)
{
	int x, y, r, n;
	int i = 0;
	unsigned long m, l = strlen(decstr);
	char numstr[l + 1], outstr[l + 1];
	// result = 0
	bnum *result = bninit(1);
	// copy the input decimal string into a temp num str
	strncpy(numstr, decstr, l);
	numstr[l] = '\0';
	m = l;
	// while there are numbers left to be divided by 2
	while ((m > 1) || (numstr[0] > '0'))
	{
		// if we are shifting above 32 bits then append a new bit block
		if (i > 31)
		{
			result->leng += 1; result->size += 1;
			result->nums = realloc(result->nums, result->size * sizeof(unsigned int));
			result->nums[result->leng - 1] = 0;
			i = 0;
		}
		// divide the number by 2 and store the result in a temp string
		r = 0;
		outstr[0] = '\0';
		for (x = 0, y = 0; (x < m) && (y < l); ++x, ++y)
		{
			// get the last remainder and the current digit
			n = ((r * 10) + (numstr[x] - '0'));
			// if the digit num is less than the divider 2 then add on the next digit
			if ((n < 2) && ((x + 1) < m) && ((y + 1) < l))
			{
				n = ((n * 10) + (numstr[x + 1] - '0'));
				if (y > 0) { outstr[y] = '0'; ++y; }
				++x;
			}
			// store the division and the remainder
			outstr[y] = ('0' + (n / 2));
			outstr[y + 1] = '\0';
			r = (n % 2);
		}
		// save the binary result
		result->nums[result->leng - 1] = ((r << i) | result->nums[result->leng - 1]);
		++i;
		// copy the divided result into the temp num str
		strncpy(numstr, outstr, l);
		numstr[y] = '\0';
		m = strlen(numstr);
	}
	return result;
}

int bnsub(bnum *, bnum *, bnum *, int);

int bnadd(bnum *a, bnum *b, bnum *r, int s)
{
	int x = 0, y = 0, z = 0, g = 0;
	unsigned int over = 0;
	if (s == 0)
	{
		if ((a->sign == 0) && (b->sign == 1))
		{
			b->sign = 0;
			bnsub(a, b, r, 0); g = r->sign;
			b->sign = 1;
			r->sign = g;
			return 0;
		}
		else if ((a->sign == 1) && (b->sign == 0))
		{
			a->sign = 0;
			bnsub(b, a, r, 0); g = r->sign;
			a->sign = 1;
			r->sign = g;
			return 0;
		}
	}
	while ((x < a->leng) || (y < b->leng))
	{
		unsigned int c = 0; if (x < a->leng) { c = a->nums[x]; }
		unsigned int d = 0; if (y < b->leng) { d = b->nums[y]; }
		unsigned int lo = ((c & 0xffff) + (d & 0xffff) + (over & 0xffff));
		unsigned int hi = (((c >> 16) & 0xffff) + ((d >> 16) & 0xffff) + ((over >> 16) & 0xffff) + ((lo >> 16) & 0xffff));
		over = ((hi >> 16) & 0xffff);
		r->nums[z] = (((hi << 16) & 0xffff0000) | (lo & 0xffff));
		++x; ++y; ++z;
	}
	if (over > 0) { r->nums[z] = over; ++z; }
	r->sign = (a->sign | b->sign);
	r->leng = z;
	return 1;
}

int bnsub(bnum *a, bnum *b, bnum *r, int s)
{
	int x = 0, y = 0;
	int n = 1, g = 0;
	bnum *t = a, *m = b, *temp;
	if (s == 0)
	{
		if ((a->sign == 0) && (b->sign == 1))
		{
			b->sign = 0;
			bnadd(a, b, r, 0); g = r->sign;
			b->sign = 1;
			r->sign = g;
			return 0;
		}
		else if ((a->sign == 1) && (b->sign == 0))
		{
			a->sign = 0;
			bnadd(a, b, r, 0);
			a->sign = 1;
			r->sign = 1;
			return 0;
		}
		else if ((a->sign == 1) && (b->sign == 1))
		{
			t = b; m = a;
		}
		s = bncmp(t, m);
		if (s < 0)
		{
			g = 1;
			temp = t;
			t = m;
			m = temp;
		}
		else if (s == 0)
		{
			r->sign = 0; r->leng = 1;
			r->nums[0] = 0;
			return 2;
		}
	}
	int indx = -1;
	unsigned int over = 0;
	while ((x < t->leng) || (y < m->leng))
	{
		unsigned int c = 0; if (x < t->leng) { c = t->nums[x]; }
		unsigned int d = 0; if (y < m->leng) { d = m->nums[y]; }
		if (x == indx) { c = over; indx = -1; }
		if (c < d)
		{
			int i = x, z;
			for (z = (x + 1); z < t->leng; ++z)
			{
				if (t->nums[z] > 0)
				{
					unsigned int hic = ((1 << 16) + (c >> 16)), loc = (c & 0xffff);
					unsigned int hid = (d >> 16)              , lod = (d & 0xffff);
					if (loc < lod) { hic -= 1; loc += (1 << 16); }
					r->nums[i] = (((hic - hid) << 16) + (loc - lod));
					indx = z; over = (t->nums[z] - 1);
					if (r->nums[i] > 0) { n = max(n, (i + 1)); }
					break;
				}
				++x; ++y;
				if (y < m->leng) { r->nums[z] = (0xffffffff - m->nums[y]); }
				else { r->nums[z] = 0xffffffff; }
				if (r->nums[z] > 0) { n = max(n, (z + 1)); }
			}
		}
		else { r->nums[x] = (c - d); }
		if (r->nums[x] > 0) { n = max(n, (x + 1)); }
		++x; ++y;
	}
	r->sign = g;
	r->leng = n;
	return 1;
}

void bnmul(bnum *a, bnum *b, bnum *r)
{
	int x, y;
	bnum *dub = bninit(r->size);
	bncopy(b, dub);
	for (x = 0; x < a->leng; ++x)
	{
		for (y = 0; y < 32; ++y)
		{
			if (((a->nums[x] >> y) & 0x1) == 1)
			{
				bnadd(r, dub, r, 1);
			}
			bnadd(dub, dub, dub, 1);
		}
	}
	r->sign = (a->sign ^ b->sign);
	bnfree(dub);
}

void bndiv(bnum *a, bnum *b, bnum *r, bnum *m)
{
	int s = 0, d = 0, c = bncmp(a, b);
	bnum *t = bninit(max(a->size, b->size) * 2);
	bncopy(a, t);
	r->nums[0] = 0; r->leng = 1;
	m->nums[0] = 0; m->leng = 1;
	if (c < 0)
	{
		//printf("return <\n");
		r->nums[0] = 0; r->leng = 1;
		bncopy(a, m);
	}
	else if (c == 0)
	{
		//printf("return ==\n");
		r->nums[0] = 1; r->leng = 1;
		m->nums[0] = 0; m->leng = 1;
	}
	else if ((b->leng == 1) && (b->nums[0] == 0))
	{
		//printf("return /0!\n");
	}
	else if (c > 0)
	{
		int hir = 0;
		
		//printf("> right shift the number to match the divider\n");
		int hia = bnhigh(a), hib = bnhigh(b), hid = (hia - hib - 1);
		bnrshift(t, hid + 1);
		c = bncmp(t, b);
		if (c < 0) { s = 1; }
		
		while (1)
		{
			if (s == 1)
			{
				//printf("equal bit length but number < divider\n");
				int abyte = (hid / 32), abit = (hid % 32);
				int bitv = ((a->nums[abyte] & (1 << abit)) >> abit);
				bnlshift(t, 1); t->nums[0] |= bitv;
				--hid;
				if (d > 0) { bnlshift(r, 1); ++hir; }
			}
			s = 0;
			
			//printf("substract num - div and shift 1 into result\n");
			bnsub(t, b, t, 1);
			bnlshift(r, 1); r->nums[0] |= 1; ++hir;
			
			int hit = bnhigh(t);
			d = 0;
			while ((hit < hib) && (hid > -1))
			{
				int abyte = (hid / 32), abit = (hid % 32);
				int diff = min(hib - hit, 32);
				if ((hid + 1) >= diff)
				{
					//printf("large: shift another bit into the number to match the divider\n");
					hid -= (diff - 1);
					int bbyte = (hid / 32), bbit = (hid % 32);
					--hid;
					bnlshift(t, diff);
					t->nums[0] = (t->nums[0] | ((a->nums[abyte] << (31 - abit)) >> (32 - diff)));
					if (abyte != bbyte) { t->nums[0] = (t->nums[0] | (a->nums[bbyte] >> bbit)); }
					d += diff;
				}
				else
				{
					//printf("small: shift another bit into the number to match the divider\n");
					int bitv = ((a->nums[abyte] & (1 << abit)) >> abit);
					bnlshift(t, 1); t->nums[0] |= bitv;
					--hid;
					++d;
				}
				hit = bnhigh(t);
			}
			if (d > 1) { bnlshift(r, d - 1); hir += (d - 1); }
			
			//printf("check resulting number and break if done dividing else repeat\n");
			c = bncmp(t, b);
			if (c < 0)
			{
				if (hid < 0)
				{
					//printf("exiting <\n");
					if (d > 0) { bnlshift(r, 1); ++hir; }
					bncopy(t, m);
					break;
				}
				s = 1;
			}
			else if (c == 0)
			{
				if (hid < 0)
				{
					//printf("exiting ==\n");
					bnlshift(r, 1); r->nums[0] |= 1; ++hir;
					break;
				}
			}
		}
	}
	r->sign = (a->sign ^ b->sign);
	m->sign = r->sign;
	bnfree(t);
}

void bnpowmod(bnum *b, bnum *e, bnum *m, bnum *r)
{
	int s = max(max(b->size, e->size), max(m->size, r->size));
	bnum *t = bninit(s * 3), *u = bninit(s * 3);
	bnum *base = bninit(s * 3), *exp = bninit(s * 3);
	bncopy(b, base); bncopy(e, exp);
	r->nums[0] = 1; r->leng = 1;
	while ((exp->leng > 1) || (exp->nums[0] > 0))
	{
		if ((exp->nums[0] % 2) == 1)
		{
			// r = r * base
			bnzero(t); bnmul(r, base, t);
			// r = r % m
			bndiv(t, m, u, r);
		}
		// exp = exp / 2
		bnrshift(exp, 1);
		// b = b * b
		bnzero(t); bnmul(base, base, t);
		// b = b % m
		bndiv(t, m, u, base);
	}
	// free the rest
	bnfree(base); bnfree(exp);
	bnfree(t); bnfree(u);
}
