#define JWB_INTERNAL_
#include <jwb.h>
#include <math.h>

void jwb_rotation(jwb_rotation_t *rot, jwb_num_t angle)
{
	rot->sin = sin(angle);
	rot->cos = cos(angle);
}

jwb_num_t jwb_rotation_angle(const jwb_rotation_t *rot)
{
	return atan2(rot->sin, rot->cos);
}

void jwb_rotation_flip(jwb_rotation_t *rot)
{
	/* sin(-θ) = -sin(θ), cos(-θ) = cos(θ) */
	rot->sin *= -1;
}

void jwb_vect_rotate(struct jwb_vect *vect, const jwb_rotation_t *rot)
{
	jwb_num_t x = vect->x;
	vect->x = x * rot->cos - vect->y * rot->sin;
	vect->y = x * rot->sin + vect->y * rot->cos;
}

jwb_num_t jwb_vect_magnitude(const struct jwb_vect *vect)
{
	return sqrt(vect->x * vect->x + vect->y * vect->y);
}

void jwb_vect_normalize(struct jwb_vect *vect)
{
	jwb_num_t mag = jwb_vect_magnitude(vect);
	vect->x /= mag;
	vect->y /= mag;
}

jwb_num_t jwb_vect_angle(const struct jwb_vect *vect)
{
	return atan2(vect->y, vect->x);
}

void jwb_vect_rotation(const struct jwb_vect *vect, jwb_rotation_t *rot)
{
	jwb_num_t mag = jwb_vect_magnitude(vect);
	rot->sin = vect->y / mag;
	rot->cos = vect->x / mag;
}
