#include "test.h"
#include <jwb.h>
#include <assert.h>
#include <time.h>

int main(void)
{
	size_t i;
	srand(time(NULL));
	for (i = 0; i < 1000; ++i) {
		struct jwb_vect v;
		jwb_rotation_t rot;
		v.x = frand() - 0.5;
		v.y = frand() - 0.5;
		jwb_vect_rotation(&v, &rot);
		assert(fequal(jwb_vect_angle(&v), jwb_rotation_angle(&rot)));
		jwb_rotation_flip(&rot);
		assert(fequal(jwb_vect_angle(&v), -jwb_rotation_angle(&rot)));
	}
	return 0;
}
