#define JWB_INTERNAL_
#include <jwb.h>

int jwb_get_hit_axis(struct jwb_hit_info *info, jwb_rotation_t *rot)
{
	if (info->dist == 0.) {
		return 0;
	}
	jwb_vect_rotation(&info->rel, rot); /* FIXME:Recalculates magnitude. */
	jwb_rotation_flip(rot);
	return 1;
}

void jwb_no_overlap(
	WORLD *world,
	struct jwb_hit_info *info,
	EHANDLE ent1,
	EHANDLE ent2)
{
	jwb_num_t overlap, cor1, cor2;
	struct jwb__entity *info1, *info2;
	info1 = &GET(world, ent1);
	info2 = &GET(world, ent2);
	overlap = 1. - info->dist / (info1->radius + info2->radius);
	cor1 = -overlap / (info1->mass / info2->mass + 1.);
	cor2 = cor1 + overlap;
	info1->correct.x += info->rel.x * cor1;
	info1->correct.y += info->rel.y * cor1;
	info2->correct.x += info->rel.x * cor2;
	info2->correct.y += info->rel.y * cor2;
}

void jwb_elastic_collision(
	WORLD *world,
	EHANDLE ent1,
	EHANDLE ent2,
	struct jwb_hit_info *info)
{
	jwb_num_t mass1, mass2;
	VECT vel1, vel2;
	jwb_num_t bounced1, bounced2;
	jwb_rotation_t rot;
	if (!jwb_get_hit_axis(info, &rot)) {
		return;
	}
	mass1 = GET(world, ent1).mass;
	mass2 = GET(world, ent2).mass;
	vel1 = GET(world, ent1).vel;
	vel2 = GET(world, ent2).vel;
	jwb_vect_rotate(&vel1, &rot);
	jwb_vect_rotate(&vel2, &rot);
	bounced1 = (mass1 - mass2) / (mass1 + mass2) * vel1.x + 2 * mass2
		/ (mass1 + mass2) * vel2.x;
	bounced2 = vel1.x - vel2.x + bounced1;
	vel1.x = bounced1;
	vel2.x = bounced2;
	jwb_rotation_flip(&rot);
	jwb_vect_rotate(&vel1, &rot);
	jwb_vect_rotate(&vel2, &rot);
	GET(world, ent1).vel = vel1;
	GET(world, ent2).vel = vel2;
	jwb_no_overlap(world, info, ent1, ent2);
}

void jwb_inelastic_collision(
	WORLD *world,
	EHANDLE ent1,
	EHANDLE ent2,
	struct jwb_hit_info *info)
{
	jwb_num_t mass1, mass2;
	VECT vel1, vel2;
	jwb_num_t smashed;
	jwb_rotation_t rot;
	if (!jwb_get_hit_axis(info, &rot)) {
		return;
	}
	mass1 = GET(world, ent1).mass;
	mass2 = GET(world, ent2).mass;
	vel1 = GET(world, ent1).vel;
	vel2 = GET(world, ent2).vel;
	jwb_vect_rotate(&vel1, &rot);
	jwb_vect_rotate(&vel2, &rot);
	smashed = (mass1 * vel1.x + mass2 * vel2.x) / (mass1 + mass2);
	vel1.x = smashed;
	vel2.x = smashed;
	jwb_rotation_flip(&rot);
	jwb_vect_rotate(&vel1, &rot);
	jwb_vect_rotate(&vel2, &rot);
	GET(world, ent1).vel = vel1;
	GET(world, ent2).vel = vel2;
	jwb_no_overlap(world, info, ent1, ent2);
}

static int apply_friction(WORLD *world, EHANDLE ent, jwb_num_t friction)
{
	struct jwb_vect *vel;
	jwb_num_t speed, ratio;
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

void jwb_world_apply_friction(WORLD *world, jwb_num_t friction)
{
	EHANDLE e;
	for (e = jwb_world_first(world); e >= 0; e = jwb_world_next(world, e)) {
		apply_friction(world, e, friction);
	}
}
