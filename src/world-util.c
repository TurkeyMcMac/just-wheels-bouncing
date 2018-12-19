#define JWB_INTERNAL_
#include <jwb.h>

void jwb_elastic_collision(WORLD *world, EHANDLE ent1, EHANDLE ent2)
{
	double rad1, rad2, distance;
	double mass1, mass2;
	VECT pos1, pos2, relative;
	VECT vel1, vel2;
	double bounced1, bounced2;
	double overlap;
	double cor1, cor2;
	jwb_rotation_t rot;
	pos1 = world->ents[ent1].pos;
	pos2 = world->ents[ent2].pos;
	relative.x = pos2.x - pos1.x;
	relative.y = pos2.y - pos1.y;
	rad1 = world->ents[ent1].radius;
	rad2 = world->ents[ent2].radius;
	distance = jwb_vect_magnitude(&relative);
	if (rad1 + rad2 < distance || distance == 0.) {
		return;
	}
	mass1 = world->ents[ent1].mass;
	mass2 = world->ents[ent2].mass;
	vel1 = world->ents[ent1].vel;
	vel2 = world->ents[ent2].vel;
	jwb_vect_rotation(&relative, &rot); /* FIXME: Recalculates magnitude. */
	jwb_rotation_flip(&rot);
	jwb_vect_rotate(&vel1, &rot);
	jwb_vect_rotate(&vel2, &rot);
	bounced1 = (mass1 - mass2) / (mass1 + mass2) * vel1.x + 2 * mass2
		/ (mass1 + mass2) * vel2.x;
	bounced2 = (mass2 - mass1) / (mass2 + mass1) * vel2.x + 2 * mass1
		/ (mass2 + mass1) * vel1.x;
	vel1.x = bounced1;
	vel2.x = bounced2;
	overlap = 1. - distance / (rad1 + rad2);
	cor1 = -overlap / (mass1 / mass2 + 1.);
	cor2 = cor1 + overlap;
	jwb_rotation_flip(&rot);
	jwb_vect_rotate(&vel1, &rot);
	jwb_vect_rotate(&vel2, &rot);
	world->ents[ent1].vel = vel1;
	world->ents[ent2].vel = vel2;
	world->ents[ent1].correct.x += relative.x * cor1;
	world->ents[ent1].correct.y += relative.y * cor1;
	world->ents[ent2].correct.x += relative.x * cor2;
	world->ents[ent2].correct.y += relative.y * cor2;
}

static int apply_friction(WORLD *world, EHANDLE ent, void *fricp)
{
	struct jwb_vect *vel;
	double speed, friction, ratio;
	vel = &world->ents[ent].vel;
	speed = jwb_vect_magnitude(vel);
	friction = *(double *)fricp;
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
	jwb_world_for_each(world, apply_friction, &friction);
}
