#include <common.h>

force_inline u16 iDiv(u32 rem, u16 base)
{
	rem <<= FRACTIONAL_BITS;
	u32 b = base;
	u32 res, d = 1;
	u16 high = rem >> 16;

	res = 0;
	if (high >= base)
	{
		high /= base;
		res = (u32)high << 16;
		rem -= (u32)(high * base) << 16;
	}

	while ((u32)b > 0 && b < rem)
	{
		b *= 2;
		d *= 2;
	}

	do
	{
		if (rem >= b)
		{
			rem -= b;
			res += d;
		}
		b >>= 1;
		d >>= 1;
	} while (d);

	return res;
}

s16 FP_Div(s16 a, s16 b)
{
	int s = 1;
	if (a < 0)
	{
		a = -a;
		s = -1;
	}
	if (b < 0)
	{
		b = -b;
		s = -s;
	}
	return iDiv(a, b) * s;
}
