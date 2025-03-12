#pragma once
#include <stdint.h>
#include <rand.h>

// unsigned fix point arithmetic

typedef uint32_t fp_t;
typedef uint64_t  __fp_d_t;

#define FIX_FRACTION_BITS 31
#define FIX_SCALE (1 << FIX_FRACTION_BITS)
#define FP_ONE FIX_SCALE

fp_t fp_add(fp_t a, fp_t b) {
	return a + b;
}

fp_t fp_sub(fp_t a, fp_t b) {
	return a - b;
}

fp_t fp_mul(fp_t a, fp_t b) {
	// Use 64-bit intermediate to prevent overflow
	fp_t product = ( __fp_d_t)a * ( __fp_d_t)b;
	// Scale back to the correct fixed-point format
	return (fp_t)(product >> FIX_FRACTION_BITS);
}

fp_t fp_div(fp_t a, fp_t b) {
	// Scale numerator to maintain precision
	__fp_d_t numerator = (__fp_d_t)a << FIX_FRACTION_BITS;

	// Perform division and return result
	return (fp_t)(numerator / b);
}

uint64_t fp_recerpical_to_uint(fp_t a) {
	const __fp_d_t numerator = ((__fp_d_t)FP_ONE) << FIX_FRACTION_BITS;
	__fp_d_t denumerator = a << FIX_FRACTION_BITS;
	return (uint64_t)(numerator / denumerator);
}

// Convert integer to fixed-point
fp_t fp_from_int(int32_t integer) {
	return integer << FIX_FRACTION_BITS;
}

// Convert fixed-point to integer (truncates fractional part)
int32_t fix16_to_int(fp_t fixed) {
	return fixed >> FIX_FRACTION_BITS;
}

fp_t fp_random(void) {
	// get a uniform random number [0, 1)
	int32_t x = rand();
	x &= (~FP_ONE); // make sure the integer part is zero
	return x;
}

