#ifndef JWB_H_
#define JWB_H_

#include <stddef.h>

#define JWBE_NO_MEMORY 1
#define JWBE_REMOVED_ENTITY 2
#define JWBE_DESTROYED_ENTITY 3
#define JWBE_INVALID_ARGUMENT 4
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
	jwb_ehandle_t e2);

typedef struct jwb__world {
	double cell_size;
	jwb_hit_handler_t on_hit;
	size_t width, height;
	size_t n_ents;
	size_t ent_cap;
	jwb_ehandle_t *cells;
	struct jwb__entity *ents;
	jwb_ehandle_t freed;
	jwb_ehandle_t available;
	int flags;
} jwb_world_t;

int jwb_world_alloc(
	jwb_world_t *world,
	size_t width,
	size_t height,
	size_t ent_buf_size,
	void *ent_buf,
	void *cell_buf);

#define JWB_WORLD_DEFAULT_CELL_SIZE 10.

double jwb_world_get_cell_size(jwb_world_t *world);

int jwb_world_set_cell_size(jwb_world_t *world, double cell_size);

double jwb_world_get_cell_size_unck(jwb_world_t *world);

void jwb_world_set_cell_size_unck(jwb_world_t *world, double cell_size);

void jwb_world_set_walls(jwb_world_t *world, int on);

#define JWB_WORLD_DEFAULT_HIT_HANDLER jwb_elastic_collision

jwb_hit_handler_t jwb_world_get_hit_handler(jwb_world_t *world);

void jwb_world_on_hit(jwb_world_t *world, jwb_hit_handler_t on_hit);

void jwb_elastic_collision(
	jwb_world_t *world,
	jwb_ehandle_t e1,
	jwb_ehandle_t e2);

void jwb_world_step(jwb_world_t *world);

int jwb_world_remove_ent(jwb_world_t *world, jwb_ehandle_t ent);

int jwb_world_destroy_ent(jwb_world_t *world, jwb_ehandle_t ent);

void jwb_world_destroy(jwb_world_t *world);

jwb_ehandle_t jwb_world_add_ent(
	jwb_world_t *world,
	const struct jwb_vect *pos,
	const struct jwb_vect *vel,
	double mass,
	double radius);

int jwb_world_confirm_ent(jwb_world_t *world, jwb_ehandle_t ent);

int jwb_world_get_pos(
	jwb_world_t *world,
	jwb_ehandle_t ent,
	struct jwb_vect *dest);

int jwb_world_get_vel(
	jwb_world_t *world,
	jwb_ehandle_t ent,
	struct jwb_vect *dest);

int jwb_world_set_pos(
	jwb_world_t *world,
	jwb_ehandle_t ent,
	const struct jwb_vect *dest);

int jwb_world_set_vel(
	jwb_world_t *world,
	jwb_ehandle_t ent,
	const struct jwb_vect *dest);

int jwb_world_translate(
	jwb_world_t *world,
	jwb_ehandle_t ent,
	const struct jwb_vect *delta);

int jwb_world_accelerate(
	jwb_world_t *world,
	jwb_ehandle_t ent,
	const struct jwb_vect *delta);

void jwb_world_get_pos_unck(
	jwb_world_t *world,
	jwb_ehandle_t ent,
	struct jwb_vect *dest);

void jwb_world_get_vel_unck(
	jwb_world_t *world,
	jwb_ehandle_t ent,
	struct jwb_vect *dest);

void jwb_world_set_pos_unck(
	jwb_world_t *world,
	jwb_ehandle_t ent,
	const struct jwb_vect *dest);

void jwb_world_set_vel_unck(
	jwb_world_t *world,
	jwb_ehandle_t ent,
	const struct jwb_vect *dest);

void jwb_world_translate_unck(
	jwb_world_t *world,
	jwb_ehandle_t ent,
	const struct jwb_vect *delta);

void jwb_world_accelerate_unck(
	jwb_world_t *world,
	jwb_ehandle_t ent,
	const struct jwb_vect *delta);

double jwb_world_get_mass(jwb_world_t *world, jwb_ehandle_t ent);

double jwb_world_get_radius(jwb_world_t *world, jwb_ehandle_t ent);

int jwb_world_set_mass(jwb_world_t *world, jwb_ehandle_t ent, double mass);

int jwb_world_set_radius(jwb_world_t *world, jwb_ehandle_t ent, double radius);

double jwb_world_get_mass_unck(jwb_world_t *world, jwb_ehandle_t ent);

double jwb_world_get_radius_unck(jwb_world_t *world, jwb_ehandle_t ent);

void jwb_world_set_mass_unck(
	jwb_world_t *world,
	jwb_ehandle_t ent,
	double mass);

void jwb_world_set_radius_unck(
	jwb_world_t *world,
	jwb_ehandle_t ent,
	double radius);

typedef int (*jwb_world_iter_t)(
	jwb_world_t *world,
	jwb_ehandle_t ent,
	void *data);

int jwb_world_for_each(jwb_world_t *world, jwb_world_iter_t iter, void *data);

#ifdef JWB_INTERNAL_
typedef jwb_world_t WORLD;
typedef struct jwb_vect VECT;
typedef jwb_ehandle_t EHANDLE;

#	ifdef JWBO_NO_ALLOC
#		define ALLOC(size) ((void)(size), NULL)
#		define FREE(ptr) ((void)(ptr))
#	else
#		define ALLOC(size) malloc((size))
#		define FREE(ptr) free((ptr))
#	endif /* JWBO_NO_ALLOC */

/* Private flags for WORLD::flags */
#	define HAS_WALLS (1 << 0)

/* Private flags for jwb__entity::flags */
#	define REMOVED (1 << 0)
#	define MOVED_THIS_STEP (1 << 1)
#	define DESTROYED (1 << 2)

#endif /* JWB_INTERNAL_ */

#endif /* Header guard */
