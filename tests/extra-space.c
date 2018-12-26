#include <jwb.h>
#include <assert.h>
#include "test.h"
#include <time.h>

struct world_outcome {
	struct jwb_vect pos, vel;
};

static int seed;

static int get_outcome(jwb_world_t *world, jwb_ehandle_t ent, void *data)
{
	struct world_outcome *outcome = data;
	struct jwb_vect pos, vel;
	jwb_world_get_pos(world, ent, &pos);
	jwb_world_get_vel(world, ent, &vel);
	outcome->pos.x += pos.x;
	outcome->pos.y += pos.y;
	outcome->vel.x += vel.x;
	outcome->vel.y += vel.y;
	return 0;
}

static int scramble_extra(jwb_world_t *world, jwb_ehandle_t ent, void *null)
{
	size_t size, i;
	char *extra;
	(void)null;
	extra = jwb_world_get_extra_unck(world, ent);
	size = jwb_world_extra_size(world);
	for (i = 0; i < size; ++i) {
		extra[i] = rand();
	}
	return 0;
}

static void sim_world(size_t extra_space, struct world_outcome *outcome)
{
	size_t i;
	jwb_world_t *world = malloc(sizeof(*world));
	struct jwb_world_init alloc_info;
	alloc_info.cell_size = 10.;
	alloc_info.flags = 0;
	alloc_info.width = 10;
	alloc_info.height = 10;
	alloc_info.ent_buf_size = 10;
	alloc_info.ent_extra = extra_space;
	alloc_info.ent_buf = NULL;
	alloc_info.cell_buf = NULL;
	jwb_world_alloc(world, &alloc_info);
	srand(seed);
	for (i = 0; i < 10; ++i) {
		struct jwb_vect pos, vel;
		double radius, mass;
		pos.x = frand(), pos.y = frand();
		vel.x = frand(), vel.y = frand();
		radius = frand();
		mass = frand();
		jwb_world_add_ent(world, &pos, &vel, mass, radius);
	}
	for (i = 0; i < 1000; ++i) {
		jwb_world_step(world);
		jwb_world_for_each(world, scramble_extra, NULL);
	}
	jwb_world_for_each(world, get_outcome, outcome);
	jwb_world_destroy(world);
	free(world);
}

static void equal_outcome(struct world_outcome *o1, struct world_outcome *o2)
{
	assert(o1->pos.x == o2->pos.x);
	assert(o1->pos.y == o2->pos.y);
	assert(o1->vel.x == o2->vel.x);
	assert(o1->vel.y == o2->vel.y);
}

int main(void)
{
	struct world_outcome no_extra, some_extra, much_extra;
	seed = time(NULL);
	sim_world(0, &no_extra);
	sim_world(4, &some_extra);
	sim_world(40, &much_extra);
	equal_outcome(&no_extra, &some_extra);
	equal_outcome(&no_extra, &much_extra);
	return 0;
}
