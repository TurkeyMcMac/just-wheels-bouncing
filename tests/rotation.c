#include "test.h"
#include <jwb.h>
#include <assert.h>
#include <time.h>

int main(void)
{
	size_t i;
	srand(time(NULL));
	for (i = 0; i < 1000; ++i) {
		struct jwb_vect v1, v2;
		jwb_rotation_t rot;
		v1.x = frand() - 0.5;
		v1.y = frand() - 0.5;
		v2 = v1;
		jwb_vect_rotation(&v1, &rot);
		jwb_rotation_flip(&rot);
		jwb_vect_rotate(&v2, &rot);
		assert(fequal(v2.x, jwb_vect_magnitude(&v1)));
		assert(fequal(v2.y, 0.));
		jwb_rotation_flip(&rot);
		jwb_vect_rotate(&v2, &rot);
		assert(fequal(v1.x, v2.x));
		assert(fequal(v1.y, v2.y));
		assert(fequal(jwb_vect_angle(&v1), jwb_rotation_angle(&rot)));
	}
	return 0;
}
