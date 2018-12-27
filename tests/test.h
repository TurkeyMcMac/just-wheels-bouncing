#ifndef JWB_TEST_H_
#define JWB_TEST_H_

#include <jwb.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

static jwb_num_t frand(void)
{
	return (jwb_num_t)rand() / RAND_MAX;
}

static int fequal(jwb_num_t a, jwb_num_t b)
{
	return fabs(a - b) < 0.0001;
}

#endif /* Header guard. */
