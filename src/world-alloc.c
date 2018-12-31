#define JWB_INTERNAL_
#include <jwb.h>
#include <stdlib.h>
#include <string.h>

int jwb_world_alloc(WORLD *world, struct jwb_world_init *info)
{
	int ret = 0;
	if (info->width == 0 || info->height == 0 || info->cell_size <= 0.) {
		ret = -JWBE_INVALID_ARGUMENT;
		goto error_validity;
	}
	world->flags = info->flags;
	world->cell_size = info->cell_size;
	world->width = info->width;
	world->height = info->height;
	if (world->width == 1 || world->height == 1) {
		world->flags |= ONE_CELL_THICK;
		world->width *= 2;
		world->height *= 2;
		world->cell_size /= 2.;
	}
	if (info->cell_buf) {
		world->flags |= PROVIDED_ENT_BUF;
		world->cells = info->cell_buf;
	} else {
		size_t size = world->width * world->height * sizeof(EHANDLE);
		world->cells = ALLOC(size);
		if (!world->cells) {
			ret = -JWBE_NO_MEMORY;
			goto error_cells;
		}
		memset(world->cells, -1, size);
	}
	world->ent_cap = info->ent_buf_size;
	world->ent_size = JWB__ENTITY_SIZE(info->ent_extra);
	if (info->ent_buf) {
		world->ents = info->ent_buf;
	} else {
		world->ents = ALLOC(world->ent_cap * world->ent_size);
		if (!world->ents) {
			ret = -JWBE_NO_MEMORY;
			goto error_entities;
		}
	}
	world->n_ents = 0;
	world->on_hit = JWB_WORLD_DEFAULT_HIT_HANDLER;
	world->freed = -1;
	world->available = -1;
	world->offset.x = world->offset.y = 0.;
	world->tracking = -1;
	return ret;

error_entities:
	if (!info->cell_buf) {
		FREE(world->cells);
	}
error_cells:
error_validity:
	return ret;
}

void jwb_world_destroy(WORLD *world)
{
	FREE(world->cells);
	FREE(world->ents);
}
