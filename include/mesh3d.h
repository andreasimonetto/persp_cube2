/***************************************************************************
 *            mesh3d.h
 *
 *  Thu Mar 20 15:38:24 2008
 *  Copyright  2008  User
 *  Email
 ****************************************************************************/

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
 
#ifndef _MESH3D_H_
#define _MESH3D_H_
#include "vtypes.h"

typedef struct {
	vertex_t *vertex_data;
	unsigned vertex_num;
	
	edge_t *edge_data;
	unsigned edge_num;
	
	face_t *face_data;
	unsigned face_num;
} mesh3d_t;

mesh3d_t* mesh3d_create(unsigned vertex_num, unsigned face_num, unsigned edge_num);
mesh3d_t* mesh3d_load(const char *path);
float mesh3d_max_coord(mesh3d_t *mesh);
void mesh3d_delete(mesh3d_t *mesh);

#endif
