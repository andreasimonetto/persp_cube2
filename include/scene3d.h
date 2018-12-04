#ifndef _SCENE3D_H_
#define _SCENE3D_H_
#include <X11/Xlib.h>
#include "mesh3d.h"
#include "vtypes.h"


typedef struct {
	vector3_t vu; /* view-up vector */
	vector3_t vp; /* view-point coordinates (spherics) */
	angle_t alpha;
	float front_plane_z, back_plane_z;
	/*float d;*/
	
	/**/
	
	struct object_node {
		mesh3d_t *mesh;
		char *name;
		struct object_node *next;
	} *objects;
	
	Display *dpy;
	XImage *ximage;
	unsigned *viewport;
	int vp_width, vp_height;
	int *tam_top, *tam_bottom;
	float **zbuffer;
} scene3d_t;

scene3d_t* scene3d_create(Display *dpy, unsigned width, unsigned height, unsigned depth);
void scene3d_delete(scene3d_t *scene);
void scene3d_render(scene3d_t *scene);
#define scene3d_clear(s) scene3d_fill_rect(s, 0, 0, (s)->vp_width, (s)->vp_height, 0xFFFFFF)
void scene3d_fill_rect(scene3d_t *scene, unsigned x0, unsigned y0, unsigned width, unsigned height, unsigned col);

void scene3d_object_add(scene3d_t *scene, const char *obj_name, mesh3d_t *obj_mesh);
mesh3d_t* scene3d_object_delete(scene3d_t *scene, const char *obj_name);
mesh3d_t* mesh3d_view_param_create(const scene3d_t *scene);

#endif
