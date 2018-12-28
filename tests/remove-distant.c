#include <jwb.h>
#include <assert.h>
#include "test.h"
#include <time.h>

#define NUM_ENTS 10

static void count_remaining(int flags, size_t placed, size_t removed)
{
	jwb_world_t *world = malloc(sizeof(*world));
	struct jwb_world_init alloc_info = JWB_WORLD_INIT_DEFAULT;
	size_t n;
	jwb_ehandle_t e;
	alloc_info.cell_size = 10.;
	alloc_info.flags = flags;
	alloc_info.width = 10;
	alloc_info.height = 10;
	alloc_info.ent_buf_size = NUM_ENTS;
	alloc_info.ent_extra = 0;
	alloc_info.ent_buf = NULL;
	alloc_info.cell_buf = NULL;
	jwb_world_alloc(world, &alloc_info);
	for (n = 0; n < NUM_ENTS; ++n) {
		struct jwb_vect pos, vel;
		pos.x = pos.y = n;
		vel.x = -5;
		vel.y = 0;
		jwb_world_add_ent(world, &pos, &vel, 1., 1.);
	}
	for (n = 0; n < 100; ++n) {
		jwb_world_step(world);
	}
	n = 0;
	for (e = jwb_world_first(world); e >= 0; e = jwb_world_next(world, e)) {
		++n;
	}
	assert(n == placed);
	n = 0;
	for (e = jwb_world_first_removed(world);
	     e >= 0;
	     e = jwb_world_next_removed(world, e)) {
		++n;
	}
	assert(n == removed);
	jwb_world_destroy(world);
	free(world);
}


int main(void)
{
	count_remaining(0, NUM_ENTS, 0);
	count_remaining(JWBF_REMOVE_DISTANT, 0, NUM_ENTS);
	return 0;
}
