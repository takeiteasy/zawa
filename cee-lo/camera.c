//
//  camera.c
//  cee-lo
//
//  Created by George Watson on 26/06/2017.
//  Copyright © 2017 George Watson. All rights reserved.
//

#include "camera.h"

void camera_init_def(camera_t* cam) {
	cam->pos   = vec3_new(.0f, .0f, .0f);
	cam->front = vec3_new(0.f, 0.f, -1.f);
	cam->up    = vec3_new(0.f, 1.f,  0.f);
	cam->world = vec3_new(0.f, 1.f, 0.f);

	cam->yaw         = -90.f;
	cam->pitch       = 0.f;
	cam->sensitivity = .25f;
	cam->move_speed  = 0.05f;
	cam->zoom        = 45.f;
}

void camera_init(camera_t* cam, vec3 a, vec3 b, float y, float p) {
	cam->pos   = a;
	cam->world = vec3_new(0.f, 1.f, 0.f);
	cam->up    = b;
	cam->front = vec3_new(0.f, 0.f, -1.f);

	cam->yaw         = y;
	cam->pitch       = p;
	cam->sensitivity = .25f;
	cam->move_speed  = 0.05f;
	cam->zoom        = 45.f;
}

void camera_update(camera_t* cam) {
	cam->front = vec3_normalize(vec3_new(cosf(radians(cam->yaw)) * cosf(radians(cam->pitch)),
				sinf(radians(cam->pitch)),
				sinf(radians(cam->yaw)) * cosf(radians(cam->pitch))));
	cam->right = vec3_normalize(vec3_cross(cam->front, cam->world));
	cam->up    = vec3_normalize(vec3_cross(cam->right, cam->front));

	cam->view  = mat4_view_look_at(cam->pos, vec3_add(cam->pos, cam->front), cam->up);
}

void camera_move(camera_t* cam, const enum VIEW_DIRECTION d) {
	switch (d) {
		case UP:
			cam->pos = vec3_add(cam->pos, vec3_mul(cam->up, cam->move_speed));
			break;
		case DOWN:
			cam->pos = vec3_sub(cam->pos, vec3_mul(cam->up, cam->move_speed));
			break;
		case RIGHT:
			cam->pos = vec3_add(cam->pos, vec3_mul(vec3_normalize(vec3_cross(cam->front, cam->up)), cam->move_speed));
			break;
		case LEFT:
			cam->pos = vec3_sub(cam->pos, vec3_mul(vec3_normalize(vec3_cross(cam->front, cam->up)), cam->move_speed));
			break;
		case FORWARD:
			cam->pos = vec3_add(cam->pos, vec3_mul(cam->front, cam->move_speed));
			break;
		case BACK:
			cam->pos = vec3_sub(cam->pos, vec3_mul(cam->front, cam->move_speed));
			break;
	}
}

void camera_look(camera_t* cam, int x, int y) {
	cam->yaw  += (float)x * cam->sensitivity;
	cam->pitch = CLAMP(cam->pitch + ((float)y * cam->sensitivity), -89.f, 89.f);
}
