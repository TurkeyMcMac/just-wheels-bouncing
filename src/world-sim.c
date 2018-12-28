#define JWB_INTERNAL_
#include <jwb.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

static jwb_num_t fframe(jwb_num_t num, jwb_num_t lim)
{
	jwb_num_t mod = fmod(num, lim);
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
		char *new_buf;
		if (world->flags & PROVIDED_ENT_BUF) {
			return -JWBE_NO_MEMORY;
		}
		new_cap = world->ent_cap * 3 / 2 + 1;
		new_buf = realloc(world->ents, new_cap * world->ent_size);
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
	next = GET(world, ent).next;
	last = GET(world, ent).last;
	if (next >= 0) {
		GET(world, next).last = last;
	}
	if (last >= 0) {
		GET(world, last).next = next;
	} else {
		*list = next;
	}
}

static void link_dead(WORLD *world, EHANDLE ent, EHANDLE *list)
{
	GET(world, ent).last = -1;
	GET(world, ent).next = *list;
	*list = ent;
}

static void unlink_living(WORLD *world, EHANDLE ent)
{
	EHANDLE next, last;
	next = GET(world, ent).next;
	last = GET(world, ent).last;
	if (next >= 0) {
		GET(world, next).last = last;
	}
	if (last >= 0) {
		GET(world, last).next = next;
	} else {
		last = ~last;
		world->cells[last] = next;
	}
}

static void link_living(WORLD *world, EHANDLE ent, size_t cell_idx)
{
	EHANDLE cell = world->cells[cell_idx];
	GET(world, ent).last = ~cell_idx;
	GET(world, ent).next = cell;
	if (cell >= 0) {
		GET(world, cell).last = ent;
	}
	world->cells[cell_idx] = ent;
}

static void remove_unck(WORLD *world, EHANDLE ent)
{
	unlink_living(world, ent);
	link_dead(world, ent, &world->freed);
	GET(world, ent).flags |= REMOVED;
}

static void pos_to_idx(WORLD *world, struct jwb_vect *pos, size_t *x, size_t *y)
{
	*x = pos->x / world->cell_size;
	*y = pos->y / world->cell_size;
}

static size_t reposition(WORLD *world, EHANDLE ent)
{
	size_t x, y;
	VECT pos;
	pos = GET(world, ent).pos;
	pos.x -= world->offset.x;
	pos.y -= world->offset.y;
	pos.x = fframe(pos.x, world->width * world->cell_size);
	pos.y = fframe(pos.y, world->height * world->cell_size);
	pos_to_idx(world, &pos, &x, &y);
	pos.x += world->offset.x;
	pos.y += world->offset.y;
	GET(world, ent).pos = pos;
	return y * world->width + x;
}

static size_t reposition_nowrap(WORLD *world, EHANDLE ent)
{
	size_t x, y;
	VECT pos;
	pos = GET(world, ent).pos;
	pos.x -= world->offset.x;
	pos.y -= world->offset.y;
	pos_to_idx(world, &pos, &x, &y);
	if (x >= world->width || y >= world->height) {
		return -1;
	}
	return y * world->width + x;
}

static void place_ent(WORLD *world, EHANDLE ent)
{
	size_t cell;
	if (REMOVING_DISTANT(world)) {
		cell = reposition_nowrap(world, ent);
		if (cell == (size_t)-1) {
			return;
		}
	} else {
		cell = reposition(world, ent);
	}
	link_living(world, ent, cell);
}

static void check_hit(WORLD *world, EHANDLE ent1, EHANDLE ent2)
{
	struct jwb_hit_info info;
	info.rel.x = GET(world, ent2).pos.x - GET(world, ent1).pos.x;
	info.rel.y = GET(world, ent2).pos.y - GET(world, ent1).pos.y;
	info.dist = jwb_vect_magnitude(&info.rel);
	if (info.dist < GET(world, ent1).radius + GET(world, ent2).radius) {
		world->on_hit(world, ent1, ent2, &info);
	}
}

static void update_cell(WORLD *world, size_t x, size_t y)
{
	EHANDLE next = world->cells[y * world->width + x];
	while (next >= 0) {
		EHANDLE self, next_other;
		self = next;
		next = GET(world, next).next;
		next_other = next;
		while (next_other >= 0) {
			EHANDLE other;
			other = next_other;
			next_other = GET(world, next_other).next;
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
		next1 = GET(world, next1).next;
		next_other = next2;
		while (next_other >= 0) {
			EHANDLE other;
			other = next_other;
			next_other = GET(world, next_other).next;
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
		next = GET(world, next).next;
		if (GET(world, self).flags & MOVED_THIS_STEP) {
			GET(world, self).flags &= ~MOVED_THIS_STEP;
			continue;
		}
		GET(world, self).pos.x += GET(world, self).vel.x
			+ GET(world, self).correct.x;
		GET(world, self).pos.y += GET(world, self).vel.y
			+ GET(world, self).correct.y;
		GET(world, self).correct.x = 0.;
		GET(world, self).correct.y = 0.;
		if (REMOVING_DISTANT(world)) {
			cell = reposition_nowrap(world, self);
			if (cell == (size_t)-1) {
				remove_unck(world, self);
				continue;
			}
		} else {
			cell = reposition(world, self);
		}
		if (cell != here) {
			if (cell > here) {
				GET(world, self).flags |= MOVED_THIS_STEP;
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
		next = GET(world, next).next)
	{
		GET(world, next).pos.x += disp->x;
		GET(world, next).pos.y += disp->y;
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

static void update_top_left_nowrap(WORLD *world)
{
	update_cell(world, 0, 0);
	update_cells(world, 0, 0, 1, 0);
	update_cells(world, 0, 0, 1, 1);
	update_cells(world, 0, 0, 0, 1);
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

static void update_top_right_nowrap(WORLD *world)
{
	size_t x = world->width - 1;
	update_cell(world, x, 0);
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

static void update_left_nowrap(WORLD *world)
{
	size_t y;
	for (y = 1; y < world->height - 1; ++y) {
		update_cell(world, 0, y);
		update_cells(world, 0, y, 1, y);
		update_cells(world, 0, y, 1, y + 1);
		update_cells(world, 0, y, 0, y + 1);
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

static void update_right_nowrap(WORLD *world)
{
	size_t x, y;
	x = world->width - 1;
	for (y = 1; y < world->height - 1; ++y) {
		update_cell(world, x, y);
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

static void update_bottom_left_nowrap(WORLD *world)
{
	size_t y = world->height - 1;
	update_cell(world, 0, y);
	update_cells(world, 0, y, 1, y);
}

static void update_bottom(WORLD *world)
{
	VECT wrap_down;
	size_t x, y;
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

static void update_bottom_nowrap(WORLD *world)
{
	size_t x, y;
	y = world->height - 1;
	for (x = 1; x < world->width - 1; ++x) {
		update_cell(world, x, y);
		update_cells(world, x, y, x + 1, y);
	}
}

static void update_bottom_right(WORLD *world)
{
	VECT wrap_right, wrap_down;
	jwb_num_t x, y;
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
	if (REMOVING_DISTANT(world)) {
		if (world->width == 1) {
			update_cell(world, 0, 0);
			for (y = 1; y < world->height; ++y) {
				update_cells(world, 0, y - 1, 0, y);
				update_cell(world, 0, y);
			}
		} else if (world->height == 1) {
			update_cell(world, 0, 0);
			for (x = 1; x < world->height; ++x) {
				update_cells(world, x - 1, 0, x, 0);
				update_cell(world, x, 0);
			}
		} else {
			update_top_left_nowrap(world);
			update_top_right_nowrap(world);
			update_left_nowrap(world);
			update_middle(world);
			update_right_nowrap(world);
			update_bottom_left_nowrap(world);
			update_bottom_nowrap(world);
		}
	} else {
		update_top_left(world);
		update_top_right(world);
		update_left(world);
		update_middle(world);
		update_right(world);
		update_bottom_left(world);
		update_bottom(world);
		update_bottom_right(world);
	}
	if (world->tracking >= 0) {
		struct jwb__entity *tracked = &GET(world, world->tracking);
		if (tracked->flags & REMOVED) {
			world->tracking = -1;
		} else {
			world->offset.x += tracked->correct.x + tracked->vel.x;
			world->offset.y += tracked->correct.y + tracked->vel.y;
		}
	}
	for (y = 0; y < world->height; ++y) {
		for (x = 0; x < world->width; ++x) {
			move_ents(world, x, y);
		}
	}
}

EHANDLE jwb_world_add_ent(WORLD *world,
	const VECT *pos,
	const VECT *vel,
	jwb_num_t mass,
	jwb_num_t radius)
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
	GET(world, ent).pos = *pos;
	GET(world, ent).vel = *vel;
	GET(world, ent).correct.x = 0.;
	GET(world, ent).correct.y = 0.;
	GET(world, ent).mass = mass;
	GET(world, ent).radius = radius;
	GET(world, ent).flags = 0;
	place_ent(world, ent);
	return ent;
}

int jwb_world_re_add_ent(WORLD *world, EHANDLE ent)
{
	int err = jwb_world_confirm_ent(world, ent);
	switch (-err) {
	case JWBE_REMOVED_ENTITY:
		GET(world, ent).flags &= ~REMOVED;
		place_ent(world, ent);
		return 0;
	case 0:
		return 0;
	default:
		return err;
	}
}

int jwb_world_remove_ent(WORLD *world, EHANDLE ent)
{
	int status = jwb_world_confirm_ent(world, ent);
	if (status == 0) {
		remove_unck(world, ent);
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
	GET(world, ent).flags |= DESTROYED;
	return 0;
}

int jwb_world_confirm_ent(WORLD *world, EHANDLE ent)
{
	int flags;
	if (ent >= (long)world->n_ents) {
		return -JWBE_DESTROYED_ENTITY;
	}
	flags = GET(world, ent).flags;
	if (flags & DESTROYED) {
		return -JWBE_DESTROYED_ENTITY;
	} else if (flags & REMOVED) {
		return -JWBE_REMOVED_ENTITY;
	} else {
		return 0;
	}
}
