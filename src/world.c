#include <jwb.h>
#include <stdlib.h>
#include <string.h>

/* Private flags for jwb_world_t::flags */
#	define HAS_WALLS (1 << 0)
#	define REMOVED_ENTS_LOCKED (1 << 1)

#define NAN (0.0 / 0.0)
#define IS_NAN(n) ((n) != (n))

jwb_ehandle_t alloc_new_ent(jwb_world_t *world)
{
	if (world->n_ents >= world->ent_cap) {
		size_t new_cap;
		struct jwb__entity *new_buf;
		new_cap = world->ent_cap * 3 / 2 + 1;
		new_buf = realloc(world->ents, new_cap * sizeof(*new_buf));
		if (new_buf) {
			world->ents = new_buf;
			world->ent_cap = new_cap;
		} else {
			return -JWBE_NO_MEMORY;
		}
	}
	return world->n_ents++;
}

int jwb_world_alloc(
	jwb_world_t *world,
	size_t width,
	size_t height,
	size_t ent_buf_size,
	void *ent_buf,
	void *cell_buf)
{
	int ret = 0;
	if (cell_buf) {
		world->cells = cell_buf;
	} else {
		size_t size = width * height * JWB_CELL_SIZE;
		world->cells = malloc(size);
		if (!world->cells) {
			ret = -JWBE_NO_MEMORY;
			goto error_cells;
		}
		memset(world->cells, -1, size);
	}
	if (ent_buf) {
		world->ents = ent_buf;
	} else {
		world->ents = malloc(ent_buf_size * JWB_ENTITY_SIZE);
		if (!world->ents) {
			ret = -JWBE_NO_MEMORY;
			goto error_entities;
		}
	}
	world->width = width;
	world->height = height;
	world->n_ents = 0;
	world->ent_cap = ent_buf_size;
	/* world->on_hit = jwb_do_hit; */
	world->freed = -1;
	world->flags = HAS_WALLS | REMOVED_ENTS_LOCKED;

error_entities:
	if (!cell_buf) {
		free(world->cells);
	}
error_cells:
	return ret;
}

void jwb_world_set_walls(jwb_world_t *world, int on)
{
	if (on) {
		world->flags |= HAS_WALLS;
	} else {
		world->flags &= ~HAS_WALLS;
	}
}

void jwb_world_on_hit(jwb_world_t *world, jwb_hit_handler_t on_hit)
{
	world->on_hit = on_hit;
}

void jwb_world_step(jwb_world_t *world)
{}

void jwb_world_clear_removed(jwb_world_t *world)
{
	world->flags &= ~REMOVED_ENTS_LOCKED;
}

void jwb_world_destroy(jwb_world_t *world)
{
	free(world->cells);
	free(world->ents);
}

jwb_ehandle_t jwb_world_add_ent(
	jwb_world_t *world,
	const struct jwb_vect *pos,
	const struct jwb_vect *vel,
	double mass,
	double radius)
{
	jwb_ehandle_t ent;
	if ((world->flags & REMOVED_ENTS_LOCKED) == 0 && world->freed >= 0) {
		ent = world->freed;
		world->freed = world->ents[ent].next;
	} else {
		ent = alloc_new_ent(world);
		if (ent < 0) {
			return ent;
		}
	}
	world->ents[ent].pos = *pos;
	world->ents[ent].vel = *vel;
	world->ents[ent].mass = mass;
	world->ents[ent].radius = radius;
	place_ent(world, ent);
	return ent;
}

int jwb_world_remove_ent(jwb_world_t *world, jwb_ehandle_t ent)
{
	if (jwb_world_ent_exists(world, ent)) {
		jwb_ehandle_t next, last;
		next = world->ents[ent].next;
		last = world->ents[ent].last;
		if (next > 0) {
			world->ents[next].last = last;
		}
		if (last > 0) {
			world->ents[last].next = next;
		} else {
			last = ~last;
			world->cells[last] = next;
		}
		world->ents[ent].next = world->freed;
		world->freed = ent;
		world->ents[ent].mass = NAN;
		return 0;
	}
	return -JWBE_INVALID_ENTITY;
}

int jwb_world_ent_exists(jwb_world_t *world, jwb_ehandle_t ent)
{
	return ent < (long)world->n_ents && IS_NAN(world->ents[ent].mass);
}

void jwb_world_get_pos(
	jwb_world_t *world,
	jwb_ehandle_t ent,
	struct jwb_vect *dest)
{
	*dest = world->ents[ent].pos;
}

void jwb_world_get_vel(
	jwb_world_t *world,
	jwb_ehandle_t ent,
	struct jwb_vect *dest)
{
	*dest = world->ents[ent].vel;
}

void jwb_world_set_pos(
	jwb_world_t *world,
	jwb_ehandle_t ent,
	const struct jwb_vect *pos)
{
	world->ents[ent].pos = *pos;
}

void jwb_world_set_vel(
	jwb_world_t *world,
	jwb_ehandle_t ent,
	const struct jwb_vect *vel)
{
	world->ents[ent].vel = *vel;
}

void jwb_world_translate(
	jwb_world_t *world,
	jwb_ehandle_t ent,
	const struct jwb_vect *delta)
{
	world->ents[ent].pos.x += delta->x;
	world->ents[ent].pos.y += delta->y;
}

void jwb_world_accelerate(
	jwb_world_t *world,
	jwb_ehandle_t ent,
	const struct jwb_vect *delta)
{
	world->ents[ent].vel.x += delta->x;
	world->ents[ent].vel.y += delta->y;
}

double jwb_world_get_mass(jwb_world_t *world, jwb_ehandle_t ent)
{
	return world->ents[ent].mass;
}

double jwb_world_get_radius(jwb_world_t *world, jwb_ehandle_t ent)
{
	return world->ents[ent].radius;
}

void jwb_world_set_mass(jwb_world_t *world, jwb_ehandle_t ent, double mass)
{
	world->ents[ent].mass = mass;
}

void jwb_world_set_radius(jwb_world_t *world, jwb_ehandle_t ent, double radius)
{
	world->ents[ent].radius = radius;
}
