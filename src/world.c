#include <jwb.h>
#define _GNU_SOURCE
#include <math.h>
#include <stdlib.h>
#include <string.h>

#ifdef JWBO_NO_ALLOC
#	define ALLOC(size) ((void)(size), NULL)
#	define FREE(ptr) ((void)(ptr))
#else
#	define ALLOC(size) malloc((size))
#	define FREE(ptr) free((ptr))
#endif /* JWBO_NO_ALLOC */

/* Private flags for jwb_world_t::flags */
#	define HAS_WALLS (1 << 0)

/* Private flags for jwb__entity::flags */
#	define REMOVED (1 << 0)
#	define MOVED_THIS_STEP (1 << 1)
#	define DESTROYED (1 << 2)

static double fframe(double num, double lim)
{
	double mod = fmod(num, lim);
	if (mod < 0) {
		mod += lim;
	}
	return mod;
}

jwb_ehandle_t alloc_new_ent(jwb_world_t *world)
{
	if (world->n_ents >= world->ent_cap) {
#ifdef JWBO_NO_ALLOC
		return -JWBE_NO_MEMORY;
#else
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
#endif /* JWBO_NO_ALLOC */
	}
	return world->n_ents++;
}

static void unlink_dead(
	jwb_world_t *world,
	jwb_ehandle_t ent,
	jwb_ehandle_t *list)
{
	jwb_ehandle_t next, last;
	next = world->ents[ent].next;
	last = world->ents[ent].last;
	if (next >= 0) {
		world->ents[next].last = last;
	}
	if (last >= 0) {
		world->ents[last].next = next;
	} else {
		*list = next;
	}
}

static void link_dead(
	jwb_world_t *world,
	jwb_ehandle_t ent,
	jwb_ehandle_t *list)
{
	world->ents[ent].last = -1;
	world->ents[ent].next = *list;
	*list = ent;
}

static size_t reposition(jwb_world_t *world, jwb_ehandle_t ent)
{
	size_t x, y;
	struct jwb_vect pos;
	pos = world->ents[ent].pos;
	pos.x = fframe(pos.x, world->width * world->cell_size);
	pos.y = fframe(pos.y, world->height * world->cell_size);
	x = pos.x / world->cell_size;
	y = pos.y / world->cell_size;
	world->ents[ent].pos = pos;
	return y * world->width + x;
}

static void unlink_living(jwb_world_t *world, jwb_ehandle_t ent)
{
	jwb_ehandle_t next, last;
	next = world->ents[ent].next;
	last = world->ents[ent].last;
	if (next >= 0) {
		world->ents[next].last = last;
	}
	if (last >= 0) {
		world->ents[last].next = next;
	} else {
		last = ~last;
		world->cells[last] = next;
	}
}

static void link_living(jwb_world_t *world, jwb_ehandle_t ent, size_t cell_idx)
{
	jwb_ehandle_t cell = world->cells[cell_idx];
	world->ents[ent].last = ~cell_idx;
	world->ents[ent].next = cell;
	if (cell >= 0) {
		world->ents[cell].last = ent;
	}
	world->cells[cell_idx] = ent;
}

static void place_ent(jwb_world_t *world, jwb_ehandle_t ent)
{
	link_living(world, ent, reposition(world, ent));
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
		world->cells = ALLOC(size);
		if (!world->cells) {
			ret = -JWBE_NO_MEMORY;
			goto error_cells;
		}
		memset(world->cells, -1, size);
	}
	if (ent_buf) {
		world->ents = ent_buf;
	} else {
		world->ents = ALLOC(ent_buf_size * JWB_ENTITY_SIZE);
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
	world->available = -1;
	world->flags = HAS_WALLS;
	world->cell_size = 0.;
	return ret;

error_entities:
	if (!cell_buf) {
		FREE(world->cells);
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
	double rad1, rad2, distance;
	double mass1, mass2;
	struct jwb_vect pos1, pos2, relative;
	struct jwb_vect vel1, vel2;
	double bounced1, bounced2;
	double overlap;
	double cor1, cor2;
	jwb_rotation_t rot;
	pos1 = world->ents[ent1].pos;
	pos2 = world->ents[ent2].pos;
	relative.x = pos2.x - pos1.x;
	relative.y = pos2.y - pos1.y;
	rad1 = world->ents[ent1].radius;
	rad2 = world->ents[ent2].radius;
	distance = jwb_vect_magnitude(&relative);
	if (rad1 + rad2 < distance || distance == 0.) {
		return;
	}
	mass1 = world->ents[ent1].mass;
	mass2 = world->ents[ent2].mass;
	vel1 = world->ents[ent1].vel;
	vel2 = world->ents[ent2].vel;
	jwb_vect_rotation(&relative, &rot); /* FIXME: Recalculates magnitude. */
	jwb_rotation_flip(&rot);
	jwb_vect_rotate(&vel1, &rot);
	jwb_vect_rotate(&vel2, &rot);
	bounced1 = (mass1 - mass2) / (mass1 + mass2) * vel1.x + 2 * mass2
		/ (mass1 + mass2) * vel2.x;
	bounced2 = (mass2 - mass1) / (mass2 + mass1) * vel2.x + 2 * mass1
		/ (mass2 + mass1) * vel1.x;
	vel1.x = bounced1;
	vel2.x = bounced2;
	overlap = 1. - distance / (rad1 + rad2);
	cor1 = -overlap / (mass1 / mass2 + 1.);
	cor2 = cor1 + overlap;
	jwb_rotation_flip(&rot);
	jwb_vect_rotate(&vel1, &rot);
	jwb_vect_rotate(&vel2, &rot);
	world->ents[ent1].vel = vel1;
	world->ents[ent2].vel = vel2;
	world->ents[ent1].correct.x += relative.x * cor1;
	world->ents[ent1].correct.y += relative.y * cor1;
	world->ents[ent2].correct.x += relative.x * cor2;
	world->ents[ent2].correct.y += relative.y * cor2;
}

static void update_cell(jwb_world_t *world, size_t x, size_t y)
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

static void update_cells(
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

static void move_ents(jwb_world_t *world, size_t x, size_t y)
{
	size_t here = y * world->width + x;
	jwb_ehandle_t next = world->cells[here];
	while (next >= 0) {
		jwb_ehandle_t self;
		size_t cell;
		self = next;
		next = world->ents[next].next;
		if (world->ents[self].flags & MOVED_THIS_STEP) {
			world->ents[self].flags &= ~MOVED_THIS_STEP;
			continue;
		}
		world->ents[self].pos.x += world->ents[self].vel.x
			+ world->ents[self].correct.x;
		world->ents[self].pos.y += world->ents[self].vel.y
			+ world->ents[self].correct.y;
		world->ents[self].correct.x = 0.;
		world->ents[self].correct.y = 0.;
		cell = reposition(world, self);
		if (cell != here) {
			if (cell > here) {
				world->ents[self].flags |= MOVED_THIS_STEP;
			}
			unlink_living(world, self);
			link_living(world, self, cell);
		}
	}
}

static void cell_translate(
	jwb_world_t *world,
	size_t x,
	size_t y,
	const struct jwb_vect *disp)
{
	jwb_ehandle_t next;
	for (next = world->cells[y * world->width + x];
		next >= 0;
		next = world->ents[next].next)
	{
		world->ents[next].pos.x += disp->x;
		world->ents[next].pos.y += disp->y;
	}
}

static void update_top_left(jwb_world_t *world)
{
	struct jwb_vect wrap_left;
	wrap_left.x = world->cell_size * world->width;
	wrap_left.y = 0.;
	update_cell(world, 0, 0);
	update_cells(world, 0, 0, 1, 0);
	update_cells(world, 0, 0, 1, 1);
	update_cells(world, 0, 0, 0, 1);
	cell_translate(world, 0, 0, &wrap_left);
	update_cells(world, 0, 0, world->width - 1, 1);
	wrap_left.x *= -1;
	cell_translate(world, 0, 0, &wrap_left);
}

static void update_top_right(jwb_world_t *world)
{
	struct jwb_vect wrap_right;
	size_t x = world->width - 1;
	wrap_right.x = -world->cell_size * world->width;
	wrap_right.y = 0.;
	update_cell(world, x, 0);
	cell_translate(world, x, 0, &wrap_right);
	update_cells(world, x, 0, 0, 0);
	update_cells(world, x, 0, 0, 1);
	wrap_right.x *= -1;
	cell_translate(world, x, 0, &wrap_right);
	update_cells(world, x, 0, x, 1);
	update_cells(world, x, 0, x - 1, 1);
}

static void update_left(jwb_world_t *world)
{
	struct jwb_vect wrap_left;
	size_t y;
	wrap_left.x = world->cell_size * world->width;
	wrap_left.y = 0.;
	for (y = 1; y < world->height - 1; ++y) {
		update_cell(world, 0, y);
		update_cells(world, 0, y, 1, y);
		update_cells(world, 0, y, 1, y + 1);
		update_cells(world, 0, y, 0, y + 1);
		cell_translate(world, 0, y, &wrap_left);
		update_cells(world, 0, y, world->width - 1, y + 1);
		wrap_left.x *= -1.;
		cell_translate(world, 0, y, &wrap_left);
		wrap_left.x *= -1.;
	}
}

static void update_middle(jwb_world_t *world)
{
	size_t x, y;
	for (y = 0; y < world->height - 1; ++y) {
		for (x = 1; x < world->width - 1; ++x) {
			update_cell(world, x, y);
			update_cells(world, x, y, x + 1, y);
			update_cells(world, x, y, x + 1, y + 1);
			update_cells(world, x, y, x, y + 1);
			update_cells(world, x, y, x - 1, y + 1);
		}
	}
}

static void update_right(jwb_world_t *world)
{
	struct jwb_vect wrap_right;
	size_t x, y;
	wrap_right.x = -world->cell_size * world->width;
	wrap_right.y = 0.;
	x = world->width - 1;
	for (y = 1; y < world->height - 1; ++y) {
		update_cell(world, x, y);
		cell_translate(world, x, y, &wrap_right);
		update_cells(world, x, y, 0, y);
		update_cells(world, x, y, 0, y + 1);
		wrap_right.x *= -1.;
		cell_translate(world, x, y, &wrap_right);
		wrap_right.x *= -1.;
		update_cells(world, x, y, x, y + 1);
		update_cells(world, x, y, x - 1, y + 1);
	}
}

static void update_bottom_left(jwb_world_t *world)
{
	struct jwb_vect wrap_left, wrap_down;
	size_t y = world->height - 1;
	wrap_left.x = world->cell_size * world->width;
	wrap_left.y = 0.;
	wrap_down.x = 0.;
	wrap_down.y = -world->cell_size * world->height;
	update_cell(world, 0, y);
	update_cells(world, 0, y, 1, y);
	cell_translate(world, 0, y, &wrap_down);
	update_cells(world, 0, y, 1, 0);
	update_cells(world, 0, y, 0, 0);
	cell_translate(world, 0, y, &wrap_left);
	update_cells(world, 0, y, world->width - 1, 0);
	wrap_left.x *= -1.;
	cell_translate(world, 0, y, &wrap_left);
	wrap_down.y *= -1.;
	cell_translate(world, 0, y, &wrap_down);
}

static void update_bottom(jwb_world_t *world)
{
	struct jwb_vect wrap_down;
	double x, y;
	wrap_down.x = 0.;
	wrap_down.y = -world->cell_size * world->height;
	y = world->height - 1;
	for (x = 1; x < world->width - 1; ++x) {
		update_cell(world, x, y);
		update_cells(world, x, y, x + 1, y);
		cell_translate(world, x, y, &wrap_down);
		update_cells(world, x, y, x + 1, 0);
		update_cells(world, x, y, x, 0);
		update_cells(world, x, y, x - 1, 0);
		wrap_down.y *= -1.;
		cell_translate(world, x, y, &wrap_down);
		wrap_down.y *= -1.;
	}
}

static void update_bottom_right(jwb_world_t *world)
{
	struct jwb_vect wrap_right, wrap_down;
	double x, y;
	wrap_right.x = -world->cell_size * world->width;
	wrap_right.y = 0.;
	wrap_down.x = 0.;
	wrap_down.y = -world->cell_size * world->height;
	x = world->width - 1;
	y = world->height - 1;
	update_cell(world, x, y);
	cell_translate(world, x, y, &wrap_right);
	update_cells(world, x, y, 0, y);
	cell_translate(world, x, y, &wrap_down);
	update_cells(world, x, y, 0, 0);
	wrap_right.x *= -1.;
	cell_translate(world, x, y, &wrap_right);
	update_cells(world, x, y, x, 0);
	update_cells(world, x, y, x - 1, 0);
	wrap_down.y *= -1.;
	cell_translate(world, x, y, &wrap_down);
}

void jwb_world_step(jwb_world_t *world)
{
	/* TODO: Unroll this loop a bit. */
	size_t x, y;
	update_top_left(world);
	update_top_right(world);
	update_left(world);
	update_middle(world);
	update_right(world);
	update_bottom_left(world);
	update_bottom(world);
	update_bottom_right(world);
	/*
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
	*/
	for (y = 0; y < world->height; ++y) {
		for (x = 0; x < world->width; ++x) {
			move_ents(world, x, y);
		}
	}
}

void jwb_world_destroy(jwb_world_t *world)
{
	FREE(world->cells);
	FREE(world->ents);
}

jwb_ehandle_t jwb_world_add_ent(
	jwb_world_t *world,
	const struct jwb_vect *pos,
	const struct jwb_vect *vel,
	double mass,
	double radius)
{
	jwb_ehandle_t ent;
	if (world->available >= 0) {
		ent = world->available;
		unlink_dead(world, ent, &world->available);
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
	world->ents[ent].flags = 0;
	place_ent(world, ent);
	return ent;
}

int jwb_world_remove_ent(jwb_world_t *world, jwb_ehandle_t ent)
{
	int status = jwb_world_confirm_ent(world, ent);
	if (status == 0) {
		unlink_living(world, ent);
		link_dead(world, ent, &world->freed);
		world->ents[ent].flags |= REMOVED;
		return 0;
	}
	return status;
}

int jwb_world_destroy_ent(jwb_world_t *world, jwb_ehandle_t ent)
{
	int status = jwb_world_confirm_ent(world, ent);
	switch (-status) {
	case 0:
		unlink_living(world, ent);
		break;
	case JWBE_REMOVED_ENTITY:
		unlink_dead(world, ent, &world->freed);
		break;
	default:
		return status;
	}
	link_dead(world, ent, &world->available);
	world->ents[ent].flags |= DESTROYED;
	return 0;
}

int jwb_world_confirm_ent(jwb_world_t *world, jwb_ehandle_t ent)
{
	int flags;
	if (ent >= (long)world->n_ents) {
		return -JWBE_DESTROYED_ENTITY;
	}
	flags = world->ents[ent].flags;
	if (flags & DESTROYED) {
		return -JWBE_DESTROYED_ENTITY;
	} else if (flags & REMOVED) {
		return -JWBE_REMOVED_ENTITY;
	} else {
		return 0;
	}
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
