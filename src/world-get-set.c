#define JWB_INTERNAL_
#include <jwb.h>

void jwb_world_set_walls(WORLD *world, int on)
{
	if (on) {
		world->flags |= HAS_WALLS;
	} else {
		world->flags &= ~HAS_WALLS;
	}
}

double jwb_world_get_cell_size(jwb_world_t *world)
{
	return world->cell_size;
}

void jwb_world_set_cell_size(jwb_world_t *world, double cell_size)
{
	world->cell_size = cell_size;
}

jwb_hit_handler_t jwb_world_get_hit_handler(WORLD *world)
{
	return world->on_hit;
}

void jwb_world_on_hit(WORLD *world, jwb_hit_handler_t on_hit)
{
	world->on_hit = on_hit;
}

void jwb_world_get_pos(WORLD *world, EHANDLE ent, VECT *dest)
{
	*dest = world->ents[ent].pos;
}

void jwb_world_get_vel(WORLD *world, EHANDLE ent, VECT *dest)
{
	*dest = world->ents[ent].vel;
}

void jwb_world_set_pos(WORLD *world, EHANDLE ent, const VECT *pos)
{
	world->ents[ent].pos = *pos;
}

void jwb_world_set_vel(WORLD *world, EHANDLE ent, const VECT *vel)
{
	world->ents[ent].vel = *vel;
}

void jwb_world_translate(WORLD *world, EHANDLE ent, const VECT *delta)
{
	world->ents[ent].pos.x += delta->x;
	world->ents[ent].pos.y += delta->y;
}

void jwb_world_accelerate(WORLD *world, EHANDLE ent, const VECT *delta)
{
	world->ents[ent].vel.x += delta->x;
	world->ents[ent].vel.y += delta->y;
}

double jwb_world_get_mass(WORLD *world, EHANDLE ent)
{
	return world->ents[ent].mass;
}

double jwb_world_get_radius(WORLD *world, EHANDLE ent)
{
	return world->ents[ent].radius;
}

void jwb_world_set_mass(WORLD *world, EHANDLE ent, double mass)
{
	world->ents[ent].mass = mass;
}

void jwb_world_set_radius(WORLD *world, EHANDLE ent, double radius)
{
	world->ents[ent].radius = radius;
}
