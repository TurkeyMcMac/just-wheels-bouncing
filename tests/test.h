#ifndef JWB_TEST_H_
#define JWB_TEST_H_

#include <math.h>
#include <stdlib.h>
#include <stdio.h>

static double frand(void)
{
	return (double)rand() / RAND_MAX;
}

static int fequal(double a, double b)
{
	return fabs(a - b) < 0.0001;
}

#endif /* Header guard. */
