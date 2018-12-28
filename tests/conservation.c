#include "test.h"
#include <jwb.h>
#include <assert.h>
#include <stdlib.h>
#include <time.h>

static jwb_num_t get_energy(jwb_world_t *world, jwb_ehandle_t ent)
{
	jwb_num_t mass;
	struct jwb_vect vel;
	jwb_world_get_vel_unck(world, ent, &vel);
	mass = jwb_world_get_mass_unck(world, ent);
	return mass * (vel.x*vel.x + vel.y*vel.y) / 2;
}

static jwb_num_t get_total_energy(jwb_world_t *world)
{
	jwb_num_t energy = 0.;
	jwb_ehandle_t e;
	for (e = jwb_world_first(world); e >= 0; e = jwb_world_next(world, e)) {
		energy += get_energy(world, e);
	}
	return energy;
}

static void get_total_momentum(jwb_world_t *world, struct jwb_vect *momentum)
{
	jwb_ehandle_t e;
	momentum->x = momentum->y = 0.;
	for (e = jwb_world_first(world); e >= 0; e = jwb_world_next(world, e)) {
		jwb_num_t mass;
		struct jwb_vect vel;
		jwb_world_get_vel_unck(world, e, &vel);
		mass = jwb_world_get_mass_unck(world, e);
		momentum->x += mass * vel.x;
		momentum->y += mass * vel.y;
	}
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
	struct jwb_world_init alloc_info = JWB_WORLD_INIT_DEFAULT;
	jwb_num_t energy_i, energy_f;
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
		jwb_num_t radius, mass;
		pos.x = frand(), pos.y = frand();
		vel.x = frand(), vel.y = frand();
		radius = frand();
		mass = frand();
		jwb_world_add_ent(world, &pos, &vel, mass, radius);
	}
	/* Elastic collisions. */
	energy_i = get_total_energy(world);
	get_total_momentum(world, &momentum_i);
	sim_world(world);
	energy_f = get_total_energy(world);
	get_total_momentum(world, &momentum_f);
	assert(fequal(energy_i, energy_f));
	momentum_i.x -= momentum_f.x;
	momentum_i.y -= momentum_f.y;
	assert(fequal(jwb_vect_magnitude(&momentum_i), 0.));
	/* Inelastic collisions */
	jwb_world_on_hit(world, jwb_inelastic_collision);
	momentum_i.x += momentum_f.x;
	momentum_i.y += momentum_f.y;
	sim_world(world);
	get_total_momentum(world, &momentum_f);
	momentum_i.x -= momentum_f.x;
	momentum_i.y -= momentum_f.y;
	assert(fequal(jwb_vect_magnitude(&momentum_i), 0.));
	return 0;
}
