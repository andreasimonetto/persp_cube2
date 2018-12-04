#include <stdio.h>
#include <stdlib.h>
#include "vtypes.h"

/* list of property information for a vertex */
const ply_property_t vertex_props[] = {
  { "x", Float32, Float32, offsetof(vertex_t, pos[0]), 0, 0, 0, 0 },
  { "y", Float32, Float32, offsetof(vertex_t, pos[1]), 0, 0, 0, 0 },
  { "z", Float32, Float32, offsetof(vertex_t, pos[2]), 0, 0, 0, 0 },
  { "r", Float32, Float32, offsetof(vertex_t, r), 0, 0, 0, 0 },
  { "g", Float32, Float32, offsetof(vertex_t, g), 0, 0, 0, 0 },
  { "b", Float32, Float32, offsetof(vertex_t, b), 0, 0, 0, 0 },
  { "nx", Float32, Float32, offsetof(vertex_t, nx), 0, 0, 0, 0 },
  { "ny", Float32, Float32, offsetof(vertex_t, ny), 0, 0, 0, 0 },
  { "nz", Float32, Float32, offsetof(vertex_t, nz), 0, 0, 0, 0 }
};

/* list of property information for a face */
const ply_property_t face_props[] = { 
  { "vertex_indices", Int32, Int32, offsetof(face_t, verts), 1, Uint8, Uint8, offsetof(face_t, nverts) },
};

void* __secure_alloc(int block_size, const char *filename, int lineno)
{
void *mem = malloc(block_size);
	
	if(!mem) { 
		fprintf(stderr, "%s:%d: error: failed to allocate block of %d bytes.\n", filename, lineno, block_size);
		exit(1); 
	}
	return mem; 
}
