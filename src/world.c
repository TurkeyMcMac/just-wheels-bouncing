#include <jwb.h>
#define _GNU_SOURCE
#include <math.h>
#include <stdlib.h>
#include <string.h>

/* Private flags for jwb_world_t::flags */
#	define HAS_WALLS (1 << 0)
#	define REMOVED_ENTS_LOCKED (1 << 1)

#define IS_NAN(n) ((n) != (n))

static unsigned long frame(long num, unsigned long lim)
{
	long mod = num % lim;
	if (mod < 0) {
		mod += lim;
	}
	return (unsigned long)mod;
}

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

static void place_ent(jwb_world_t *world, jwb_ehandle_t ent)
{
	size_t x, y;
	struct jwb_vect pos;
	size_t cell_idx;
	jwb_ehandle_t cell;
	pos = world->ents[ent].pos;
	x = frame(pos.x / world->cell_size, world->width);
	y = frame(pos.y / world->cell_size, world->height);
	cell_idx = y * world->width + x;
	cell = world->cells[cell_idx];
	world->ents[ent].last = ~cell_idx;
	world->ents[ent].next = cell;
	if (cell >= 0) {
		world->ents[cell].last = ent;
	}
	world->cells[cell_idx] = ent;
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
	world->cell_size = 0.;
	return ret;

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

void check_ent_hit(jwb_world_t *world, jwb_ehandle_t ent1, jwb_ehandle_t ent2)
{
	double rad1, rad2;
	double mass1, mass2;
	struct jwb_vect pos1, pos2, relative;
	struct jwb_vect vel1, vel2;
	double bounced1, bounced2;
	jwb_rotation_t rot;
	pos1 = world->ents[ent1].pos;
	pos2 = world->ents[ent2].pos;
	relative.x = pos2.x - pos1.x;
	relative.y = pos2.y - pos1.y;
	rad1 = world->ents[ent1].radius;
	rad2 = world->ents[ent2].radius;
	if (rad1 + rad2 >= jwb_vect_magnitude(&relative)) {
		return;
	}
	mass1 = world->ents[ent1].mass;
	mass2 = world->ents[ent2].mass;
	vel1 = world->ents[ent1].vel;
	vel2 = world->ents[ent2].vel;
	jwb_vect_rotation(&relative, &rot); /* FIXME: Recalculates magnitude. */
	jwb_vect_rotate(&vel1, &rot);
	jwb_vect_rotate(&vel2, &rot);
	bounced1 = (mass1 - mass2) / (mass1 + mass2) * vel1.y + 2 * mass2
		/ (mass1 + mass2) * vel2.y;
	bounced2 = (mass2 - mass1) / (mass2 + mass1) * vel2.y + 1 * mass1
		/ (mass2 + mass1) * vel1.y;
	vel1.y = bounced1;
	vel2.y = bounced2;
	jwb_rotation_flip(&rot);
	jwb_vect_rotate(&vel1, &rot);
	jwb_vect_rotate(&vel2, &rot);
	world->ents[ent1].vel = vel1;
	world->ents[ent2].vel = vel2;
}

void update_cell(jwb_world_t *world, size_t x, size_t y)
{
	jwb_ehandle_t next = world->cells[y * world->width + x];
	while (next >= 0) {
		jwb_ehandle_t self, next_other;
		self = next;
		next = world->ents[next].next;
		next_other = next;
		while (next_other >= 0) {
			jwb_ehandle_t other;
			other = next_other;
			next_other = world->ents[next_other].next;
			check_ent_hit(world, self, other);
		}
	}
}

void update_cells(
	jwb_world_t *world,
	size_t x1,
	size_t y1,
	size_t x2,
	size_t y2)
{
	jwb_ehandle_t next1, next2;
	next1 = world->cells[y1 * world->width + x1];
	next2 = world->cells[y2 * world->width + x2];
	while (next1 >= 0) {
		jwb_ehandle_t self, next_other;
		self = next1;
		next1 = world->ents[next1].next;
		next_other = next2;
		while (next_other >= 0) {
			jwb_ehandle_t other;
			other = next_other;
			next_other = world->ents[next_other].next;
			check_ent_hit(world, self, other);
		}
	}
}

void jwb_world_step(jwb_world_t *world)
{
	/* TODO: Unroll this loop a bit. */
	size_t x, y;
	for (y = 0; y < world->height; ++y) {
		for (x = 0; x < world->width; ++x) {
			update_cell(world, x, y);
			update_cells(world, x, y, frame(x + 1, world->width),
				y);
			update_cells(world, x, y, frame(x + 1, world->width),
				frame(y + 1, world->height));
			update_cells(world, x, y, x,
				frame(y + 1, world->height));
			update_cells(world, x, y, frame(x - 1, world->width),
				frame(y + 1, world->height));
		}
	}
}

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
