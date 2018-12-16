#ifndef JWB_H_
#define JWB_H_

#include <stddef.h>

#define JWBE_NO_MEMORY 1
#define JWBE_REMOVED_ENTITY 2
#define JWBE_DESTROYED_ENTITY 3
const char *jwb_errmsg(int errcode);

typedef struct {
	double sin, cos;
} jwb_rotation_t;

void jwb_rotation(jwb_rotation_t *rot, double angle);

double jwb_rotation_angle(const jwb_rotation_t *rot);

void jwb_rotation_flip(jwb_rotation_t *rot);

struct jwb_vect {
	double x, y;
};

void jwb_vect_rotate(struct jwb_vect *vect, const jwb_rotation_t *rot);

double jwb_vect_magnitude(const struct jwb_vect *vect);

void jwb_vect_normalize(struct jwb_vect *vect);

double jwb_vect_angle(const struct jwb_vect *vect);

void jwb_vect_rotation(const struct jwb_vect *vect, jwb_rotation_t *rot);

typedef long jwb_ehandle_t;
#define JWB_CELL_SIZE sizeof(jwb_ehandle_t)

struct jwb__entity {
	jwb_ehandle_t next, last;
	struct jwb_vect pos, vel;
	struct jwb_vect correct; /* Correctional displacement */
	double mass;
	double radius;
	int flags;
};
#define JWB_ENTITY_SIZE sizeof(struct jwb__entity)

struct jwb__world;
typedef void (*jwb_hit_handler_t)(
	struct jwb__world *world,
	jwb_ehandle_t e1,
	jwb_ehandle_t e2,
	struct jwb_vect *where);

typedef struct jwb__world {
	double cell_size;
	jwb_hit_handler_t on_hit;
	size_t width, height;
	size_t n_ents;
	size_t ent_cap;
	jwb_ehandle_t *cells;
	struct jwb__entity *ents;
	jwb_ehandle_t freed;
	int flags;
} jwb_world_t;

int jwb_world_alloc(
	jwb_world_t *world,
	size_t width,
	size_t height,
	size_t ent_buf_size,
	void *ent_buf,
	void *cell_buf);

void jwb_world_set_walls(jwb_world_t *world, int on);

void jwb_world_on_hit(jwb_world_t *world, jwb_hit_handler_t on_hit);

void jwb_world_step(jwb_world_t *world);

int jwb_world_remove_ent(jwb_world_t *world, jwb_ehandle_t ent);

void jwb_world_clear_removed(jwb_world_t *world);

void jwb_world_destroy(jwb_world_t *world);

jwb_ehandle_t jwb_world_add_ent(
	jwb_world_t *world,
	const struct jwb_vect *pos,
	const struct jwb_vect *vel,
	double mass,
	double radius);

int jwb_world_confirm_ent(jwb_world_t *world, jwb_ehandle_t ent);

void jwb_world_get_pos(
	jwb_world_t *world,
	jwb_ehandle_t ent,
	struct jwb_vect *dest);

void jwb_world_get_vel(
	jwb_world_t *world,
	jwb_ehandle_t ent,
	struct jwb_vect *dest);

void jwb_world_set_pos(
	jwb_world_t *world,
	jwb_ehandle_t ent,
	const struct jwb_vect *dest);

void jwb_world_set_vel(
	jwb_world_t *world,
	jwb_ehandle_t ent,
	const struct jwb_vect *dest);

void jwb_world_translate(
	jwb_world_t *world,
	jwb_ehandle_t ent,
	const struct jwb_vect *delta);

void jwb_world_accelerate(
	jwb_world_t *world,
	jwb_ehandle_t ent,
	const struct jwb_vect *delta);

double jwb_world_get_mass(jwb_world_t *world, jwb_ehandle_t ent);

double jwb_world_get_radius(jwb_world_t *world, jwb_ehandle_t ent);

void jwb_world_set_mass(jwb_world_t *world, jwb_ehandle_t ent, double mass);

void jwb_world_set_radius(jwb_world_t *world, jwb_ehandle_t ent, double radius);

#endif /* Header guard */
