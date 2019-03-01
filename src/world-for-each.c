#define JWB_INTERNAL_
#include <jwb.h>

static EHANDLE next(WORLD *world, EHANDLE now)
{
	do {
		++now;
	} while (now < (long)world->n_ents
	    && GET(world, now).flags & (REMOVED | DESTROYED));
	return now < (long)world->n_ents ? now : -1;
}

int jwb_world_first_perm(WORLD *world, EHANDLE *ent1, EHANDLE *ent2)
{
	*ent1 = next(world, -1);
	if (*ent1 < 0) {
		return -1;
	}
	*ent2 = next(world, *ent1);
	if (*ent2 < 0) {
		return -1;
	}
	return 0;
}

int jwb_world_next_perm(WORLD *world, EHANDLE *ent1, EHANDLE *ent2)
{
	if (*ent1 == -1) {
		/* Failsafe to prevent infinite looping. */
		return -1;
	}
	*ent2 = next(world, *ent2);
	if (*ent2 >= 0) {
		return 0;
	}
	*ent1 = next(world, *ent1);
	if (*ent1 < 0) {
		return -1;
	}
	*ent2 = next(world, *ent1);
	if (*ent2 < 0) {
		return -1;
	}
	return 0;
}

EHANDLE jwb_world_first(WORLD *world)
{
	return next(world, -1);
}

EHANDLE jwb_world_next(WORLD *world, EHANDLE now)
{
	return now >= 0 ? next(world, now) : -1;
}

EHANDLE jwb_world_first_removed(WORLD *world)
{
	return world->freed;
}

EHANDLE jwb_world_next_removed(WORLD *world, EHANDLE now)
{
	return now >= 0 ? GET(world, now).next : -1;
}
