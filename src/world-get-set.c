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
	double cell_size = world->cell_size;
	if (world->flags & ONE_CELL_THICK) {
		cell_size *= 2.;
	}
	return cell_size;
}

int jwb_world_set_cell_size(jwb_world_t *world, double cell_size)
{
	if (cell_size < 0.) {
		return -JWBE_INVALID_ARGUMENT;
	}
	jwb_world_set_cell_size_unck(world, cell_size);
	return 0;
}

void jwb_world_set_cell_size_unck(jwb_world_t *world, double cell_size)
{
	if (world->flags & ONE_CELL_THICK) {
		cell_size /= 2.;
	}
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

#define VECT_METHOD(name, vtype, code) \
	int jwb_world_##name(WORLD *world, EHANDLE ent, vtype *vect) \
	{ \
		int err; \
		if (!vect) { \
			return -JWBE_INVALID_ARGUMENT; \
		} \
		err = jwb_world_confirm_ent(world, ent); \
		if (err == -JWBE_DESTROYED_ENTITY) { \
			return err; \
		} \
		jwb_world_##name##_unck(world, ent, vect); \
		return 0; \
	} \
	void jwb_world_##name##_unck(WORLD *world, EHANDLE ent, vtype *vect) \
	{ code }

VECT_METHOD(get_pos, VECT, {
	*vect = world->ents[ent].pos;
})

VECT_METHOD(get_vel, VECT, {
	*vect = world->ents[ent].vel;
})

VECT_METHOD(set_pos, const VECT, {
	world->ents[ent].pos = *vect;
})

VECT_METHOD(set_vel, const VECT, {
	world->ents[ent].vel = *vect;
})

VECT_METHOD(translate, const VECT, {
	world->ents[ent].pos.x += vect->x;
	world->ents[ent].pos.y += vect->y;
})

VECT_METHOD(move_later, const VECT, {
	world->ents[ent].correct.x += vect->x;
	world->ents[ent].correct.y += vect->y;
})

VECT_METHOD(accelerate, const VECT, {
	world->ents[ent].vel.x += vect->x;
	world->ents[ent].vel.y += vect->y;
})

#define SCALAR_GETTER(name, ret_expr) \
	double jwb_world_get_##name(WORLD *world, EHANDLE ent) \
	{ \
		int err = jwb_world_confirm_ent(world, ent); \
		if (err == -JWBE_DESTROYED_ENTITY) { \
			return err; \
		} \
		return jwb_world_get_mass_unck(world, ent); \
	} \
	double jwb_world_get_##name##_unck(WORLD *world, EHANDLE ent) \
	{ return ret_expr; }

SCALAR_GETTER(mass, world->ents[ent].mass)

SCALAR_GETTER(radius, world->ents[ent].radius)

#define SCALAR_SETTER(name, extra_check, code) \
	int jwb_world_set_##name(WORLD *world, EHANDLE ent, double v) \
	{ \
		int err; \
		if (v < 0. || (extra_check)) { \
			return -JWBE_INVALID_ARGUMENT; \
		} \
		err = jwb_world_confirm_ent(world, ent); \
		if (err == -JWBE_DESTROYED_ENTITY) { \
			return err; \
		} \
		jwb_world_set_##name##_unck(world, ent, v); \
		return 0; \
	} \
	void jwb_world_set_##name##_unck(WORLD *world, EHANDLE ent, double v) \
	{ code }

SCALAR_SETTER(mass, 0, {
	world->ents[ent].mass = v;
})

SCALAR_SETTER(radius, v > world->cell_size, {
	world->ents[ent].radius = v;
})
