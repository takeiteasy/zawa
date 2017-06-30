#ifndef __OBJ_Plane__H__
#define __OBJ_Plane__H__

static float Plane_vertices[] = {
	6.332604, 0.000000, 6.332604,
	-6.332604, 0.000000, -6.332604,
	-6.332604, 0.000000, 6.332604,
	6.332604, 0.000000, 6.332604,
	6.332604, 0.000000, -6.332604,
	-6.332604, 0.000000, -6.332604,
};

static float Plane_normals[] = {
	0.000000, 1.000000, 0.000000,
	0.000000, 1.000000, 0.000000,
	0.000000, 1.000000, 0.000000,
	0.000000, 1.000000, 0.000000,
	0.000000, 1.000000, 0.000000,
	0.000000, 1.000000, 0.000000,
};

static float Plane_texcoords[] = {
	0.999900, 0.000100,
	0.000100, 0.999900,
	0.000100, 0.000100,
	0.999900, 0.000100,
	0.999900, 0.999900,
	0.000100, 0.999900,
};

static const size_t Plane_num_vertices  = 6;
static const int    Plane_has_normals   = 1;
static const int    Plane_num_texcoords = 1;

#endif // __OBJ_Plane__H__