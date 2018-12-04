/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "scene3d.h"

#include <math.h>
#ifndef INFINITY
#define INFINITY (__builtin_inff ())
#endif

#define sgn(n) ((n) > 0 ? 1 : ((n) < 0 ? -1 : 0))

static void scene3d_render_mesh(scene3d_t *scene, const mesh3d_t *mesh, float *xe, float *ye, float *ze, float d_s, float alpha, float beta);
static void scene3d_object_free(scene3d_t *scene);
static void create_transform_vectors(const scene3d_t *rb, float *xe, float *ye, float *ze);
static void tam_line(scene3d_t *scene, int x0, int y0, int x1, int y1);

int clip_line(float *x1, float *y1, float *x2, float *y2,
	float wxmin, float wymin, float wxmax, float wymax);

int clip_line2(int *x1, int *y1, int *x2, int *y2,
	int wxmin, int wymin, int wxmax, int wymax);

#define __line_bresenham(x, y, d1, d2, s1, s2, X, Y, M) { \
	int i, d, inc1, inc2; \
	\
	inc1 = 2 * d1; \
	d = inc1 - d2; \
	inc2 = d - d2; \
	\
	M; \
	for(i = 1; i <= d2; i++) { \
		Y += s2; \
		if(d < 0) \
			d += inc1; \
		else { \
			d += inc2; \
			X += s1; \
		} \
		\
		M; \
	} \
}

#define write_pixel_z(s, x, y, z, col, dz) { \
	if(x >= 0 && x < scene->vp_width && y >= 0 && y < scene->vp_height && z <= (s)->zbuffer[x][y]) { \
		(s)->zbuffer[x][y] = z; \
		(s)->viewport[(x) + (y) * (s)->vp_width] = (col); \
	} \
	z += dz; \
}

#define tam_check(scene, x, y) \
	if(x >= 0 && x < scene->vp_width) { \
		if(scene->tam_top[x] < y) \
			scene->tam_top[x] = y; \
		if(scene->tam_bottom[x] > y) \
			scene->tam_bottom[x] = y; \
	}

void tam_line(scene3d_t *scene, int x0, int y0, int x1, int y1)
{
int dx, dy, temp, sx, sy; 
	
	if(y0 < y1) { 
		temp = x0; 
		x0 = x1; 
		x1 = temp; 
		temp = y0; 
		y0 = y1; 
		y1 = temp; 
	} 
	
	dx = x1 - x0; 
	sx = sgn(dx); 
	dx = abs(dx); 
	sy = -1; 
	dy = y0 - y1; 
	
	if(dy <= dx) 
		__line_bresenham(x0, y0, dy, dx, sy, sx, y0, x0, tam_check(scene, x0, y0))
	else 
		__line_bresenham(x0, y0, dx, dy, sx, sy, x0, y0, tam_check(scene, x0, y0)) 
}

void draw_line_z(scene3d_t *scene, float *v1, float *v2, unsigned col)
{
int dx, dy, temp, sx, sy; 
int x0 = (int)((1.0 + v1[0]) / 2.0 * scene->vp_width);
int y0 = (int)((1.0 - v1[1]) / 2.0 * scene->vp_height);
int x1 = (int)((1.0 + v2[0]) / 2.0 * scene->vp_width);
int y1 = (int)((1.0 - v2[1]) / 2.0 * scene->vp_height);
float z0 = v1[2], dz = v2[2] - v1[2];
		
	if(y0 < y1) { 
		temp = x0; 
		x0 = x1; 
		x1 = temp; 
		temp = y0; 
		y0 = y1; 
		y1 = temp;
		dz *= -1.0;
		z0 = v2[2];
	} 
	
	dx = x1 - x0; 
	sx = sgn(dx); 
	dx = abs(dx); 
	sy = -1; 
	dy = y0 - y1; 
	
	if(dy <= dx) {
		dz /= dx;
		__line_bresenham(x0, y0, dy, dx, sy, sx, y0, x0, write_pixel_z(scene, x0, y0, z0, col, dz)) 
	}
	else {
		dz /= dy;
		__line_bresenham(x0, y0, dx, dy, sx, sy, x0, y0, write_pixel_z(scene, x0, y0, z0, col, dz))
	}
}

static void create_transform_vectors(const scene3d_t *rb, float *xe, float *ye, float *ze)
{ 
vector3_t vu;
float c;

	/* View-up vector: spheric->carthesian coordinates */ 
	vu[0] = rb->vu[0] * sinf(rb->vu[2]) * cosf(rb->vu[1]); 
	vu[1] = rb->vu[0] * sinf(rb->vu[2]) * sinf(rb->vu[1]); 
	vu[2] = rb->vu[0] * cosf(rb->vu[2]); 
	
	/* ze = -vp / ||vp|| */ 
	ze[0] = -sinf(rb->vp[2]) * cosf(rb->vp[1]); 
	ze[1] = -sinf(rb->vp[2]) * sinf(rb->vp[1]); 
	ze[2] = -cosf(rb->vp[2]); 
	
	/* xe = ze*vu / ||ze*vu|| */ 
	vector3_vprod(xe, ze, vu); 
	c = 1.0 / vector3_norm(xe);
	vector3_mult(xe, c, xe);
	
	/* ye = -ze*xe / ||ze*xe|| */
	vector3_vprod(ye, ze, xe);
	c = -1.0 / vector3_norm(ye);
	vector3_mult(ye, c, ye);
} 

void scene3d_render(scene3d_t *scene)
{
vector3_t xe, ye, ze;
const float d_s = 1.0 / tanf(scene->alpha / 2.0);
const float alpha = scene->front_plane_z / (scene->back_plane_z - scene->front_plane_z);
const float beta = -scene->back_plane_z * scene->front_plane_z / (scene->back_plane_z - scene->front_plane_z);
struct object_node *it;
int x, y;
	
	for(x = 0; x < scene->vp_width; x++) {
		for(y = 0; y < scene->vp_height; y++) {
			scene->viewport[x + y * scene->vp_width] = 0;
			scene->zbuffer[x][y] = INFINITY;
		}
	}
	
	create_transform_vectors(scene, xe, ye, ze);
	for(it = scene->objects; it; it = it->next) 
		scene3d_render_mesh(scene, it->mesh, xe, ye, ze, d_s, alpha, beta);
}
/*
void scene3d_render_view_params(scene3d_t *master, scene3d_t *slave)
{
vector3_t xe, ye, ze;
const float d_s = 1.0 / tanf(scene->alpha / 2.0);
const float alpha = scene->front_plane_z / (scene->back_plane_z - scene->front_plane_z);
const float beta = -scene->back_plane_z * scene->front_plane_z / (scene->back_plane_z - scene->front_plane_z);
struct object_node *it;
mesh3d_t *mesh = 
	
	create_transform_vectors(scene, xe, ye, ze);
	scene3d_render_mesh(scene, mesh, xe, ye, ze, d_s, alpha, beta);
}*/

static void scene3d_render_mesh(scene3d_t *scene, const mesh3d_t *mesh, float *xe, float *ye, float *ze, float d_s, float alpha, float beta)
{
float a, b, z, light_fact;
int i, x, y, xmin, xmax, col;
vector3_t *vp_verts = (vector3_t*) secure_alloc(mesh->vertex_num * sizeof(vector3_t));
vector3_t lfs;

	vector3_assign(lfs, ze);
	/*
	lfs[0] = sinf(M_PI / 2.0) * cosf(M_PI / 2.0); 
	lfs[1] = sinf(M_PI / 2.0) * sinf(M_PI / 2.0); 
	lfs[2] = cosf(M_PI / 2.0); */
	
	/*
	a = 1.0 / vector3_norm(lfs);
	vector3_mult(lfs, a, lfs);*/
	
	/* Transformation 1: from "object frame" to "observer view-point frame" */
	for(i = 0; i < mesh->vertex_num; i++) {
		vp_verts[i][0] = vector3_sprod(mesh->vertex_data[i].pos, xe) * d_s;
		vp_verts[i][1] = vector3_sprod(mesh->vertex_data[i].pos, ye) * d_s;
		vp_verts[i][2] = vector3_sprod(mesh->vertex_data[i].pos, ze) + scene->vp[0];
	}
	
	/* TODO: Frustum culling (part1) with: z=front_plane_z and z=back_plane_z */
	/* ... */
	
	/* Transformation 2: from "observer view-point frame" to "projection plane frame" */
	for(i = 0; i< mesh->vertex_num; i++) {
		vp_verts[i][0] /= vp_verts[i][2];
		vp_verts[i][1] /= vp_verts[i][2];
		vp_verts[i][2] = alpha + beta / vp_verts[i][2];
	}
	
	/* TODO: Frustum culling (part2) with: x=-1 ; y=-1 and x=1 ; y=1 */
	/* ... */
	
	/* Transformation 3: from "projection plane frame" to viewport */
	xmin = 0;
	xmax = scene->vp_width - 1;
	for(i = 0; i < mesh->face_num; i++)
	{
	float *v1 = vp_verts[mesh->face_data[i].verts[0]];
	float *v2 = vp_verts[mesh->face_data[i].verts[1]];
	float *v3 = vp_verts[mesh->face_data[i].verts[2]];
	int x1 = (int)floorf((1.0 + v1[0]) / 2.0 * scene->vp_width);
	int y1 = (int)floorf((1.0 - v1[1]) / 2.0 * scene->vp_height);
	int x2 = (int)floorf((1.0 + v2[0]) / 2.0 * scene->vp_width);
	int y2 = (int)floorf((1.0 - v2[1]) / 2.0 * scene->vp_height);
	int x3 = (int)floorf((1.0 + v3[0]) / 2.0 * scene->vp_width);
	int y3 = (int)floorf((1.0 - v3[1]) / 2.0 * scene->vp_height);
	int A = x1 - x3, B = x2 - x3, C = y2 - y3, D = y1 - y3;
	int ox3 = x3, oy3 = y3;
	float k1 = A * C - B * D;
	float k2 = B * D - A * C;
		
		for(x = xmin; x <= xmax; x++) {
			scene->tam_top[x] = INT_MIN;
			scene->tam_bottom[x] = INT_MAX;
		}
				
		if(x1 < x2) {
			xmin = x1;
			xmax = x2;
		}
		else {
			xmin = x2;
			xmax = x1;			
		}
		if(x3 < xmin)
			xmin = x3;
		else if(x3 > xmax)
			xmax = x3;
		
		if(xmin < 0)
			xmin = 0;
		if(xmax >= scene->vp_width)
			xmax = scene->vp_width - 1;
		
		tam_line(scene, x1, y1, x2, y2);
		tam_line(scene, x2, y2, x3, y3);
		tam_line(scene, x3, y3, x1, y1);

		light_fact = (vector3_sprod(mesh->face_data[i].normal, lfs) + 1.0) / 4.0;
		col = ((int)(light_fact * 255)) | ((int)(light_fact * 255) << 8) | ((int)(light_fact * 255) << 16);
		
		for(x = xmin; x <= xmax; x++) {
			for(y = scene->tam_bottom[x]; y <= scene->tam_top[x]; y++) {
				a = (B * (oy3 - y) - (ox3 - x) * C) / k1;
				b = (A * (oy3 - y) - (ox3 - x) * D) / k2;
				z = a * v1[2] + b * v2[2] + (1.0 - a - b) * v3[2];
				if(y >= 0 && y < scene->vp_height && z <= scene->zbuffer[x][y]) {
					scene->zbuffer[x][y] = z;
					scene->viewport[x + y * scene->vp_width] = col;
				}
			}
		}
	}

	for(i = 0; i < mesh->edge_num; i++) {
		if(mesh->edge_data[i].r || mesh->edge_data[i].g || mesh->edge_data[i].b) {
			col = ((int)(mesh->edge_data[i].b * 255)) | ((int)(mesh->edge_data[i].g * 255) << 8) | ((int)(mesh->edge_data[i].r * 255) << 16);
			draw_line_z(scene, vp_verts[mesh->edge_data[i].v1], vp_verts[mesh->edge_data[i].v2], col);
		}
	}
	
	free(vp_verts);
}

mesh3d_t* mesh3d_view_param_create(const scene3d_t *scene)
{ 
mesh3d_t *view_param = mesh3d_create(18, 0, 21); /* Frustum: 8 vertex, 12 edges */
vector3_t t;
matrix33_t inv;
int i;
	
	/* Setup vertex */
	vector3_set(view_param->vertex_data[0].pos, tanf(scene->alpha / 2.0) * scene->front_plane_z, tanf(scene->alpha / 2.0) * scene->front_plane_z, scene->front_plane_z);
	vector3_set(view_param->vertex_data[4].pos, tanf(scene->alpha / 2.0) * scene->back_plane_z, tanf(scene->alpha / 2.0) * scene->back_plane_z, scene->back_plane_z);
	for(i = 1; i < 4; i++) {
		vector3_assign(view_param->vertex_data[i].pos, view_param->vertex_data[0].pos);
		vector3_assign(view_param->vertex_data[i + 4].pos, view_param->vertex_data[4].pos);
	}
	view_param->vertex_data[1].pos[1] *= -1.0;
	view_param->vertex_data[2].pos[0] *= -1.0;
	view_param->vertex_data[2].pos[1] *= -1.0;
	view_param->vertex_data[3].pos[0] *= -1.0;
	view_param->vertex_data[5].pos[1] *= -1.0;
	view_param->vertex_data[6].pos[0] *= -1.0;
	view_param->vertex_data[6].pos[1] *= -1.0;
	view_param->vertex_data[7].pos[0] *= -1.0;
	
	/**/
	vector3_set(view_param->vertex_data[8].pos, 0, 0, 0);
	vector3_set(view_param->vertex_data[9].pos, 0, 1.0, 0);
	vector3_set(view_param->vertex_data[10].pos, 0.3, 0.7, 0);
	vector3_set(view_param->vertex_data[11].pos, -0.3, 0.7, 0);
	vector3_set(view_param->vertex_data[12].pos, 1.0, 0, 0);
	vector3_set(view_param->vertex_data[13].pos, 0.7, 0, 0.3);
	vector3_set(view_param->vertex_data[14].pos, 0.7, 0, -0.3);
	vector3_set(view_param->vertex_data[15].pos, 0, 0, 1.0);
	vector3_set(view_param->vertex_data[16].pos, 0.3, 0, 0.7);
	vector3_set(view_param->vertex_data[17].pos, -0.3, 0, 0.7);
	
	/* Setup edges */
	for(i = 0; i < 4; i++) {
		view_param->edge_data[i].v1 = i;
		view_param->edge_data[i].v2 = (i + 1) % 4;
		
		view_param->edge_data[4 + i].v1 = 4 + i;
		view_param->edge_data[4 + i].v2 = 4 + (i + 1) % 4;
		
		view_param->edge_data[8 + i].v1 = i;
		view_param->edge_data[8 + i].v2 = i + 4;
		
		view_param->edge_data[i].r = 
			view_param->edge_data[4 + i].r = 
				view_param->edge_data[8 + i].r = 0.0;
		view_param->edge_data[i].g = 
			view_param->edge_data[4 + i].g = 
				view_param->edge_data[8 + i].g = 0.0;
		view_param->edge_data[i].b = 
			view_param->edge_data[4 + i].b = 
				view_param->edge_data[8 + i].b = 1.0;
	}
	
	/**/
	for(i = 0; i < 3; i++) {
		view_param->edge_data[12 + i * 3].v1 = 8;
		view_param->edge_data[12 + i * 3].v2 = 9 + i * 3;
		view_param->edge_data[12 + i * 3].r = view_param->edge_data[12 + i * 3].g = view_param->edge_data[12 + i * 3].b = 0;
		view_param->edge_data[13 + i * 3].v1 = 9 + i * 3;
		view_param->edge_data[13 + i * 3].v2 = 10 + i * 3;
		view_param->edge_data[13 + i * 3].r = view_param->edge_data[13 + i * 3].g = view_param->edge_data[13 + i * 3].b = 0;
		view_param->edge_data[14 + i * 3].v1 = 9 + i * 3;
		view_param->edge_data[14 + i * 3].v2 = 11 + i * 3;
		view_param->edge_data[14 + i * 3].r = view_param->edge_data[14 + i * 3].g = view_param->edge_data[14 + i * 3].b = 0;
	}
	
	view_param->edge_data[12].b = view_param->edge_data[13].b = view_param->edge_data[14].b = 1.0;
	view_param->edge_data[15].r = view_param->edge_data[16].r = view_param->edge_data[17].r = 1.0;
	view_param->edge_data[18].g = view_param->edge_data[19].g = view_param->edge_data[20].g = 1.0;
		
	/* Transform vertex to world coordinates */
	create_transform_vectors(scene, inv[0], inv[1], inv[2]);
	for(i = 0; i < view_param->vertex_num; i++) {
		t[0] = matrix33_column_sprod(view_param->vertex_data[i].pos, inv, 0) +
			scene->vp[0] * sinf(scene->vp[2]) * cosf(scene->vp[1]);
		t[1] = matrix33_column_sprod(view_param->vertex_data[i].pos, inv, 1) + 
			scene->vp[0] * sinf(scene->vp[2]) * sinf(scene->vp[1]);
		t[2] = matrix33_column_sprod(view_param->vertex_data[i].pos, inv, 2) + 
			scene->vp[0] * cosf(scene->vp[2]);
		
		vector3_assign(view_param->vertex_data[i].pos, t);
	}
	
	return view_param;
}

scene3d_t* scene3d_create(Display *dpy, unsigned width, unsigned height, unsigned depth)
{
scene3d_t *s = (scene3d_t*) secure_alloc(sizeof(scene3d_t));
int i;
	
	s->dpy = dpy;
	s->vp_width = width;
	s->vp_height = height;
	s->zbuffer = (float**) secure_alloc(width * sizeof(float*));
	s->objects = 0;
	for(i = 0; i < width; i++)
		s->zbuffer[i] = (float*) secure_alloc(height * sizeof(float));
	
	s->viewport = (unsigned*) secure_alloc(width * height * sizeof(int));
	s->ximage = XCreateImage(dpy, DefaultVisual(dpy, 0), depth, ZPixmap, 0,
		(char*)s->viewport, width, height, 32, width * sizeof(int));
	if(!s->ximage)
		exit(33);

	s->tam_top = (int*)secure_alloc(sizeof(int) * width);
	s->tam_bottom = (int*)secure_alloc(sizeof(int) * width);
	return s;
}

void scene3d_delete(scene3d_t *s)
{
int i;
	
	if(s) {
		for(i = 0; i < s->vp_width; i++)
			free(s->zbuffer[i]);
		free(s->zbuffer);
		
		free(s->tam_top);
		free(s->tam_bottom);
		XDestroyImage(s->ximage);
		
		scene3d_object_free(s);
		
		free(s);
	}
} 

void scene3d_fill_rect(scene3d_t *s, unsigned x0, unsigned y0, unsigned width, unsigned height, unsigned col)
{
unsigned x, y;
	
	for(x = 0; x < width; x++) {
		for(y = 0; y < height; y++)
			XPutPixel(s->ximage, x0 + x, y0 + y, col);
	}
}

void scene3d_object_add(scene3d_t *scene, const char *obj_name, mesh3d_t *obj_mesh)
{
struct object_node *t = scene->objects;
	
	scene->objects = (struct object_node*) secure_alloc(sizeof(struct object_node));
	scene->objects->next = t;
	scene->objects->name = (char*) secure_alloc(strlen(obj_name) + 1);
	strcpy(scene->objects->name, obj_name);
	scene->objects->mesh = obj_mesh;
}

static void scene3d_object_free(scene3d_t *scene)
{
struct object_node *t, *p = scene->objects;
	
	while(p) {
		t = p->next;
		free(p);
		p = t;
	}
}

mesh3d_t* scene3d_object_delete(scene3d_t *scene, const char *obj_name)
{
struct object_node *p = 0;
struct object_node *t = scene->objects;
	
	while(t) {
		if(!strcmp(t->name, obj_name))
		{
		mesh3d_t *mesh = t->mesh;
			
			free(t->name);
			if(p)
				p->next = t->next;
			else
				scene->objects = t->next;
				
			free(t);
			return mesh;
		}
		
		p = t;
		t = t -> next;
	}
	
	return 0;
}
