#include "test.h"
#include <jwb.h>
#include <assert.h>
#include <stdlib.h>
#include <time.h>

static int get_energy(jwb_world_t *world, jwb_ehandle_t ent, void *data)
{
	double *sum_energy = data;
	double mass;
	struct jwb_vect vel;
	jwb_world_get_vel_unck(world, ent, &vel);
	mass = jwb_world_get_mass_unck(world, ent);
	*sum_energy += mass * (vel.x*vel.x + vel.y*vel.y) / 2;
	return 0;
}

static int get_momentum(jwb_world_t *world, jwb_ehandle_t ent, void *data)
{
	struct jwb_vect *sum_momentum = data;
	double mass;
	struct jwb_vect vel;
	jwb_world_get_vel_unck(world, ent, &vel);
	mass = jwb_world_get_mass_unck(world, ent);
	sum_momentum->x += mass * vel.x;
	sum_momentum->y += mass * vel.y;
	return 0;
}

static void sim_world(jwb_world_t *world)
{
	size_t i;
	for (i = 0; i < 1000; ++i) {
		jwb_world_step(world);
	}
}

int main(void)
{
	jwb_world_t *world;
	struct jwb_world_init alloc_info;
	double energy_i, energy_f;
	struct jwb_vect momentum_i, momentum_f;
	size_t i;
	world = malloc(sizeof(*world));
	alloc_info.cell_size = 10.;
	alloc_info.flags = 0;
	alloc_info.width = 10;
	alloc_info.height = 10;
	alloc_info.ent_buf_size = 10;
	alloc_info.ent_extra = 0;
	alloc_info.ent_buf = NULL;
	alloc_info.cell_buf = NULL;
	jwb_world_alloc(world, &alloc_info);
	srand(time(NULL));
	for (i = 0; i < 10; ++i) {
		struct jwb_vect pos, vel;
		double radius, mass;
		pos.x = frand(), pos.y = frand();
		vel.x = frand(), vel.y = frand();
		radius = frand();
		mass = frand();
		jwb_world_add_ent(world, &pos, &vel, mass, radius);
	}
	/* Elastic collisions. */
	energy_i = 0.;
	jwb_world_for_each(world, get_energy, &energy_i);
	momentum_i.x = momentum_i.y = 0.;
	jwb_world_for_each(world, get_momentum, &momentum_i);
	sim_world(world);
	energy_f = 0.;
	jwb_world_for_each(world, get_energy, &energy_f);
	momentum_f.x = momentum_f.y = 0.;
	jwb_world_for_each(world, get_momentum, &momentum_f);
	assert(fequal(energy_i, energy_f));
	momentum_i.x -= momentum_f.x;
	momentum_i.y -= momentum_f.y;
	assert(fequal(jwb_vect_magnitude(&momentum_i), 0.));
	/* Inelastic collisions */
	jwb_world_on_hit(world, jwb_inelastic_collision);
	momentum_i.x += momentum_f.x;
	momentum_i.y += momentum_f.y;
	sim_world(world);
	momentum_f.x = momentum_f.y = 0.;
	jwb_world_for_each(world, get_momentum, &momentum_f);
	momentum_i.x -= momentum_f.x;
	momentum_i.y -= momentum_f.y;
	assert(fequal(jwb_vect_magnitude(&momentum_i), 0.));
	return 0;
}
