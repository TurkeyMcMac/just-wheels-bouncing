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
 * ## Compilation Options
 * Special symbols can be used to fine-tune the library. The options used when
 * compiling the library **must** be the same as the ones used when compiling
 * programs which use it.
 *
 * ### Symbols
 *  * `JWBO_NO_ALLOC`: Never allocate anything internally. Always rely on
 *    buffers given by the user. Using this option, the number of entities can
 *    never be more than the number initially accounted for.
 *  * `JWBO_NUM_FLOAT`: The numeric type is switched to `float`, rather than the
 *    default, which is `double`. This can save some space.
 *  * `JWBO_EXTRA_ALIGN_4`: Align the extra data of entities to a 4-byte
 *    boundary. This saves a bit, but is less flexible than the default 8-byte
 *    alignment.
 *  * `JWBO_ALWAYS_REMOVE_DISTANT`: The flag `JWBF_REMOVE_DISTANT` is
 *    effectively always active. See `struct jwb_world_init`.
 *  * `JWBO_NEVER_REMOVE_DISTANT`: The flag `JWBF_REMOVE_DISTANT` is
 *    effectively always inactive. See `struct jwb_world_init`.
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
 * ## Scalars
 *
 * ### `jwb_num_t`
 * This is the scalar type for all floating-point calculations in the library.
 * The default is `double`, but `float` can be specified by defining
 * `JWBO_NUM_FLOAT` during compilation.
 */
#ifdef JWBO_NUM_FLOAT
#	if !(__FLT_MANT_DIG__ == 24 && __FLT_MAX_EXP__ == 128)
#		error float is not 32 bits.
#	endif
typedef float jwb_num_t;
#else
#	if !(__DBL_MANT_DIG__ == 53 && __DBL_MAX_EXP__ == 1024)
#		error double is not 64 bits.
#	endif
typedef double jwb_num_t;
#endif


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
	jwb_num_t sin, cos;
} jwb_rotation_t;

/**
 * ### `jwb_rotation`
 * ```
 * void jwb_rotation(jwb_rotation_t *rot, jwb_num_t angle);
 * ```
 *
 * Construct a rotation from an angle.
 *
 * #### Parameters
 *  1. `rot`: The destination where the rotation will be cached.
 *  2. `angle`: The angle in radians.
 */
void jwb_rotation(jwb_rotation_t *rot, jwb_num_t angle);

/**
 * ### `jwb_rotation_angle`
 * ```
 * jwb_num_t jwb_rotation_angle(const jwb_rotation_t *rot);
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
jwb_num_t jwb_rotation_angle(const jwb_rotation_t *rot);

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
 *   jwb_num_t x;
 *   jwb_num_t y;
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
	jwb_num_t x, y;
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
 * jwb_num_t jwb_vect_magnitude(const struct jwb_vect *vect);
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
jwb_num_t jwb_vect_magnitude(const struct jwb_vect *vect);

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
 * jwb_num_t jwb_vect_angle(const struct jwb_vect *vect);
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
jwb_num_t jwb_vect_angle(const struct jwb_vect *vect);

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

#define JWB__ALIGN(num, size) (((num) + (size) - 1) / (size) * (size))

struct jwb__entity {
	jwb_ehandle_t next, last;
	struct jwb_vect pos, vel;
	struct jwb_vect correct; /* Correctional displacement */
	jwb_num_t mass;
	jwb_num_t radius;
	int flags;
#ifndef JWBO_EXTRA_ALIGN_4
	/* Extra has 8-byte alignment. */
	char padding_[8 - sizeof(int)];
	char extra[1 /* Variadic */];
#	define JWB__ENTITY_SIZE(extra) \
	(sizeof(struct jwb__entity) - 8 + JWB__ALIGN((extra), 8))
#	define JWB__ENTITY_EXTRA_MIN_SIZE 0
#elif defined(JWBO_NUM_FLOAT) && ULONG_MAX == 4294967295
	/* Struct has 4-byte alignment. */
	char extra[1 /* Variadic */];
#	define JWB__ENTITY_SIZE(extra) \
	(sizeof(struct jwb__entity) - 4 + JWB__ALIGN((extra), 4))
#	define JWB__ENTITY_EXTRA_MIN_SIZE 0
#else
	/* Struct has 8-byte alignment. */
#	define JWB__ENTITY_EXTRA_MIN_SIZE (8 - sizeof(int))
	char extra[JWB__ENTITY_EXTRA_MIN_SIZE /* Variadic */];
#	define JWB__ENTITY_SIZE(extra) (sizeof(struct jwb__entity) \
		+ ((extra) <= JWB__ENTITY_EXTRA_MIN_SIZE \
		? 0 : JWB__ALIGN((extra) - JWB__ENTITY_EXTRA_MIN_SIZE, 4)))
#endif /* !defined(JWBO_EXTRA_ALIGN_4) */
};

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
 *   jwb_num_t dist;
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
	jwb_num_t dist;
};

/**
 * ### `jwb_world_t`
 * The world itself. This structure holds and manages a number of entities. It
 * can be quite large, so you might consider allocating it on the heap.
 */
typedef struct jwb__world {
	jwb_num_t cell_size;
	struct jwb_vect offset;
	jwb_hit_handler_t on_hit;
	size_t width, height;
	size_t n_ents;
	size_t ent_cap;
	size_t ent_size;
	jwb_ehandle_t *cells;
	char *ents;
	jwb_ehandle_t freed;
	jwb_ehandle_t available;
	jwb_ehandle_t tracking;
	int flags;
} jwb_world_t;

/**
 * ### `struct jwb_world_init`
 * ```
 * struct jwb_world_init {
 *   int flags;
 *   jwb_num_t cell_size;
 *   size_t width;
 *   size_t height;
 *   size_t ent_buf_size;
 *   size_t ent_extra;
 *   void *ent_buf;
 *   void *cell_buf;
 * };
 * ```
 *
 * Initialization information for use in `jwb_world_alloc`.
 *
 * #### Fields
 *  * `flags`: The flags which the world should have. Valid flags:
 *    - `JWBF_REMOVE_DISTANT`: Remove off-grid entities rather than wrapping.
 *  * `cell_size`: The size of cells in the world.
 *  * `width`: The width of the world in cells.
 *  * `height`: The height of the world in cells.
 *  * `ent_buf_size`: The number of entities to allocate initially. If an
 *    entity buffer is given, the buffer is assumed to have this much space.
 *  * `ent_extra`: Extra space to allocate for each entity.
 *  * `ent_buf`: The entity buffer. If this is `NULL`, a new one is allocated. A
 *    buffer of size `JWB_WORLD_ENT_BUF_SIZE(...)` must be provided if
 *    allocation is turned off.
 *  * `cell_buf`: The cell buffer. If this is `NULL`, a new one is allocated. A
 *    buffer of size `JWB_WORLD_CELL_BUF_SIZE(...)` must be provided if
 *    allocation is turned off.
 */
struct jwb_world_init {
	int flags;
	jwb_num_t cell_size;
	size_t width;
	size_t height;
	size_t ent_buf_size;
	size_t ent_extra;
	void *ent_buf;
	void *cell_buf;
};
#define JWBF_REMOVE_DISTANT (1 << 0)

/**
 * ### `JWB_WORLD_INIT_DEFAULT`
 * ```
 * #define JWB_WORLD_INIT_DEFAULT
 * ```
 *
 * The struct initializer containing the default values for world allocation.
 * You should use this to make sure you don't pass in any garbage data.
 */
#define JWB_WORLD_INIT_DEFAULT \
{	/* flags */        0, \
	/* cell_size */    1, \
	/* width */        1, \
	/* height */       1, \
	/* ent_buf_size */ 0, \
	/* ent_extra */    0, \
	/* ent_buf */   NULL, \
	/* cell_buf */  NULL}

/**
 * ### `jwb_world_alloc`
 * ```
 * int jwb_world_alloc(jwb_world_t *world, struct jwb_world_init *info);
 * ```
 *
 * Allocate the necessary resources for a given world.
 *
 * #### Parameters
 *  1. `world`: The world to initialize.
 *  2. `info`: The initialization info. See `struct jwb_world_init`.
 *
 * #### Return Value
 *  * `0`: Success.
 *  * `-JWBE_NO_MEMORY`: Buffer allocation failed.
 *  * `-JWBE_INVALID_ARGUMENT`: Invalid parameter (such as zero for width,
 *    height, or cell size)
 */
int jwb_world_alloc(jwb_world_t *world, struct jwb_world_init *info);

/**
 * ### `JWB_WORLD_ENT_BUF_SIZE`
 * ```
 * #define JWB_WORLD_ENT_BUF_SIZE(flags, num, extra_space) ...
 * ```
 *
 * Get the needed buffer size for a number of entities.
 *
 * #### Parameters
 *  1. `flags`: The flags applied to the relevant world.
 *  2. `num`: The number of entities to account for.
 *  3. `extra_space`: The amount of entity extra space in the relevant world.
 *
 * #### Return Value
 * The needed buffer size in bytes.
 */
#define JWB_WORLD_ENT_BUF_SIZE(flags, num, extra_space) ((void)(flags), \
	(num) * JWB__ENTITY_SIZE(extra_space))

/**
 * ### `JWB_WORLD_CELL_BUF_SIZE`
 * ```
 * #define JWB_WORLD_CELL_BUF_SIZE(flags, width, height) ...
 * ```
 *
 * Get the needed buffer size for a world's cell grid.
 *
 * #### Parameters
 *  1. `flags`: The flags applied to the relevant world.
 *  2. `width`: The number of cells across.
 *  3. `height`: The number of cells down.
 *
 * #### Return Value
 * The needed buffer size in bytes.
 */
#define JWB_WORLD_CELL_BUF_SIZE(flags, width, height) ((void)(flags), \
	((width) == 1 || (height) == 1 ? 4 : 1) \
	* (width) * (height) * sizeof(jwb_ehandle_t))

/**
 * ### `JWB_WORLD_DEFAULT_HIT_HANDLER`
 * This is defined as `jwb_elastic_collision`.
 */
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
 * ### `jwb_inelastic_collision`
 * ```
 * void jwb_inelastic_collision(
 *   jwb_world_t *world,
 *   jwb_ehandle_t ent1,
 *   jwb_ehandle_t ent2,
 *   struct jwb_hit_info *info);
 * ```
 *
 * Perform a collision between two circles where momentum is conserved, but not
 * energy. This is designed to be used as a hit handler. See the documentation
 * for `jwb_hit_handler_t`.
 */
void jwb_inelastic_collision(
	jwb_world_t *world,
	jwb_ehandle_t ent1,
	jwb_ehandle_t ent2,
	struct jwb_hit_info *info);

/**
 * ### `jwb_world_apply_friction`
 * ```
 * void jwb_world_apply_friction(jwb_world_t *world, jwb_num_t friction);
 * ```
 *
 * Apply an acceleration due to friction to every entity in the world.
 *
 * #### Parameters
 *  1. `world`: The world to go through.
 *  2. `friction`: The frictional acceleration to apply. This reduces velocity
 *     when positive.
 */
void jwb_world_apply_friction(jwb_world_t *world, jwb_num_t friction);

/**
 * ### `jwb_world_first`
 * ```
 * jwb_ehandle_t jwb_world_first(jwb_world_t *world);
 * ```
 *
 * Get the first placed entity for iteration.
 *
 * #### Parameters
 *  1. `world`: The iterated world.
 *
 * #### Return Value
 * The entity being checked out, or -1 if none exist.
 */
jwb_ehandle_t jwb_world_first(jwb_world_t *world);

/**
 * ### `jwb_world_next`
 * ```
 * jwb_ehandle_t jwb_world_next(jwb_world_t *world, jwb_ehandle_t now);
 * ```
 *
 * Get the next placed entity for iteration.
 *
 * #### Parameters
 *  1. `world`: The iterated world.
 *  2. `now`: The last examined entity when iterating.
 *
 * #### Return Value
 * The entity being checked out, or -1 if there are no more.
 */
jwb_ehandle_t jwb_world_next(jwb_world_t *world, jwb_ehandle_t now);

/**
 * ### `jwb_world_first_removed`
 * ```
 * jwb_ehandle_t jwb_world_first_removed(jwb_world_t *world);
 * ```
 *
 * Get the first removed entity for iteration.
 *
 * #### Parameters
 *  1. `world`: The iterated world.
 *
 * #### Return Value
 * The entity being checked out, or -1 if none exist.
 */
jwb_ehandle_t jwb_world_first_removed(jwb_world_t *world);

/**
 * ### `jwb_world_next_removed`
 * ```
 * jwb_ehandle_t jwb_world_next_removed(jwb_world_t *world, jwb_ehandle_t now);
 * ```
 *
 * Get the next placed entity for iteration.
 *
 * #### Parameters
 *  1. `world`: The iterated world.
 *  2. `now`: The last examined entity when iterating.
 *
 * #### Return Value
 * The entity being checked out, or -1 if there are no more.
 */
jwb_ehandle_t jwb_world_next_removed(jwb_world_t *world, jwb_ehandle_t now);

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
 *   jwb_num_t mass,
 *   jwb_num_t radius);
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
	jwb_num_t mass,
	jwb_num_t radius);

/**
 * ### `jwb_world_re_add_ent`
 * ```
 * int jwb_world_re_add_ent(jwb_world_t *world, jwb_ehandle_t ent);
 * ```
 *
 * Add an entity which was removed.
 *
 * #### Parameters
 *  1. `world`: The world to add the entity to.
 *  2. `ent`: Which entity to add.
 *
 * #### Return Value
 *  * `0`: The entity was added or existed already.
 *  * `-JWBE_DESTROYED_ENTITY`: The entity specified does not exist.
 */
int jwb_world_re_add_ent(jwb_world_t *world, jwb_ehandle_t ent);

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
 * where `-JWBE_ENTITY_DESTROYED` is returned.
 *
 * Most of these are pretty similar, so the descriptions are short.
 */

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
 * ### `jwb_world_extra_size`
 * ```
 * size_t jwb_world_extra_size(jwb_world_t *world);
 * ```
 *
 * Get the size of each entity's extra custom space. This will be at least the
 * amount provided at initialization.
 *
 * #### Parameters
 *  1. `world`: The world to examine.
 * #### Return Value
 * The size in bytes.
 */
size_t jwb_world_extra_size(jwb_world_t *world);

/**
 * ### `jwb_world_offset`
 * ```
 * int jwb_world_offset(jwb_world_t *world, const struct jwb_vect *off);
 * ```
 *
 * Set the offset of the world's grid.
 *
 * #### Parameters
 *  1. `world`: The world to change.
 *  2. `off`: The new offset.
 */
int jwb_world_offset(jwb_world_t *world, const struct jwb_vect *off);

/**
 * ### `jwb_world_get_offset`
 * ```
 * int jwb_world_get_offset(jwb_world_t *world, struct jwb_vect *dest);
 * ```
 *
 * Check the offset of the world's grid.
 *
 * #### Parameters
 *  1. `world`: The world to change.
 *  2. `dest`: The place to put the current offset.
 */
int jwb_world_get_offset(jwb_world_t *world, struct jwb_vect *dest);

/**
 * ### `jwb_world_track`
 * ```
 * int jwb_world_track(jwb_world_t *world, jwb_ehandle_t ent);
 * ```
 *
 * Track an entity. The tracked entity will stay at the same position relative
 * to the grid as long as it is tracked and it is not moved through `translate`
 * functions.
 *
 * #### Parameters
 *  1. `world`: The world to change.
 *  2. `ent`: The new tracked target. If this is negative, no entity is tracked;
 *     this is not an error.
 */
int jwb_world_track(jwb_world_t *world, jwb_ehandle_t ent);

/**
 * ### `jwb_world_tracking`
 * ```
 * ```
 *
 * Get the currently tracked entity.
 *
 * #### Parameters
 *  1. `world`: The world to look in.
 *
 * #### Return Value
 * The current tracking target. A negative value indicates the absence of one.
 */
jwb_ehandle_t jwb_world_tracking(jwb_world_t *world);

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
 * jwb_num_t jwb_world_get_mass(jwb_world_t *world, jwb_ehandle_t ent);
 * ```
 *
 * #### Parameters
 *  1. `world`: The world to look at.
 *  2. `ent`: The entity to look in.
 * #### Return Value
 * The mass of the entity, or a negative error code on nonexistence.
 */
jwb_num_t jwb_world_get_mass(jwb_world_t *world, jwb_ehandle_t ent);

/**
 * ### `jwb_world_get_radius`
 * ```
 * jwb_num_t jwb_world_get_radius(jwb_world_t *world, jwb_ehandle_t ent);
 * ```
 *
 * #### Parameters
 *  1. `world`: The world to look at.
 *  2. `ent`: The entity to look in.
 * #### Return Value
 * The radius of the entity, or a negative error code on nonexistence.
 */
jwb_num_t jwb_world_get_radius(jwb_world_t *world, jwb_ehandle_t ent);

/**
 * ### `jwb_world_set_mass`
 * ```
 * int jwb_world_set_mass(jwb_world_t *world, jwb_ehandle_t ent, jwb_num_t mass);
 * ```
 *
 * #### Parameters
 *  1. `world`: The world to change.
 *  2. `ent`: The entity to change.
 *  3. `mass`: What to set the mass to. Must be over zero.
 */
int jwb_world_set_mass(jwb_world_t *world, jwb_ehandle_t ent, jwb_num_t mass);

/**
 * ### `jwb_world_set_radius`
 * ```
 * int jwb_world_set_radius(
 *   jwb_world_t *world,
 *   jwb_ehandle_t ent,
 *   jwb_num_t radius);
 * ```
 *
 * #### Parameters
 *  1. `world`: The world to change.
 *  2. `ent`: The entity to change.
 *  3. `radius`: What to set the radius to. Must be over zero.
 */
int jwb_world_set_radius(jwb_world_t *world, jwb_ehandle_t ent, jwb_num_t radius);

/**
 * ### `jwb_world_get_mass_unck`
 * ```
 * jwb_num_t jwb_world_get_mass_unck(jwb_world_t *world, jwb_ehandle_t ent);
 * ```
 *
 * #### Parameters
 *  1. `world`: The world to look at.
 *  2. `ent`: The entity to look in.
 *
 * #### Return Value
 * The mass of the entity.
 */
jwb_num_t jwb_world_get_mass_unck(jwb_world_t *world, jwb_ehandle_t ent);

/**
 * ### `jwb_world_get_extra`
 * ```
 * void *jwb_world_get_extra(jwb_world_t *world, jwb_ehandle_t ent);
 * ```
 *
 * Get extra data for an entity. This is aligned to 8 bytes unless
 * `JWBO_EXTRA_ALIGN_4` is defined during compilation.
 *
 * #### Parameters
 *  1. `world`: The world to look in.
 *  2. `ent`: The entity to look at.
 *
 * #### Return Value
 * The extra data buffer with the capacity specified for `jwb_world_alloc`.
 * On failure, `NULL` is returned to indicate `JWBE_DESTROYED_ENTITY`.
 */
void *jwb_world_get_extra(jwb_world_t *world, jwb_ehandle_t ent);

/**
 * ### `jwb_world_get_extra_unck`
 * ```
 * void *jwb_world_get_extra_unck(jwb_world_t *world, jwb_ehandle_t ent);
 * ```
 *
 * Get extra data for an entity. This is aligned to 8 bytes unless
 * `JWBO_EXTRA_ALIGN_4` is defined during compilation.
 *
 * #### Parameters
 *  1. `world`: The world to look in.
 *  2. `ent`: The entity to look at.
 *
 * #### Return Value
 * The extra data buffer with the capacity specified for `jwb_world_alloc`.
 */
void *jwb_world_get_extra_unck(jwb_world_t *world, jwb_ehandle_t ent);

/**
 * ### `jwb_world_get_radius_unck`
 * ```
 * jwb_num_t jwb_world_get_radius_unck(jwb_world_t *world, jwb_ehandle_t ent);
 * ```
 *
 * #### Parameters
 *  1. `world`: The world to look at.
 *  2. `ent`: The entity to look in.
 * #### Return Value
 * The radius of the entity.
 */
jwb_num_t jwb_world_get_radius_unck(jwb_world_t *world, jwb_ehandle_t ent);

/**
 * ### `jwb_world_set_mass_unck`
 * ```
 * void jwb_world_set_mass_unck(
 *   jwb_world_t *world,
 *   jwb_ehandle_t ent,
 *   jwb_num_t mass);
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
	jwb_num_t mass);

/**
 * ### `jwb_world_set_radius_unck`
 * ```
 * void jwb_world_set_radius_unck(
 *   jwb_world_t *world,
 *   jwb_ehandle_t ent,
 *   jwb_num_t radius);
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
	jwb_num_t radius);

#ifdef JWB_INTERNAL_
/* Internal type name shortcuts, macros, and flags. */

typedef jwb_world_t WORLD;
typedef struct jwb_vect VECT;
typedef jwb_ehandle_t EHANDLE;

#	define GET(world, ent) (*(struct jwb__entity *)&((world)->ents[(ent) \
			* (world)->ent_size]))

#	ifdef JWBO_ALWAYS_REMOVE_DISTANT
#		define REMOVING_DISTANT(world) ((void)(world), 1)
#	elif defined(JWBO_NEVER_REMOVE_DISTANT)
#		define REMOVING_DISTANT(world) ((void)(world), 0)
#	else
#		define REMOVING_DISTANT(world) \
			((world)->flags & JWBF_REMOVE_DISTANT)
#	endif

#	ifdef JWBO_NO_ALLOC
#		define ALLOC(size) ((void)(size), NULL)
#		define FREE(ptr) ((void)(ptr))
#	else
#		define ALLOC(size) malloc((size))
#		define FREE(ptr) free((ptr))
#	endif /* JWBO_NO_ALLOC */

/* Private flags for WORLD::flags */
#	define ONE_CELL_THICK (1 << 1)
#	define PROVIDED_ENT_BUF (1 << 2)

/* Private flags for jwb__entity::flags */
#	define REMOVED (1 << 0)
#	define MOVED_THIS_STEP (1 << 1)
#	define DESTROYED (1 << 2)

#endif /* JWB_INTERNAL_ */

#endif /* Header guard */
