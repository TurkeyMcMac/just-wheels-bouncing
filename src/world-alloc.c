#define JWB_INTERNAL_
#include <jwb.h>
#include <stdlib.h>
#include <string.h>

int jwb_world_alloc(
	WORLD *world,
	size_t width,
	size_t height,
	size_t ent_buf_size,
	void *ent_buf,
	void *cell_buf)
{
	int ret = 0;
	if (width == 0 || height == 0) {
		ret = -JWBE_INVALID_ARGUMENT;
		goto error_validity;
	}
	world->flags = 0;
	world->cell_size = JWB_WORLD_DEFAULT_CELL_SIZE;
	if (width == 1 || height == 1) {
		world->flags |= ONE_CELL_THICK;
		width *= 2;
		height *= 2;
		world->cell_size /= 2.;
	}
	if (cell_buf) {
		world->cells = cell_buf;
	} else {
		size_t size = width * height * sizeof(EHANDLE);
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
		world->ents = ALLOC(ent_buf_size * sizeof(struct jwb__entity));
		if (!world->ents) {
			ret = -JWBE_NO_MEMORY;
			goto error_entities;
		}
	}
	world->width = width;
	world->height = height;
	world->n_ents = 0;
	world->ent_cap = ent_buf_size;
	world->on_hit = JWB_WORLD_DEFAULT_HIT_HANDLER;
	world->freed = -1;
	world->available = -1;
	return ret;

error_entities:
	if (!cell_buf) {
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
