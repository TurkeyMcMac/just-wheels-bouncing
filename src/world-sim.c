#define JWB_INTERNAL_
#include <jwb.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

static double fframe(double num, double lim)
{
	double mod = fmod(num, lim);
	if (mod < 0) {
		mod += lim;
	}
	return mod;
}

static EHANDLE alloc_new_ent(WORLD *world)
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

static void unlink_dead(WORLD *world,
	EHANDLE ent,
	EHANDLE *list)
{
	EHANDLE next, last;
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

static void link_dead(WORLD *world, EHANDLE ent, EHANDLE *list)
{
	world->ents[ent].last = -1;
	world->ents[ent].next = *list;
	*list = ent;
}

static size_t reposition(WORLD *world, EHANDLE ent)
{
	size_t x, y;
	VECT pos;
	pos = world->ents[ent].pos;
	pos.x = fframe(pos.x, world->width * world->cell_size);
	pos.y = fframe(pos.y, world->height * world->cell_size);
	x = pos.x / world->cell_size;
	y = pos.y / world->cell_size;
	world->ents[ent].pos = pos;
	return y * world->width + x;
}

static void unlink_living(WORLD *world, EHANDLE ent)
{
	EHANDLE next, last;
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

static void link_living(WORLD *world, EHANDLE ent, size_t cell_idx)
{
	EHANDLE cell = world->cells[cell_idx];
	world->ents[ent].last = ~cell_idx;
	world->ents[ent].next = cell;
	if (cell >= 0) {
		world->ents[cell].last = ent;
	}
	world->cells[cell_idx] = ent;
}

static void place_ent(WORLD *world, EHANDLE ent)
{
	link_living(world, ent, reposition(world, ent));
}

static void check_hit(WORLD *world, EHANDLE ent1, EHANDLE ent2)
{
	struct jwb_hit_info info;
	info.rel.x = world->ents[ent2].pos.x - world->ents[ent1].pos.x;
	info.rel.y = world->ents[ent2].pos.y - world->ents[ent1].pos.y;
	info.dist = jwb_vect_magnitude(&info.rel);
	if (info.dist < world->ents[ent1].radius + world->ents[ent2].radius) {
		world->on_hit(world, ent1, ent2, &info);
	}
}

static void update_cell(WORLD *world, size_t x, size_t y)
{
	EHANDLE next = world->cells[y * world->width + x];
	while (next >= 0) {
		EHANDLE self, next_other;
		self = next;
		next = world->ents[next].next;
		next_other = next;
		while (next_other >= 0) {
			EHANDLE other;
			other = next_other;
			next_other = world->ents[next_other].next;
			check_hit(world, self, other);
		}
	}
}

static void update_cells(
	WORLD *world,
	size_t x1,
	size_t y1,
	size_t x2,
	size_t y2)
{
	EHANDLE next1, next2;
	next1 = world->cells[y1 * world->width + x1];
	next2 = world->cells[y2 * world->width + x2];
	while (next1 >= 0) {
		EHANDLE self, next_other;
		self = next1;
		next1 = world->ents[next1].next;
		next_other = next2;
		while (next_other >= 0) {
			EHANDLE other;
			other = next_other;
			next_other = world->ents[next_other].next;
			check_hit(world, self, other);
		}
	}
}

static void move_ents(WORLD *world, size_t x, size_t y)
{
	size_t here = y * world->width + x;
	EHANDLE next = world->cells[here];
	while (next >= 0) {
		EHANDLE self;
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

static void cell_translate(WORLD *world, size_t x, size_t y, const VECT *disp)
{
	EHANDLE next;
	for (next = world->cells[y * world->width + x];
		next >= 0;
		next = world->ents[next].next)
	{
		world->ents[next].pos.x += disp->x;
		world->ents[next].pos.y += disp->y;
	}
}

static void update_top_left(WORLD *world)
{
	VECT wrap_left;
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

static void update_top_right(WORLD *world)
{
	VECT wrap_right;
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

static void update_left(WORLD *world)
{
	VECT wrap_left;
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

static void update_middle(WORLD *world)
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

static void update_right(WORLD *world)
{
	VECT wrap_right;
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

static void update_bottom_left(WORLD *world)
{
	VECT wrap_left, wrap_down;
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

static void update_bottom(WORLD *world)
{
	VECT wrap_down;
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

static void update_bottom_right(WORLD *world)
{
	VECT wrap_right, wrap_down;
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

void jwb_world_step(WORLD *world)
{
	size_t x, y;
	update_top_left(world);
	update_top_right(world);
	update_left(world);
	update_middle(world);
	update_right(world);
	update_bottom_left(world);
	update_bottom(world);
	update_bottom_right(world);
	for (y = 0; y < world->height; ++y) {
		for (x = 0; x < world->width; ++x) {
			move_ents(world, x, y);
		}
	}
}

EHANDLE jwb_world_add_ent(WORLD *world,
	const VECT *pos,
	const VECT *vel,
	double mass,
	double radius)
{
	EHANDLE ent;
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

int jwb_world_remove_ent(WORLD *world, EHANDLE ent)
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

int jwb_world_destroy_ent(WORLD *world, EHANDLE ent)
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

int jwb_world_confirm_ent(WORLD *world, EHANDLE ent)
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
