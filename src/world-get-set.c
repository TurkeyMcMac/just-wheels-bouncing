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

int jwb_world_set_cell_size(jwb_world_t *world, double cell_size)
{
	if (cell_size < 0.) {
		return -JWBE_INVALID_ARGUMENT;
	}
	world->cell_size = cell_size;
	return 0;
}

void jwb_world_set_cell_size_unck(jwb_world_t *world, double cell_size)
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

int jwb_world_get_pos(WORLD *world, EHANDLE ent, VECT *dest)
{
	int err;
	if (!dest) {
		return -JWBE_INVALID_ARGUMENT;
	}
	err = jwb_world_confirm_ent(world, ent);
	if (err) {
		return err;
	}
	jwb_world_get_pos_unck(world, ent, dest);
	return 0;
}

int jwb_world_get_vel(WORLD *world, EHANDLE ent, VECT *dest)
{
	int err;
	if (!dest) {
		return -JWBE_INVALID_ARGUMENT;
	}
	err = jwb_world_confirm_ent(world, ent);
	if (err) {
		return err;
	}
	jwb_world_get_vel_unck(world, ent, dest);
	return 0;
}

int jwb_world_set_pos(WORLD *world, EHANDLE ent, const VECT *pos)
{
	int err;
	if (!pos) {
		return -JWBE_INVALID_ARGUMENT;
	}
	err = jwb_world_confirm_ent(world, ent);
	if (err) {
		return err;
	}
	jwb_world_set_pos_unck(world, ent, pos);
	return 0;
}

int jwb_world_set_vel(WORLD *world, EHANDLE ent, const VECT *vel)
{
	int err;
	if (!vel) {
		return -JWBE_INVALID_ARGUMENT;
	}
	err = jwb_world_confirm_ent(world, ent);
	if (err) {
		return err;
	}
	jwb_world_set_vel_unck(world, ent, vel);
	return 0;
}

int jwb_world_translate(WORLD *world, EHANDLE ent, const VECT *delta)
{
	int err;
	if (!delta) {
		return -JWBE_INVALID_ARGUMENT;
	}
	err = jwb_world_confirm_ent(world, ent);
	if (err) {
		return err;
	}
	jwb_world_translate_unck(world, ent, delta);
	return 0;
}

int jwb_world_move_later(WORLD *world, EHANDLE ent, const VECT *delta)
{
	int err;
	if (!delta) {
		return -JWBE_INVALID_ARGUMENT;
	}
	err = jwb_world_confirm_ent(world, ent);
	if (err) {
		return err;
	}
	jwb_world_move_later_unck(world, ent, delta);
	return 0;
}

int jwb_world_accelerate(WORLD *world, EHANDLE ent, const VECT *delta)
{
	int err;
	if (!delta) {
		return -JWBE_INVALID_ARGUMENT;
	}
	err = jwb_world_confirm_ent(world, ent);
	if (err) {
		return err;
	}
	jwb_world_accelerate_unck(world, ent, delta);
	return 0;
}

double jwb_world_get_mass(WORLD *world, EHANDLE ent)
{
	int err = jwb_world_confirm_ent(world, ent);
	if (err) {
		return err;
	}
	return jwb_world_get_mass_unck(world, ent);
}

double jwb_world_get_radius(WORLD *world, EHANDLE ent)
{
	int err = jwb_world_confirm_ent(world, ent);
	if (err) {
		return err;
	}
	return jwb_world_get_radius_unck(world, ent);
}

int jwb_world_set_mass(WORLD *world, EHANDLE ent, double mass)
{
	int err;
	if (mass < 0.) {
		return -JWBE_INVALID_ARGUMENT;
	}
	err = jwb_world_confirm_ent(world, ent);
	if (err) {
		return err;
	}
	jwb_world_set_mass_unck(world, ent, mass);
	return 0;
}

int jwb_world_set_radius(WORLD *world, EHANDLE ent, double radius)
{
	int err;
	if (radius < 0. || radius > world->cell_size) {
		return -JWBE_INVALID_ARGUMENT;
	}
	err = jwb_world_confirm_ent(world, ent);
	if (err) {
		return err;
	}
	jwb_world_set_radius_unck(world, ent, radius);
	return 0;
}

void jwb_world_get_pos_unck(WORLD *world, EHANDLE ent, VECT *dest)
{
	*dest = world->ents[ent].pos;
}

void jwb_world_get_vel_unck(WORLD *world, EHANDLE ent, VECT *dest)
{
	*dest = world->ents[ent].vel;
}

void jwb_world_set_pos_unck(WORLD *world, EHANDLE ent, const VECT *pos)
{
	world->ents[ent].pos = *pos;
}

void jwb_world_set_vel_unck(WORLD *world, EHANDLE ent, const VECT *vel)
{
	world->ents[ent].vel = *vel;
}

void jwb_world_translate_unck(WORLD *world, EHANDLE ent, const VECT *delta)
{
	world->ents[ent].pos.x += delta->x;
	world->ents[ent].pos.y += delta->y;
}

void jwb_world_move_later_unck(WORLD *world, EHANDLE ent, const VECT *delta)
{
	world->ents[ent].correct.x += delta->x;
	world->ents[ent].correct.y += delta->y;
}

void jwb_world_accelerate_unck(WORLD *world, EHANDLE ent, const VECT *delta)
{
	world->ents[ent].vel.x += delta->x;
	world->ents[ent].vel.y += delta->y;
}

double jwb_world_get_mass_unck(WORLD *world, EHANDLE ent)
{
	return world->ents[ent].mass;
}

double jwb_world_get_radius_unck(WORLD *world, EHANDLE ent)
{
	return world->ents[ent].radius;
}

void jwb_world_set_mass_unck(WORLD *world, EHANDLE ent, double mass)
{
	world->ents[ent].mass = mass;
}

void jwb_world_set_radius_unck(WORLD *world, EHANDLE ent, double radius)
{
	world->ents[ent].radius = radius;
}
