#define JWB_INTERNAL_
#include <jwb.h>

int jwb_world_for_each(WORLD *world, jwb_world_iter_t iter, void *data)
{
	EHANDLE ent;
	for (ent = 0; ent < (long)world->n_ents; ++ent) {
		int err;
		if (GET(world, ent).flags & (REMOVED | DESTROYED)) {
			continue;
		}
		err = iter(world, ent, data);
		if (err) {
			return err;
		}
	}
	return 0;
}
