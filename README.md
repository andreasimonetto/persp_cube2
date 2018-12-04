PerspCube2
==========

![Screenshot](https://raw.github.com/andreasimonetto/persp_cube2/master/images/screenshot1.png)

Load and display 3D mesh files (.m, .obj, .ply) **having triangular** faces,
implemented using only X Window System graphics primitives.

Rendering algorithm is _Hidden Surface_ based on Z-buffer, while illumination
is a simple _flat shading_.

Compile
-------
- `cd persp_cube2`
- `./configure`
- `make`
- `./src/persp_cube2 mfiles/mannequin.m`

Quick reference
---------------

The header file `scene3d.h` contains the prototypes for the management of
the 3D scene. To create/delete a scene:

```
	scene3d_create(Xdpy, width, height, depth);
	scene3d_delete(scene3d);
```

The user can add/remove any number of objects to/from the scene using:

```
	scene3d_object_add(scene3d, obj_name, obj_mesh);
	scene3d_object_delete(scene3d, obj_name);
```

The file `mesh3d.h` contains the functions for loading, creating and deleting
meshes. Every mesh, no matter if loaded from file or dynamically created, must
be explicitly deallocated:

```
	mesh3d_load(mesh_path);
  mesh3d_create(vertices_num, faces_num, edges_num);
	mesh3d_delete(mesh3d);
```

The entire scene is rendered with the function `scene3d_render()`. The image
buffer is contained in the field `ximage` of the C structure `scene3d_t`.

### Pseudo-code example: mesh loading and rendering

```
display_mesh(Xdpy, Xwindow, XGC, mesh_path)
{
	/* Load the mesh from file */
	mesh3d_t *mymesh = mesh3d_load(mesh_path);
	if(!mymesh)
		throw("Cannot load mesh '" + mesh_path + "'");

	/* Create an empty scene */
	scene3d_t *myscene = scene3d_create(Xdpy, 320, 200, 32);

	/* Add the mesh to the scene */
	scene3d_object_add(myscene, "myMesh", mymesh);

	/* Render the scene */
	scene3d_render(myscene);

	/* Display the scene on the window */
	XPutImage(Xdpy, Xwindow, XGC, myscene->ximage, 0, 0, 0, 0, 320, 200);

	/* Free allocated resources */
	scene3d_delete(myscene);
	mesh3d_delete(mymesh);
}
```
