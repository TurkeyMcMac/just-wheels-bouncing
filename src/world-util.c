#define JWB_INTERNAL_
#include <jwb.h>

void jwb_elastic_collision(
	WORLD *world,
	EHANDLE ent1,
	EHANDLE ent2,
	struct jwb_hit_info *info)
{
	double mass1, mass2;
	VECT vel1, vel2;
	double bounced1, bounced2;
	double overlap;
	double cor1, cor2;
	jwb_rotation_t rot;
	if (info->dist == 0.) {
		return;
	}
	mass1 = GET(world, ent1).mass;
	mass2 = GET(world, ent2).mass;
	vel1 = GET(world, ent1).vel;
	vel2 = GET(world, ent2).vel;
	jwb_vect_rotation(&info->rel, &rot); /* FIXME:Recalculates magnitude. */
	jwb_rotation_flip(&rot);
	jwb_vect_rotate(&vel1, &rot);
	jwb_vect_rotate(&vel2, &rot);
	bounced1 = (mass1 - mass2) / (mass1 + mass2) * vel1.x + 2 * mass2
		/ (mass1 + mass2) * vel2.x;
	bounced2 = (mass2 - mass1) / (mass2 + mass1) * vel2.x + 2 * mass1
		/ (mass2 + mass1) * vel1.x;
	vel1.x = bounced1;
	vel2.x = bounced2;
	overlap = 1. - info->dist
		/ (GET(world, ent1).radius + GET(world, ent2).radius);
	cor1 = -overlap / (mass1 / mass2 + 1.);
	cor2 = cor1 + overlap;
	jwb_rotation_flip(&rot);
	jwb_vect_rotate(&vel1, &rot);
	jwb_vect_rotate(&vel2, &rot);
	GET(world, ent1).vel = vel1;
	GET(world, ent2).vel = vel2;
	GET(world, ent1).correct.x += info->rel.x * cor1;
	GET(world, ent1).correct.y += info->rel.y * cor1;
	GET(world, ent2).correct.x += info->rel.x * cor2;
	GET(world, ent2).correct.y += info->rel.y * cor2;
}

void jwb_inelastic_collision(
	WORLD *world,
	EHANDLE ent1,
	EHANDLE ent2,
	struct jwb_hit_info *info)
{
	double mass1, mass2;
	VECT vel1, vel2;
	double smashed;
	double overlap;
	double cor1, cor2;
	jwb_rotation_t rot;
	if (info->dist == 0.) {
		return;
	}
	mass1 = GET(world, ent1).mass;
	mass2 = GET(world, ent2).mass;
	vel1 = GET(world, ent1).vel;
	vel2 = GET(world, ent2).vel;
	jwb_vect_rotation(&info->rel, &rot); /* FIXME:Recalculates magnitude. */
	jwb_rotation_flip(&rot);
	jwb_vect_rotate(&vel1, &rot);
	jwb_vect_rotate(&vel2, &rot);
	smashed = (mass1 * vel1.x + mass2 * vel2.x) / (mass1 + mass2);
	vel1.x = smashed;
	vel2.x = smashed;
	overlap = 1. - info->dist
		/ (GET(world, ent1).radius + GET(world, ent2).radius);
	cor1 = -overlap / (mass1 / mass2 + 1.);
	cor2 = cor1 + overlap;
	jwb_rotation_flip(&rot);
	jwb_vect_rotate(&vel1, &rot);
	jwb_vect_rotate(&vel2, &rot);
	GET(world, ent1).vel = vel1;
	GET(world, ent2).vel = vel2;
	GET(world, ent1).correct.x += info->rel.x * cor1;
	GET(world, ent1).correct.y += info->rel.y * cor1;
	GET(world, ent2).correct.x += info->rel.x * cor2;
	GET(world, ent2).correct.y += info->rel.y * cor2;
}

static int apply_friction(WORLD *world, EHANDLE ent, double friction)
{
	struct jwb_vect *vel;
	double speed, ratio;
	vel = &GET(world, ent).vel;
	speed = jwb_vect_magnitude(vel);
	ratio = (speed - friction) / speed;
	if (ratio > 0.) {
		vel->x *= ratio;
		vel->y *= ratio;
	} else {
		vel->x = 0.;
		vel->y = 0.;
	}
	return 0;
}

void jwb_world_apply_friction(WORLD *world, double friction)
{
	EHANDLE e;
	for (e = jwb_world_first(world); e >= 0; e = jwb_world_next(world, e)) {
		apply_friction(world, e, friction);
	}
}
