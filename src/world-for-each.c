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
