#define JWB_INTERNAL_
#include <jwb.h>
#include <math.h>

void jwb_rotation(jwb_rotation_t *rot, double angle)
{
	rot->sin = sin(angle);
	rot->cos = cos(angle);
}

double jwb_rotation_angle(const jwb_rotation_t *rot)
{
	return asin(rot->sin);
}

void jwb_rotation_flip(jwb_rotation_t *rot)
{
	/* sin(-θ) = -sin(θ), cos(-θ) = cos(θ) */
	rot->sin *= -1;
}

void jwb_vect_rotate(struct jwb_vect *vect, const jwb_rotation_t *rot)
{
	double x = vect->x;
	vect->x = x * rot->cos - vect->y * rot->sin;
	vect->y = x * rot->sin + vect->y * rot->cos;
}

double jwb_vect_magnitude(const struct jwb_vect *vect)
{
	return sqrt(vect->x * vect->x + vect->y * vect->y);
}

void jwb_vect_normalize(struct jwb_vect *vect)
{
	double mag = jwb_vect_magnitude(vect);
	vect->x /= mag;
	vect->y /= mag;
}

double jwb_vect_angle(const struct jwb_vect *vect)
{
	return atan(vect->y / vect->x);
}

void jwb_vect_rotation(const struct jwb_vect *vect, jwb_rotation_t *rot)
{
	double mag = jwb_vect_magnitude(vect);
	rot->sin = vect->y / mag;
	rot->cos = vect->x / mag;
}
