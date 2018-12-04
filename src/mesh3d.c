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
#include <X11/Xutil.h>
#include "mesh3d.h"
#include "ply.h"

static mesh3d_t* mesh3d_create_empty();
extern int mesh3d_load_m(mesh3d_t *mesh, FILE *fp);
extern int mesh3d_load_obj(mesh3d_t *mesh, FILE *fp);
int mesh3d_load_ply(mesh3d_t *mesh, FILE *fp);

mesh3d_t* mesh3d_load(const char *path)
{
int i, extension_len = 0;
const char *extension_ptr = path ? path + strlen(path) - 1 : 0;
FILE *fp = path ? fopen(path, "rb") : 0;
int retval;
mesh3d_t *mesh;
	
	if(!fp || !path || !extension_ptr)
		return 0;
	
	while(extension_ptr > path && extension_len < 10 && *(extension_ptr - 1) != '.') {
		extension_len++;
		extension_ptr--;
	}
	
	if(extension_ptr <= path || (extension_len == 10 && *(extension_ptr - 1) != '.'))
		return 0;
	
	mesh = mesh3d_create_empty();
	if(!strcasecmp(extension_ptr, "ply")) {
		if((retval = mesh3d_load_ply(mesh, fp))) {
			free(mesh);
			return 0;
		}
	}
	else if(!strcasecmp(extension_ptr, "obj")) {
		if((retval = mesh3d_load_obj(mesh, fp))) {
			free(mesh);
			return 0;
		}
	}
	else if(!strcasecmp(extension_ptr, "m")) {
		if((retval = mesh3d_load_m(mesh, fp))) {
			free(mesh);
			return 0;
		}
	}
	else {
		free(mesh);
		return 0;
	}
	
	/* TODO: Convert all the faces to triangles */
	/* ... */
	
	/* Faces and edges normals */
	for(i = 0; i < mesh->face_num; i++)
	{
	float *v1 = mesh->vertex_data[mesh->face_data[i].verts[0]].pos;
	float *v2 = mesh->vertex_data[mesh->face_data[i].verts[1]].pos;
	float *v3 = mesh->vertex_data[mesh->face_data[i].verts[2]].pos;
	vector3_t l1, l2;
	float c;
		
		vector3_sub(l1, v1, v2);
		vector3_sub(l2, v1, v3);
		
		vector3_vprod(mesh->face_data[i].normal, l1, l2);
		c = 1.0 / vector3_norm(mesh->face_data[i].normal);											
		vector3_mult(mesh->face_data[i].normal, c, mesh->face_data[i].normal);
		
	}
	
	return mesh;
}

static mesh3d_t* mesh3d_create_empty()
{
mesh3d_t *mesh = (mesh3d_t*)malloc(sizeof(mesh3d_t));

	mesh->vertex_num = mesh->face_num = mesh->edge_num = 0;
	mesh->vertex_data = 0;
	mesh->face_data = 0;
	mesh->edge_data = 0;
	return mesh;
}

mesh3d_t* mesh3d_create(unsigned vertex_num, unsigned face_num, unsigned edge_num)
{
mesh3d_t *mesh = mesh3d_create_empty();

	if((mesh->vertex_num = vertex_num) > 0)
		mesh->vertex_data = (vertex_t*)malloc(vertex_num * sizeof(vertex_t));
	
	if((mesh->face_num = face_num) > 0)
		mesh->face_data = (face_t*)malloc(face_num * sizeof(face_t));

	if((mesh->edge_num = edge_num) > 0)
		mesh->edge_data = (edge_t*)malloc(edge_num * sizeof(edge_t));

	return mesh;	
}

void mesh3d_delete(mesh3d_t *mesh)
{
unsigned i;
	
	if(mesh->vertex_num && mesh->vertex_data)
		free(mesh->vertex_data);
	
	if(mesh->face_num && mesh->face_data) {
		for(i = 0; i < mesh->face_num; i++) {
			if(mesh->face_data[i].nverts && mesh->face_data[i].verts)
				free(mesh->face_data[i].verts);
		}
		free(mesh->face_data);
	}
	
	if(mesh->edge_num && mesh->edge_data) 
		free(mesh->edge_data);
	
	free(mesh);
}

int mesh3d_load_ply(mesh3d_t *mesh, FILE *fp)
{
ply_file_t *in_ply;
ply_other_prop_t *vert_other, *face_other;
char *elem_name;
int per_vertex_color = 0;
int has_normals = 0;
int elem_count;
int i, j;
	
	/* Read in the original PLY object */
	in_ply = ply_read(fp);
	for(i = 0; i < in_ply->num_elem_types; i++) {
		/* prepare to read the i'th list of elements */
		elem_name = ply_setup_element_read(in_ply, i, &elem_count);
		
		if(!strcmp("vertex", elem_name)) {
			/* create a vertex list to hold all the vertices */
			mesh->vertex_data = (vertex_t*)secure_alloc(sizeof(vertex_t) * elem_count);
			mesh->vertex_num = elem_count;
			
			/* set up for getting vertex elements */
			ply_setup_property(in_ply, &vertex_props[0]);
			ply_setup_property(in_ply, &vertex_props[1]);
			ply_setup_property(in_ply, &vertex_props[2]);
			
			for (j = 0; j < in_ply->elems[i]->nprops; j++) {
				ply_property_t *prop;
				prop = in_ply->elems[i]->props[j];
				if(!strcmp("r", prop->name)) {
					ply_setup_property(in_ply, &vertex_props[3]);
					per_vertex_color = 1;
				}
				else if(!strcmp("g", prop->name)) {
					ply_setup_property(in_ply, &vertex_props[4]);
					per_vertex_color = 1;
				}
				else if(!strcmp("b", prop->name)) {
					ply_setup_property(in_ply, &vertex_props[5]);
					per_vertex_color = 1;
				}
				else if(!strcmp("nx", prop->name)) {
					ply_setup_property(in_ply, &vertex_props[6]);
					has_normals = 1;
				}
				else if(!strcmp("ny", prop->name)) {
					ply_setup_property(in_ply, &vertex_props[7]);
					has_normals = 1;
				}
				else if(!strcmp("nz", prop->name)) {
					ply_setup_property(in_ply, &vertex_props[8]);
					has_normals = 1;
				}
			}
			
			vert_other = ply_get_other_properties(in_ply, offsetof(vertex_t, other_props));

			/* grab all the vertex elements */
			for (j = 0; j < elem_count; j++) {
				mesh->vertex_data[j].r = 1;
				mesh->vertex_data[j].g = 1;
				mesh->vertex_data[j].b = 1;
				ply_get_element(in_ply, mesh->vertex_data + j);
			}
		}
    	else if(!strcmp("face", elem_name)) {
			/* create a list to hold all the face elements */
			mesh->face_data = (face_t*)secure_alloc(sizeof(face_t) * elem_count);
			mesh->face_num = elem_count;
			
			/* set up for getting face elements */
			ply_setup_property(in_ply, &face_props[0]);
			face_other = ply_get_other_properties(in_ply, offsetof(face_t, other_props));
			
			/* grab all the face elements */
			for(j = 0; j < elem_count; j++) {
				ply_get_element(in_ply, mesh->face_data + j);
			}
		}
		else
			ply_get_other_element(in_ply);
	}

	ply_close(in_ply);
	ply_free(in_ply);
	return 0;
}

float mesh3d_max_coord(mesh3d_t *mesh)
{
int i, j;
float c, max = 0;
		
	for(i = 0; i < mesh->vertex_num; i++) {
		for(j = 0; j < 3; j++) {
			c = fabs(mesh->vertex_data[i].pos[j]);
			if(c > max) 
				max = c;
		}
	}
	
	return max;
}
