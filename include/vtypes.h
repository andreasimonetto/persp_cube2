/***************************************************************************
 *            vtypes.h
 *
 *  Thu Mar 20 13:55:00 2008
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
 
#ifndef _VTYPES_H_
#define _VTYPES_H_
#include <math.h>
#include "ply.h"

/**/
typedef float vector3_t[3];

/* V := [v0, v1, v2] */
#define vector3_set(V, v0, v1, v2) { \
	V[0] = v0; \
	V[1] = v1; \
	V[2] = v2; \
}

/* Y := X */
#define vector3_assign(Y, X) { \
	Y[0] = X[0]; \
	Y[1] = X[1]; \
	Y[2] = X[2]; \
}

/* V = A + B */
#define vector3_sum(V, A, B) { \
	V[0] = A[0] + B[0]; \
	V[1] = A[1] + B[1]; \
	V[2] = A[2] + B[2]; \
}

/* V = A - B */
#define vector3_sub(V, A, B) { \
	V[0] = A[0] - B[0]; \
	V[1] = A[1] - B[1]; \
	V[2] = A[2] - B[2]; \
}

/* ||V|| */
#define vector3_norm(V) sqrtf(V[0]*V[0]+V[1]*V[1]+V[2]*V[2])

/* V = A x B */
#define vector3_vprod(V, A, B) { \
	V[0] = A[1] * B[2] - A[2] * B[1]; \
	V[1] = A[2] * B[0] - A[0] * B[2]; \
	V[2] = A[0] * B[1] - A[1] * B[0]; \
}

/* A * B */
#define vector3_sprod(A, B) (A[0]*B[0]+A[1]*B[1]+A[2]*B[2])

/* V = cA */
#define vector3_mult(V, c, A) { \
	V[0] = (c) * A[0]; \
	V[1] = (c) * A[1]; \
	V[2] = (c) * A[2]; \
}

/**/
typedef vector3_t matrix33_t[3];

#define matrix33_column_sprod(A, M, col) (M[0][col]*A[0]+M[1][col]*A[1]+M[2][col]*A[2])

/**/
typedef float angle_t;

#define angle_add(alpha, val) { \
	(alpha) += (val); \
	while((alpha) >= 2 * M_PI) \
		(alpha) -= 2 * M_PI; \
	while((alpha) < 0) \
		(alpha) += 2 * M_PI; \
}

/* vertex and face definitions for a polygonal object */
typedef struct {
	/* vertex position */
	vector3_t pos;
	
	/* vertex color */
	float r, g, b;
	
	/* */
	float nx, ny, nz;
	
	/* other properties */
	void *other_props;
} vertex_t;

/* list of property information for a vertex */
extern const ply_property_t vertex_props[];

typedef struct {
	/* vertex indexes */
	int v1, v2;
	
	/* */
	float r, g, b; 
	
	/* other properties */
	void *other_props;
} edge_t;

/* list of property information for an vertex 
extern const ply_property_t edge_props[]; */

typedef struct {
	/* number of vertex indices in list */
	unsigned char nverts;
	
	/* vertex index list */
	int *verts;
	
	/* normal vector */
	vector3_t normal;
	
	/* other properties */
	void *other_props;
} face_t;

/* list of property information for a face */
extern const ply_property_t face_props[];

/**/
typedef unsigned char bitvect_t;

/**/
#define bitvect_set(bitv, id) (bitv |= (0x1 << id))

/**/
#define bitvect_isset(bitv, id) ((bitv >> id) & 0x1)


/* Memory allocation routine */
extern void* __secure_alloc(int block_size, const char *filename, int lineno);

#define secure_alloc(block_size) __secure_alloc(block_size, __FILE__, __LINE__)

#endif
