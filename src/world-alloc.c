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
	world->on_hit = JWB_WORLD_DEFAULT_HIT_HANDLER;
	world->freed = -1;
	world->available = -1;
	world->flags = HAS_WALLS;
	world->cell_size = JWB_WORLD_DEFAULT_CELL_SIZE;
	return ret;

error_entities:
	if (!cell_buf) {
		FREE(world->cells);
	}
error_cells:
	return ret;
}

void jwb_world_destroy(WORLD *world)
{
	FREE(world->cells);
	FREE(world->ents);
}
