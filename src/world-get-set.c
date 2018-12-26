#define JWB_INTERNAL_
#include <jwb.h>

jwb_hit_handler_t jwb_world_get_hit_handler(WORLD *world)
{
	return world->on_hit;
}

void jwb_world_on_hit(WORLD *world, jwb_hit_handler_t on_hit)
{
	world->on_hit = on_hit;
}

size_t jwb_world_extra_size(WORLD *world)
{
	return world->ent_size - sizeof(struct jwb__entity) +
		JWB__ENTITY_EXTRA_MIN_SIZE;
}

int jwb_world_offset(WORLD *world, const VECT *off)
{
	if (!off) {
		return -JWBE_INVALID_ARGUMENT;
	}
	world->offset = *off;
	return 0;
}

int jwb_world_get_offset(WORLD *world, VECT *dest)
{
	if (!dest) {
		return -JWBE_INVALID_ARGUMENT;
	}
	*dest = world->offset;
	return 0;
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
	*vect = GET(world, ent).pos;
})

VECT_METHOD(get_vel, VECT, {
	*vect = GET(world, ent).vel;
})

VECT_METHOD(set_pos, const VECT, {
	GET(world, ent).pos = *vect;
})

VECT_METHOD(set_vel, const VECT, {
	GET(world, ent).vel = *vect;
})

VECT_METHOD(translate, const VECT, {
	GET(world, ent).pos.x += vect->x;
	GET(world, ent).pos.y += vect->y;
})

VECT_METHOD(move_later, const VECT, {
	GET(world, ent).correct.x += vect->x;
	GET(world, ent).correct.y += vect->y;
})

VECT_METHOD(accelerate, const VECT, {
	GET(world, ent).vel.x += vect->x;
	GET(world, ent).vel.y += vect->y;
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

SCALAR_GETTER(mass, GET(world, ent).mass)

SCALAR_GETTER(radius, GET(world, ent).radius)

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
	GET(world, ent).mass = v;
})

SCALAR_SETTER(radius, v > world->cell_size, {
	GET(world, ent).radius = v;
})

void *jwb_world_get_extra(WORLD *world, EHANDLE ent)
{
	int err;
	err = jwb_world_confirm_ent(world, ent);
	if (err == -JWBE_DESTROYED_ENTITY) {
		return NULL;
	}
	return jwb_world_get_extra_unck(world, ent);
}

void *jwb_world_get_extra_unck(WORLD *world, EHANDLE ent)
{
	return GET(world, ent).extra;
}
