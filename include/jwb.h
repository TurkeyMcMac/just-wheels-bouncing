#ifndef JWB_H_
#define JWB_H_

#include <stddef.h>

/**
 * # Just Wheels Bouncing
 * This library is for physics simulations involving circles in two dimensions.
 * That's it.
 *
 * **A note about getters and setters**: Getter and setters do not have all
 * parts documented in one place. Read the 'Getters and Setters' section for
 * complete information.
 */

/**
 * ## Error Handling
 * Errors are handled using numeric error codes which can then be described in
 * human-readable text.
 */

/**
 * ### Error Codes
 *  * `JWBE_NO_MEMORY`: A memory allocation failed due to lack of memory.
 *  * `JWBE_REMOVED_ENT`: There was an attempt to access the data of a removed
 *    entity.
 *  * `JWBE_DESTROYED_ENT`: There was an attempt to access the data of a
 *    destroyed entity. This indicates something wrong with your program, and
 *    you should probably abort upon receiving this.
 *  * `JWBE_INVALID_ARGUMENT`: An invalid argument was passed to a function.
 */
#define JWBE_NO_MEMORY 1
#define JWBE_REMOVED_ENTITY 2
#define JWBE_DESTROYED_ENTITY 3
#define JWBE_INVALID_ARGUMENT 4
/**
 * ### `jwb_errmsg`
 * ```
 * const char *jwb_errmsg(int errcode);`
 * ```
 *
 * Return a string describing an error code.
 *
 * #### Parameters
 *  1. `errcode`: The error code. The effect is the same regardless of sign.
 *
 * #### Return Value
 * A description. This cannot be modified or freed.
 */
const char *jwb_errmsg(int errcode);

/**
 * ## Rotation
 * Values used in vector rotation are cached for greater efficiency. Caches can
 * also be created from angles and converted back again.
 */

/**
 * ### `jwb_rotation_t`
 * A cached rotation.
 */
typedef struct {
	double sin, cos;
} jwb_rotation_t;

/**
 * ### `jwb_rotation`
 * ```
 * void jwb_rotation(jwb_rotation_t *rot, double angle);
 * ```
 *
 * Construct a rotation from an angle.
 *
 * #### Parameters
 *  1. `rot`: The destination where the rotation will be cached.
 *  2. `angle`: The angle in radians.
 */
void jwb_rotation(jwb_rotation_t *rot, double angle);

/**
 * ### `jwb_rotation_angle`
 * ```
 * double jwb_rotation_angle(const jwb_rotation_t *rot);
 * ```
 *
 * Get the angle corresponding to a rotation.
 *
 * #### Parameters
 *  1. `rot`: The rotation to examine.
 *
 * #### Return Value
 * The angle in radians.
 */
double jwb_rotation_angle(const jwb_rotation_t *rot);

/**
 * ### `jwb_rotation_flip`
 * ```
 * void jwb_rotation_flip(jwb_rotation_t *rot);
 * ```
 *
 * Flip a rotation to rotate the other way.
 *
 * #### Parameters
 *  1. `rot`: The rotation to flip.
 */
void jwb_rotation_flip(jwb_rotation_t *rot);

/**
 * ## Vectors
 * Vectors are a vital concept in this simulation and have many different uses.
 * They represent position, velocity, acceleration, and more.
 */

/**
 * ### `struct jwb_vect`
 * ```
 * struct jwb_vect {
 *   double x;
 *   double y;
 * };
 * ```
 *
 * A vector on a cartesian coordinate grid.
 *
 * #### Fields
 *  * `x`: The x component.
 *  * `y`: The y component.
 */
struct jwb_vect {
	double x, y;
};

/**
 * ### `jwb_vect_rotate`
 * ```
 * void jwb_vect_rotate(struct jwb_vect *vect, const jwb_rotation_t *rot);
 * ```
 *
 * Rotate a vector.
 *
 * #### Parameters
 *  1. `vect`: The vector to rotate.
 *  2. `rot`: The rotation to apply to the vector.
 */
void jwb_vect_rotate(struct jwb_vect *vect, const jwb_rotation_t *rot);

/**
 * ### `jwb_vect_magnitude`
 * ```
 * double jwb_vect_magnitude(const struct jwb_vect *vect);
 * ```
 *
 * Get the magnitude/length of a vector using the pythagorean theorem.
 *
 * #### Parameters
 *  1. `vect`: The vector to examine.
 *
 * #### Return Value
 * The magnitude.
 */
double jwb_vect_magnitude(const struct jwb_vect *vect);

/**
 * ### `jwb_vect_normalize`
 * ```
 * void jwb_vect_normalize(struct jwb_vect *vect);
 * ```
 *
 * Change the magnitude of a vector to 1 while keeping the x/y ratio the same.
 *
 * #### Parameters
 *  1. `vect`: The vector to shorten or lengthen.
 */
void jwb_vect_normalize(struct jwb_vect *vect);

/**
 * ### `jwb_vect_angle`
 * ```
 * double jwb_vect_angle(const struct jwb_vect *vect);
 * ```
 *
 * Get the angle from the x-axis to the arm of a vector.
 *
 * #### Parameters
 *  1. `vect`: The vector to measure.
 *
 * #### Return Value
 * The angle in radians.
 */
double jwb_vect_angle(const struct jwb_vect *vect);

/**
 * ### `jwb_vect_rotation`
 * ```
 * void jwb_vect_rotation(const struct jwb_vect *vect, jwb_rotation_t *rot);
 * ```
 *
 * Get the rotation from the x-axis to the arm of a vector.
 *
 * #### Parameters
 *  1. `vect`: The vector to measure.
 *  2. `rot`: The place to store the rotation.
 */
void jwb_vect_rotation(const struct jwb_vect *vect, jwb_rotation_t *rot);

/**
 * ## The World Itself
 * All operations in this library revolve around the world structure. All things
 * up to this point are merely auxiliary to this goal.
 */

/**
 * ### `jwb_ehandle_t`
 * An entity handle, representing a certain entity for a certain world. Valid
 * operations on a handle are comparison to zero and passing to appropriate
 * methods, but nothing else.
 */
typedef long jwb_ehandle_t;

struct jwb__entity {
	jwb_ehandle_t next, last;
	struct jwb_vect pos, vel;
	struct jwb_vect correct; /* Correctional displacement */
	double mass;
	double radius;
	int flags;
};

/**
 * ### Size Constants
 * The constants are for manual allocations to be passed to `jwb_world_alloc`.
 *
 *  * `JWB_ENTITY_SIZE`: The size of an entity (not a handle, mind you,) to be
 *    used when manually allocating an entity buffer.
 *  * `JWB_CELL_SIZE`: The size of a segment in the world's internal grid, to be
 *    used when manually allocating a cell buffer.
 */
#define JWB_ENTITY_SIZE sizeof(struct jwb__entity)
#define JWB_CELL_SIZE sizeof(jwb_ehandle_t)

struct jwb_hit_info;
struct jwb__world;
/**
 * ### `jwb_hit_handler_t`
 * ```
 * typedef void (*jwb_hit_handler_t)(
 *   jwb_world_t *world,
 *   jwb_ehandle_t e1,
 *   jwb_ehandle_t e2,
 *   struct jwb_hit_info *info);
 * ```
 * A function for responding when two circles collide. This is called internally
 * by the world. Additional info can be gotten using this method through
 * embedding the world structure.
 *
 * #### Parameters
 *  1. `world`: The world where the interaction takes place.
 *  2. `e1`: The first involved entity.
 *  3. `e2`: The second involved entity.
 *  4. `info`: Information which might be useful for calculations. See the
 *     documentation of `struct jwb_hit_info`.
 *
 * #### Allowed Operations
 * Removal or destruction of either entity is permitted. Normal getters and
 * setters are also allowed, although translation can cause strange behaviour.
 *TODO: Add more details to this section.
 */
typedef void (*jwb_hit_handler_t)(
	struct jwb__world *world,
	jwb_ehandle_t e1,
	jwb_ehandle_t e2,
	struct jwb_hit_info *info);

/**
 * ### `struct jwb_hit_info`
 * ```
 * struct jwb_hit_info {
 *   struct jwb_vect rel;
 *   double dist;
 * };
 * ```
 *
 * Possibly useful information when calculating hits. See the documentation for
 * `jwb_hit_handler_t`.
 *
 * #### Fields
 *  * `rel`: The relative offset from the first entity to the second.
 *  * `dist`: The magnitude of `rel`.
 */
struct jwb_hit_info {
	struct jwb_vect rel;
	double dist;
};

/**
 * ### `jwb_world_t`
 * The world itself. This structure holds and manages a number of entities. It
 * can be quite large, so you might consider allocating it on the heap.
 */
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

/**
 * ### `jwb_world_alloc`
 * ```
 * int jwb_world_alloc(
 *   jwb_world_t *world,
 *   size_t width,
 *   size_t height,
 *   size_t ent_buf_size,
 *   void *ent_buf,
 *   void *cell_buf);
 * ```
 *
 * Allocate the necessary resources for a given world.
 *
 * #### Parameters
 *  1. `world`: The uninitialized world to initialize.
 *  2. `width`: The width of the world in cells.
 *  3. `height`: The height of the world in cells.
 *  4. `ent_buf_size`: The number of entities to allocate initially. If an
 *     entity buffer is given, the buffer is assumed to have this much space.
 *  5. `ent_buf`: The entity buffer. If this is `NULL`, a new one is allocated.
 *     a buffer of size `ent_buf_size * JWB_ENTITY_SIZE` must be provided if
 *     allocation is turned off.
 *  6. `cell_buf`: The cell buffer. If this is `NULL`, a new one is allocated.
 *     a buffer of size `width * height * JWB_CELL_SIZE` must be provided if
 *     allocation is turned off.
 *
 * #### Return Value
 *  * `0`: Success.
 *  * `-JWBE_NO_MEMORY`: Buffer allocation failed.
 */
int jwb_world_alloc(
	jwb_world_t *world,
	size_t width,
	size_t height,
	size_t ent_buf_size,
	void *ent_buf,
	void *cell_buf);

/**
 * #### Defaults
 *  1. The cells size is initially set to `JWB_WORLD_DEFAULT_CELL_SIZE`.
 *  2. The hit handler is initially set to `JWB_WORLD_DEFAULT_HIT_HANDLER`. This
 *     is defined as `jwb_elastic_collision`.
 */
#define JWB_WORLD_DEFAULT_CELL_SIZE 10.
#define JWB_WORLD_DEFAULT_HIT_HANDLER jwb_elastic_collision

/**
 * ### `jwb_elastic_collision`
 * ```
 * void jwb_elastic_collision(
 *   jwb_world_t *world,
 *   jwb_ehandle_t ent1,
 *   jwb_ehandle_t ent2,
 *   struct jwb_hit_info *info);
 * ```
 *
 * Perform a perfectly elastic collision between two circles. This is designed
 * to be used as a hit handler. See the documentation for `jwb_hit_handler_t`.
 */
void jwb_elastic_collision(
	jwb_world_t *world,
	jwb_ehandle_t ent1,
	jwb_ehandle_t ent2,
	struct jwb_hit_info *info);

/**
 * ### `jwb_world_apply_friction`
 * ```
 * void jwb_world_apply_friction(jwb_world_t *world, double friction);
 * ```
 *
 * Apply an acceleration due to friction to every entity in the world.
 *
 * #### Parameters
 *  1. `world`: The world to go through.
 *  2. `friction`: The frictional acceleration to apply. This reduces velocity
 *     when positive.
 */
void jwb_world_apply_friction(jwb_world_t *world, double friction);

/**
 * ### `jwb_world_iter_t`
 * ```
 * typedef int (*jwb_world_iter_t)(
 *   jwb_world_t *world,
 *   jwb_ehandle_t ent,
 *   void *data);
 * ```
 *
 * A function for handling an iteration of looping though all entities.
 * #### Parameters
 *  1. `world`: The world be iterated through.
 *  2. `ent`: The entity being visited this iteration.
 *  3. `data`: Custom data persistent through iterations.
 *
 * #### Return Value
 * If zero is returned, iteration continues. Otherwise, iteration halts.
 */
typedef int (*jwb_world_iter_t)(
	jwb_world_t *world,
	jwb_ehandle_t ent,
	void *data);

/**
 * ### `jwb_world_for_each`
 * ```
 * int jwb_world_for_each(
 *   jwb_world_t *world,
 *   jwb_world_iter_t iter,
 *   void *data);
 * ```
 *
 * Iterate through each existing entity in the world.
 *
 * #### Parameters
 *  1. `world`: The world through which to iterate.
 *  2. `iter`: The iterator function.
 *  3. `data`: Custom persistent state.
 *
 * #### Return Value
 * Zero if all calls to `iter` returned zero. Otherwise, the first non-zero
 * number returned. 
 */
int jwb_world_for_each(jwb_world_t *world, jwb_world_iter_t iter, void *data);

/**
 * ### `jwb_world_step`
 * ```
 * void jwb_world_step(jwb_world_t *world);
 * ```
 *
 * Step the world forward one tick of the simulation.
 *
 * #### Parameters
 *  1. `world`: The world which will be simulated.
 */
void jwb_world_step(jwb_world_t *world);

/**
 * ### `jwb_world_add_ent`
 * ```
 * jwb_ehandle_t jwb_world_add_ent(
 *   jwb_world_t *world,
 *   const struct jwb_vect *pos,
 *   const struct jwb_vect *vel,
 *   double mass,
 *   double radius);
 * ```
 *
 * Add an entity to the world.
 *
 * #### Parameters
 *  1. `world`: The world to which the entity will be added.
 *  2. `pos`: Where to put the entity. Must not be null.
 *  3. `vel`: The initial velocity of the entity. Must not be null.
 *  4. `mass`: The mass of the entity. Must be greater than zero.
 *  5. `radius`: The radius of the entity. Must be greater than zero.
 *
 * #### Return Value
 *  * A handle on the new entity if successful.
 *  * `-JWBE_NO_MEMORY` if there is no room.
 */
jwb_ehandle_t jwb_world_add_ent(
	jwb_world_t *world,
	const struct jwb_vect *pos,
	const struct jwb_vect *vel,
	double mass,
	double radius);

/**
 * ### `jwb_world_remove_ent`
 * ```
 * int jwb_world_remove_ent(jwb_world_t *world, jwb_ehandle_t ent);
 * ```
 *
 * Remove an entity from the world. The space is reserved for future reference.
 *
 * #### Parameters
 *  1. `world`: The world from which to remove the entity.
 *  2. `ent`: The handle of the entity to remove.
 *
 * #### Return Value
 *  * `0`: Success.
 *  * `-JWBE_REMOVED_ENTITY`: The entity was removed already.
 *  * `-JWBE_DESTROYED_ENTITY`: The entity was destroyed.
 */
int jwb_world_remove_ent(jwb_world_t *world, jwb_ehandle_t ent);

/**
 * ### `jwb_world_destroy_ent`
 * ```
 * int jwb_world_destroy_ent(jwb_world_t *world, jwb_ehandle_t ent);
 * ```
 *
 * Recycle an entity for reuse. The handle passed is now invalid.
 *
 * #### Parameters
 *  1. `world`: The world from which to take the entity.
 *  2. `ent`: The handle of the entity to destroy.
 *
 * #### Return Value
 *  * `0`: Success.
 *  * `-JWBE_DESTROYED_ENTITY`: The entity was destroyed.
 */
int jwb_world_destroy_ent(jwb_world_t *world, jwb_ehandle_t ent);

/**
 * ### `jwb_world_destroy`
 * ```
 * void jwb_world_destroy(jwb_world_t *world);
 * ```
 *
 * Free all resources associated with a world. The world is now invalid.
 *
 * #### Parameters
 *  1. `world`: The world to be destroyed.
 */
void jwb_world_destroy(jwb_world_t *world);

/**
 * ### `jwb_world_confirm_ent`
 * ```
 * int jwb_world_confirm_ent(jwb_world_t *world, jwb_ehandle_t ent);
 * ```
 *
 * Confirm that an entity is still around.
 *
 * #### Parameters
 *  1. `world`: The world to look in.
 *  2. `ent`: The entity to check.
 * #### Return Value
 *  * `0`: The entity exists.
 *  * `-JWBE_REMOVED_ENTITY`: The entity has been removed.
 *  * `-JWBE_DESTROYED_ENTITY`: The entity was destroyed.
 */
int jwb_world_confirm_ent(jwb_world_t *world, jwb_ehandle_t ent);

/**
 * ## Getters and Setters
 * All of the following functions simply get or set bits of information. Many
 * an unchecked version, with the suffix _unck, which does not check that the
 * arguments are valid. Unchecked versions might be good where performance is
 * more criticial.
 *
 * Checked versions will _not_ check the validity of the argument `world`.
 * Otherwise, numeric arguments can be restricted to ranges, entity handle
 * arguments are checked for their existence, and pointer arguments are checked
 * both in `NULL`ness and in content. Invalid arguments cause
 * `-JWBE_INVALID_ARGUMENT` to be returned, except in the case of entities,
 * where either `-JWBE_ENTITY_REMOVED` or `-JWBE_ENTITY_DESTROYED` is returned
 * depending on what happened.
 *
 * Most of these are pretty similar, so the descriptions are short.
 */

/**
 * ### `jwb_world_get_cell_size`
 * ```
 * double jwb_world_get_cell_size(jwb_world_t *world);
 * ```
 *
 * #### Parameters
 *  1. `world`: The world to look at.
 *
 * #### Return Value
 * The size of cells in the world.
 */
double jwb_world_get_cell_size(jwb_world_t *world);

/**
 * ### `jwb_world_set_cell_size`
 * ```
 * int jwb_world_set_cell_size(jwb_world_t *world, double cell_size);
 * ```
 *
 * #### Parameters
 *  1. `world`: The world to change.
 *  2. `cell_size`: The cell size, greater than 0.
 */
int jwb_world_set_cell_size(jwb_world_t *world, double cell_size);

/* Unnecessary. Undocumented. */
double jwb_world_get_cell_size_unck(jwb_world_t *world);

/**
 * ### `jwb_world_set_cell_size_unck`
 * ```
 * void jwb_world_set_cell_size_unck(jwb_world_t *world, double cell_size);
 * ```
 *
 * #### Parameters
 *  1. `world`: The world to change.
 *  2. `cell_size`: The cell size, greater than 0.
 */
void jwb_world_set_cell_size_unck(jwb_world_t *world, double cell_size);

/* Not working. Undocumented. */
void jwb_world_set_walls(jwb_world_t *world, int on);

/**
 * ### `jwb_world_get_hit_handler`
 * ```
 * jwb_hit_handler_t jwb_world_get_hit_handler(jwb_world_t *world);
 * ```
 *
 * #### Parameters
 *  1. `world`: The world to examine.
 *
 * #### Return Value
 * The current hit handler.
 */
jwb_hit_handler_t jwb_world_get_hit_handler(jwb_world_t *world);

/**
 * ### `jwb_world_on_hit`
 * ```
 * void jwb_world_on_hit(jwb_world_t *world, jwb_hit_handler_t on_hit);
 * ```
 *
 * Set the hit handler.
 *
 * #### Parameters
 *  1. `world`: The world to change.
 *  2. `on_hit`: The new hit handler.
 */
void jwb_world_on_hit(jwb_world_t *world, jwb_hit_handler_t on_hit);

/**
 * ### `jwb_world_get_pos`
 * ```
 * int jwb_world_get_pos(
 *   jwb_world_t *world,
 *   jwb_ehandle_t ent,
 *   struct jwb_vect *dest);
 * ```
 *
 * #### Parameters
 *  1. `world`: The world to look in.
 *  2. `ent`: The entity to look at.
 *  3. `dest`: Where to place the data.
 */
int jwb_world_get_pos(
	jwb_world_t *world,
	jwb_ehandle_t ent,
	struct jwb_vect *dest);

/**
 * ### `jwb_world_get_vel`
 * ```
 * int jwb_world_get_vel(
 *   jwb_world_t *world,
 *   jwb_ehandle_t ent,
 *   struct jwb_vect *dest);
 * ```
 *
 * #### Parameters
 *  1. `world`: The world to look in.
 *  2. `ent`: The entity to look at.
 *  3. `dest`: Where to place the data.
 */
int jwb_world_get_vel(
	jwb_world_t *world,
	jwb_ehandle_t ent,
	struct jwb_vect *dest);

/**
 * ### `jwb_world_set_pos`
 * ```
 * int jwb_world_set_pos(
 *   jwb_world_t *world,
 *   jwb_ehandle_t ent,
 *   const struct jwb_vect *dest);
 * ```
 *
 * #### Parameters
 *  1. `world`: The world to change.
 *  2. `ent`: The entity to change.
 *  3. `dest`: The data to enter.
 */
int jwb_world_set_pos(
	jwb_world_t *world,
	jwb_ehandle_t ent,
	const struct jwb_vect *dest);

/**
 * ### `jwb_world_set_vel`
 * ```
 * int jwb_world_set_vel(
 *   jwb_world_t *world,
 *   jwb_ehandle_t ent,
 *   const struct jwb_vect *dest);
 * ```
 *
 * #### Parameters
 *  1. `world`: The world to change.
 *  2. `ent`: The entity to change.
 *  3. `dest`: The data to enter.
 */
int jwb_world_set_vel(
	jwb_world_t *world,
	jwb_ehandle_t ent,
	const struct jwb_vect *dest);

/**
 * ### `jwb_world_translate`
 * ```
 * int jwb_world_translate(
 *   jwb_world_t *world,
 *   jwb_ehandle_t ent,
 *   const struct jwb_vect *delta);
 * ```
 *
 * #### Parameters
 *  1. `world`: The world to change.
 *  2. `ent`: The entity to change.
 *  3. `delta`: The displacement to add to the position.
 */
int jwb_world_translate(
	jwb_world_t *world,
	jwb_ehandle_t ent,
	const struct jwb_vect *delta);

/**
 * ### `jwb_world_move_later`
 * ```
 * int jwb_world_move_later(
 *   jwb_world_t *world,
 *   jwb_ehandle_t ent,
 *   const struct jwb_vect *delta);
 * ```
 *
 * Schedule a translation for next update. This is like regular translation, but
 * is allowed inside the hit handler.
 *
 * #### Parameters
 *  1. `world`: The world to change.
 *  2. `ent`: The entity to change.
 *  3. `delta`: The displacement to add to the position.
 */
int jwb_world_move_later(
	jwb_world_t *world,
	jwb_ehandle_t ent,
	const struct jwb_vect *delta);

/**
 * ### `jwb_world_accelerate`
 * ```
 * int jwb_world_accelerate(
 *   jwb_world_t *world,
 *   jwb_ehandle_t ent,
 *   const struct jwb_vect *delta);
 * ```
 *
 * #### Parameters
 *  1. `world`: The world to change.
 *  2. `ent`: The entity to change.
 *  3. `delta`: What to add to the velocity.
 */
int jwb_world_accelerate(
	jwb_world_t *world,
	jwb_ehandle_t ent,
	const struct jwb_vect *delta);
/**
 * ### `jwb_world_get_pos_unck`
 * ```
 * void jwb_world_get_pos_unck(
 *   jwb_world_t *world,
 *   jwb_ehandle_t ent,
 *   struct jwb_vect *dest);
 * ```
 *
 * #### Parameters
 *  1. `world`: The world to look in.
 *  2. `ent`: The entity to look at.
 *  3. `dest`: Where to place the data.
 */
void jwb_world_get_pos_unck(
	jwb_world_t *world,
	jwb_ehandle_t ent,
	struct jwb_vect *dest);

/**
 * ### `jwb_world_get_vel_unck`
 * ```
 * void jwb_world_get_vel_unck(
 *   jwb_world_t *world,
 *   jwb_ehandle_t ent,
 *   struct jwb_vect *dest);
 * ```
 *
 * #### Parameters
 *  1. `world`: The world to look in.
 *  2. `ent`: The entity to look at.
 *  3. `dest`: Where to place the data.
 */
void jwb_world_get_vel_unck(
	jwb_world_t *world,
	jwb_ehandle_t ent,
	struct jwb_vect *dest);

/**
 * ### `jwb_world_set_pos_unck`
 * ```
 * void jwb_world_set_pos_unck(
 *   jwb_world_t *world,
 *   jwb_ehandle_t ent,
 *   const struct jwb_vect *dest);
 * ```
 *
 * #### Parameters
 *  1. `world`: The world to change.
 *  2. `ent`: The entity to change.
 *  3. `dest`: The data to enter.
 */
void jwb_world_set_pos_unck(
	jwb_world_t *world,
	jwb_ehandle_t ent,
	const struct jwb_vect *dest);

/**
 * ### `jwb_world_set_vel_unck`
 * ```
 * void jwb_world_set_vel_unck(
 *   jwb_world_t *world,
 *   jwb_ehandle_t ent,
 *   const struct jwb_vect *dest);
 * ```
 *
 * #### Parameters
 *  1. `world`: The world to change.
 *  2. `ent`: The entity to change.
 *  3. `dest`: The data to enter.
 */
void jwb_world_set_vel_unck(
	jwb_world_t *world,
	jwb_ehandle_t ent,
	const struct jwb_vect *dest);

/**
 * ### `jwb_world_translate_unck`
 * ```
 * void jwb_world_translate_unck(
 *   jwb_world_t *world,
 *   jwb_ehandle_t ent,
 *   const struct jwb_vect *delta);
 * ```
 *
 * #### Parameters
 *  1. `world`: The world to change.
 *  2. `ent`: The entity to change.
 *  3. `delta`: The displacement to add to the position.
 */
void jwb_world_translate_unck(
	jwb_world_t *world,
	jwb_ehandle_t ent,
	const struct jwb_vect *delta);

/**
 * ### `jwb_world_move_later_unck`
 * ```
 * void jwb_world_move_later_unck(
 *   jwb_world_t *world,
 *   jwb_ehandle_t ent,
 *   const struct jwb_vect *delta);
 * ```
 *
 * Schedule a translation for next update. This is like regular translation, but
 * is allowed inside the hit handler.
 *
 * #### Parameters
 *  1. `world`: The world to change.
 *  2. `ent`: The entity to change.
 *  3. `delta`: The displacement to add to the position.
 */
void jwb_world_move_later_unck(
	jwb_world_t *world,
	jwb_ehandle_t ent,
	const struct jwb_vect *delta);

/**
 * ### `jwb_world_accelerate_unck`
 * ```
 * void jwb_world_accelerate_unck(
 *   jwb_world_t *world,
 *   jwb_ehandle_t ent,
 *   const struct jwb_vect *delta);
 * ```
 *
 * #### Parameters
 *  1. `world`: The world to change.
 *  2. `ent`: The entity to change.
 *  3. `delta`: What to add to the velocity.
 */
void jwb_world_accelerate_unck(
	jwb_world_t *world,
	jwb_ehandle_t ent,
	const struct jwb_vect *delta);

/**
 * ### `jwb_world_get_mass`
 * ```
 * double jwb_world_get_mass(jwb_world_t *world, jwb_ehandle_t ent);
 * ```
 *
 * #### Parameters
 *  1. `world`: The world to look at.
 *  2. `ent`: The entity to look in.
 * #### Return Value
 * The mass of the entity, or a negative error code on nonexistence.
 */
double jwb_world_get_mass(jwb_world_t *world, jwb_ehandle_t ent);

/**
 * ### `jwb_world_get_radius`
 * ```
 * double jwb_world_get_radius(jwb_world_t *world, jwb_ehandle_t ent);
 * ```
 *
 * #### Parameters
 *  1. `world`: The world to look at.
 *  2. `ent`: The entity to look in.
 * #### Return Value
 * The radius of the entity, or a negative error code on nonexistence.
 */
double jwb_world_get_radius(jwb_world_t *world, jwb_ehandle_t ent);

/**
 * ### `jwb_world_set_mass`
 * ```
 * int jwb_world_set_mass(jwb_world_t *world, jwb_ehandle_t ent, double mass);
 * ```
 *
 * #### Parameters
 *  1. `world`: The world to change.
 *  2. `ent`: The entity to change.
 *  3. `mass`: What to set the mass to. Must be over zero.
 */
int jwb_world_set_mass(jwb_world_t *world, jwb_ehandle_t ent, double mass);

/**
 * ### `jwb_world_set_radius`
 * ```
 * int jwb_world_set_radius(
 *   jwb_world_t *world,
 *   jwb_ehandle_t ent,
 *   double radius);
 * ```
 *
 * #### Parameters
 *  1. `world`: The world to change.
 *  2. `ent`: The entity to change.
 *  3. `radius`: What to set the radius to. Must be over zero.
 */
int jwb_world_set_radius(jwb_world_t *world, jwb_ehandle_t ent, double radius);

/**
 * ### `jwb_world_get_mass_unck`
 * ```
 * double jwb_world_get_mass_unck(jwb_world_t *world, jwb_ehandle_t ent);
 * ```
 *
 * #### Parameters
 *  1. `world`: The world to look at.
 *  2. `ent`: The entity to look in.
 * #### Return Value
 * The mass of the entity.
 */
double jwb_world_get_mass_unck(jwb_world_t *world, jwb_ehandle_t ent);

/**
 * ### `jwb_world_get_radius_unck`
 * ```
 * double jwb_world_get_radius_unck(jwb_world_t *world, jwb_ehandle_t ent);
 * ```
 *
 * #### Parameters
 *  1. `world`: The world to look at.
 *  2. `ent`: The entity to look in.
 * #### Return Value
 * The radius of the entity.
 */
double jwb_world_get_radius_unck(jwb_world_t *world, jwb_ehandle_t ent);

/**
 * ### `jwb_world_set_mass_unck`
 * ```
 * void jwb_world_set_mass_unck(
 *   jwb_world_t *world,
 *   jwb_ehandle_t ent,
 *   double mass);
 * ```
 *
 * #### Parameters
 *  1. `world`: The world to change.
 *  2. `ent`: The entity to change.
 *  3. `mass`: What to set the mass to. Must be over zero.
 */
void jwb_world_set_mass_unck(
	jwb_world_t *world,
	jwb_ehandle_t ent,
	double mass);

/**
 * ### `jwb_world_set_radius_unck`
 * ```
 * void jwb_world_set_radius_unck(
 *   jwb_world_t *world,
 *   jwb_ehandle_t ent,
 *   double radius);
 * ```
 *
 * #### Parameters
 *  1. `world`: The world to change.
 *  2. `ent`: The entity to change.
 *  3. `radius`: What to set the radius to. Must be over zero.
 */
void jwb_world_set_radius_unck(
	jwb_world_t *world,
	jwb_ehandle_t ent,
	double radius);

#ifdef JWB_INTERNAL_
/* Internal type name shortcuts, macros, and flags. */

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
