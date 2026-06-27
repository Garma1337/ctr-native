UTEST(math_sin, cardinal_points)
{
	ASSERT_EQ(0, MATH_Sin(0x000));       /* sin 0   =  0          */
	ASSERT_EQ(0x1000, MATH_Sin(0x400));  /* sin 90  =  1.0 (4.12) */
	ASSERT_EQ(0, MATH_Sin(0x800));       /* sin 180 =  0          */
	ASSERT_EQ(-0x1000, MATH_Sin(0xC00)); /* sin 270 = -1.0        */
}

UTEST(math_cos, cardinal_points)
{
	ASSERT_EQ(0x1000, MATH_Cos(0x000));  /* cos 0   =  1.0 */
	ASSERT_EQ(0, MATH_Cos(0x400));       /* cos 90  =  0   */
	ASSERT_EQ(-0x1000, MATH_Cos(0x800)); /* cos 180 = -1.0 */
	ASSERT_EQ(0, MATH_Cos(0xC00));       /* cos 270 =  0   */
}

/* Adding half a turn (0x800) flips the sign bit only, so sin/cos must
 * negate across the whole lower half-circle. Pure property, no table needed. */
UTEST(math_sin, half_turn_negates)
{
	for (u32 a = 0; a < 0x800; a += 7)
	{
		ASSERT_EQ(-MATH_Sin(a), MATH_Sin(a + 0x800));
	}
}

UTEST(math_cos, half_turn_negates)
{
	for (u32 a = 0; a < 0x800; a += 7)
	{
		ASSERT_EQ(-MATH_Cos(a), MATH_Cos(a + 0x800));
	}
}

/* The angle is masked to 12 bits, so a full turn (0x1000) is identity. */
UTEST(math_sin, full_turn_is_identity)
{
	for (u32 a = 0; a < 0x1000; a += 13)
	{
		ASSERT_EQ(MATH_Sin(a), MATH_Sin(a + 0x1000));
		ASSERT_EQ(MATH_Cos(a), MATH_Cos(a + 0x1000));
	}
}

UTEST(math_fastsqrt, zero_is_zero)
{
	ASSERT_EQ(0, MATH_FastSqrt(0u, 0));
	ASSERT_EQ(0, MATH_FastSqrt(0u, 12));
	ASSERT_EQ(0, MATH_FastSqrt(0u, 24));
}

/* sqrt is monotonic non-decreasing in its argument for a fixed shift. */
UTEST(math_fastsqrt, monotonic_in_value)
{
	s32 prev = MATH_FastSqrt(0u, 12);
	for (u32 v = 0; v < 0x01000000u; v += 0x4321u)
	{
		s32 cur = MATH_FastSqrt(v, 12);
		ASSERT_GE(cur, prev);
		prev = cur;
	}
}

UTEST(math_vector, length_of_zero_is_zero)
{
	SVec3 v = {.x = 0, .y = 0, .z = 0};
	ASSERT_EQ(0, MATH_VectorLength(&v));
}

UTEST(math_vector, normalize_zero_is_safe)
{
	SVec3 v = {.x = 0, .y = 0, .z = 0};
	s32 len = MATH_VectorNormalize(&v);
	ASSERT_EQ(0, len); /* early-out: must not divide by zero */
	ASSERT_EQ(0, v.x);
	ASSERT_EQ(0, v.y);
	ASSERT_EQ(0, v.z);
}

/* A normalized non-zero vector should have length ~1.0 (0x1000 in 4.12).
 * Integer division during normalize introduces rounding, so this is an
 * EXPECT with a generous tolerance rather than a hard equality. If it
 * trips on the first run, confirm the real value and tighten the bound
 * (it then becomes a strong joint invariant over normalize + length + GTE). */
UTEST(math_vector, normalized_has_unit_length)
{
	SVec3 v = {.x = 0x300, .y = -0x140, .z = 0x80};
	MATH_VectorNormalize(&v);
	s32 len = MATH_VectorLength(&v);
	EXPECT_NEAR(0x1000, len, 0x80);
}
