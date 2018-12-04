#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif
#include <stdio.h>
#include <math.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include "msg_box.h"
#include "mesh3d.h"
#include "scene3d.h"

static Display *dpy; /* default display */
static Window app_win; /* default application window */
static GC app_win_gc;
static int app_win_width, app_win_height, app_win_depth;

/* Connect to X server and create the appication window */
void create_app_window(Display **dpy, Window *app_win, GC *gc, int *width, int *height, int *depth)
{
Visual *visual;
XSetWindowAttributes attributes;
XWindowAttributes attr;
XFontStruct *fontinfo;
XColor color, dummy;
XGCValues gr_values;
Window root_win;
	
	/* Open the connection to the X server */
	*dpy = XOpenDisplay(NULL);
	visual = DefaultVisual(*dpy, 0);
	*depth = DefaultDepth(*dpy, 0);
	root_win = DefaultRootWindow(*dpy);
	
	/* Create a graphic context */
	fontinfo = XLoadQueryFont(*dpy, "6x10");
	XAllocNamedColor(*dpy, DefaultColormap(*dpy, 0), "black", &color, &dummy);
	gr_values.font = fontinfo->fid;
	gr_values.foreground = color.pixel;
	*gc = XCreateGC(*dpy, root_win, GCFont+GCForeground, &gr_values);
	
	/* Setup application window attributes */
	attributes.background_pixel = XWhitePixel(*dpy, 0);
	XGetWindowAttributes(*dpy, root_win, &attr);
	attr.x = attr.width * 0.025;
	attr.y = attr.height * 0.075;
	attr.width *= 0.95;
	attr.height *= 0.85;
	
	/* Create the application window */
	*app_win = XCreateWindow(*dpy, root_win, attr.x, attr.y, attr.width + 1, 
		attr.height, 1, *depth, InputOutput, visual, CWBackPixel, &attributes);	
	
	XSelectInput(*dpy, *app_win, KeyPressMask | ExposureMask | ButtonPressMask |
		ButtonReleaseMask | PointerMotionMask);
	
	/* Query application window attributes */
	XGetWindowAttributes(*dpy, *app_win, &attr);
	*width = attr.width;
	*height = attr.height;
	*depth = attr.depth;
	
	/* Map windows on display */
	XMapWindow(*dpy, *app_win);
}

/* Draw the entire scene from two different points of view */
void redraw(scene3d_t *scene_1st, scene3d_t *scene_3rd)
{
mesh3d_t *view_param;
msg_box_t *msg_box;
	
	/* First person view: render */
	scene3d_render(scene_1st);
	
	/* Copy first person view on the application window */
	XPutImage(dpy, app_win, app_win_gc, scene_1st->ximage, 0, 0, 0, 0, scene_1st->vp_width, app_win_height);
	
	/* Draw a box containing first person view information */
	msg_box = msg_box_create(dpy, app_win, app_win_gc, 3);
	msg_box_printf(msg_box, "alpha: %.3f ; front-plane: %.3f ; back-plane: %.3f", scene_1st->alpha, scene_1st->front_plane_z, scene_1st->back_plane_z);
	msg_box_printf(msg_box, "vu: [D=%.3f, theta=%.3f, phi=%.3f]", scene_1st->vu[0], scene_1st->vu[1], scene_1st->vu[2]);
	msg_box_printf(msg_box, "vp: [D=%.3f, theta=%.3f, phi=%.3f]", scene_1st->vp[0], scene_1st->vp[1], scene_1st->vp[2]);
	msg_box_draw(msg_box, 6, 6);
	msg_box_delete(msg_box);

	/* Third person view: view frustum creation */
	view_param = mesh3d_view_param_create(scene_1st);
	scene3d_object_add(scene_3rd, "view_param", view_param);

	/* Third person view: scene render */
	scene3d_render(scene_3rd);
	
	/* Third person view: view frustum delete */
	if((view_param = scene3d_object_delete(scene_3rd, "view_param"))) {
		mesh3d_delete(view_param);
	}

	/* Copy third person view on the application window */
	XPutImage(dpy, app_win, app_win_gc, scene_3rd->ximage, 0, 0, scene_1st->vp_width + 1, 0, scene_3rd->vp_width, app_win_height);
	
	/* Draw a box containing third person view information */
	msg_box = msg_box_create(dpy, app_win, app_win_gc, 3);
	msg_box_printf(msg_box, "alpha: %.3f ; front-plane: %.3f ; back-plane: %.3f", scene_3rd->alpha, scene_3rd->front_plane_z, scene_3rd->back_plane_z);
	msg_box_printf(msg_box, "vu: [D=%.3f, theta=%.3f, phi=%.3f]", scene_3rd->vu[0], scene_3rd->vu[1], scene_3rd->vu[2]);
	msg_box_printf(msg_box, "vp: [D=%.3f, theta=%.3f, phi=%.3f]", scene_3rd->vp[0], scene_3rd->vp[1], scene_3rd->vp[2]);
	msg_box_draw(msg_box, scene_1st->vp_width + 7, 6);
	msg_box_delete(msg_box);
		
	/* Draw a white separation line between the views */
	XSetForeground(msg_box->dpy, msg_box->gc, 0xFFFFFF);
	XDrawLine(dpy, app_win, app_win_gc, scene_1st->vp_width, 0, scene_1st->vp_width, scene_1st->vp_height);
	XSetForeground(msg_box->dpy, msg_box->gc, 0);
}

int main(int argc, char *argv[])
{
mesh3d_t *mesh;
char *mesh_path = (argc > 1 ? argv[1] : 0);
scene3d_t *scene, *scene_1st, *scene_3rd;
XEvent event;
int quit = 0, i, nev, x, y;
float maxcoord;
	
	/**/
	if(!mesh_path) {
		fprintf(stderr, "You must specify the path to the mesh to load (e.g. mfiles/cube.m)\n");
		return 1;
	}
	
	/* Load the mesh file */
	if(!(mesh = mesh3d_load(mesh_path))) {
		fprintf(stderr, "Unable to load mesh: %s.\n", mesh_path);
		mesh3d_delete(mesh);
		return 2;
	}
	printf("%d vertices, %d faces, %d edges\n", mesh->vertex_num, mesh->face_num, mesh->edge_num);	
	printf("load ok.\n");
	
	/**/
	maxcoord = mesh3d_max_coord(mesh);
	
	/* Create the application window */
	create_app_window(&dpy, &app_win, &app_win_gc, &app_win_width, &app_win_height, &app_win_depth);
	/*XSetStandardProperties(dpy, app_win, "persp_cube2", "persp_cube2", None, argv, argc, 0);*/

	/* Setup the main (first person) render buffer */
	scene_1st = scene3d_create(dpy, app_win_width / 2, app_win_height, app_win_depth);
	vector3_set(scene_1st->vu, 1.0, 0.0, 0.0);
	vector3_set(scene_1st->vp, 5.0 * maxcoord, M_PI / 4.0, M_PI / 4.0);
	/*scene_1st->d = 4.0;*/
	scene_1st->alpha = M_PI / 4.0;
	scene_1st->front_plane_z = 0.25;
	scene_1st->back_plane_z = 6.0 * maxcoord;
	
	scene3d_object_add(scene_1st, "mesh1", mesh);
	
	/* Setup the secondary (third person) render buffer */
	scene_3rd = scene3d_create(dpy, app_win_width / 2, app_win_height, app_win_depth);
	vector3_set(scene_3rd->vu, 1.0, 0.0, 0.0);
	vector3_set(scene_3rd->vp, 16.0 * maxcoord, 0.27, 0.81);
	/*scene_3rd->d = 16.0;*/
	scene_3rd->alpha = M_PI / 4.0;
	scene_3rd->front_plane_z = 0.25;
	scene_3rd->back_plane_z = 32.0;
	
	scene3d_object_add(scene_3rd, "mesh1", mesh);
	
	/* Main loop */
	while(!quit){
		/* Flush pending events */
		if((nev = XEventsQueued(dpy, QueuedAlready)) > 1) {
			for(i = 1; i < nev; i++) 
				XNextEvent(dpy, &event);
		}
		XNextEvent(dpy, &event);

		switch(event.type) {
			case Expose:				
				redraw(scene_1st, scene_3rd);
				break;
			case KeyPress:
				switch(XLookupKeysym(&event.xkey, 0)) {
					case XK_Page_Up:
						angle_add(scene_1st->alpha, M_PI / 64.0);
						redraw(scene_1st, scene_3rd);
					break;
					case XK_Page_Down:
						angle_add(scene_1st->alpha, -M_PI / 64.0);
						redraw(scene_1st, scene_3rd);
					break;
					case XK_Left:
						angle_add(scene_1st->vu[2], M_PI / 64.0);
						redraw(scene_1st, scene_3rd);
					break;
					case XK_Right:
						angle_add(scene_1st->vu[2], -M_PI / 64.0);
						redraw(scene_1st, scene_3rd);
					break;
					case XK_Escape:
						quit = 1;
					break;
				}
				break;
			case ButtonPress:
				/* Determine which window is clicked */
				scene = (event.xmotion.x > app_win_width / 2 ? scene_3rd : scene_1st);
					
				/* Left click: camera rotation */
				if(((XButtonPressedEvent*)&event)->button == Button1) {
					x = event.xmotion.x;
					y = event.xmotion.y;
					while(event.type != ButtonRelease)
					{
					int i, nx = x, ny = y;
					
						nev = XEventsQueued(dpy, QueuedAlready);
						if(!nev) {
							XPeekEvent(dpy, &event);
							nev = 1;
						}
						
						for(i = 0; i < nev && event.type != ButtonRelease; i++) {
							XNextEvent(dpy, &event);
							if(event.type == MotionNotify) {
								nx = event.xmotion.x;
								ny = event.xmotion.y;							
							}
						}
						
						if(nx != x || ny != y) {
							angle_add(scene->vp[1], -M_PI / 128.0 * (nx - x));
							angle_add(scene->vp[2], -M_PI / 128.0 * (ny - y));
							redraw(scene_1st, scene_3rd);
							x = nx;
							y = ny;
						}
					}
				}
				else if(((XButtonPressedEvent*)&event)->button == Button4) {
					scene->vp[0] -= 0.125 * maxcoord;
					redraw(scene_1st, scene_3rd);
				}
				else if(((XButtonPressedEvent*)&event)->button == Button5) {			
					scene->vp[0] += 0.125 * maxcoord;
					redraw(scene_1st, scene_3rd);
				}
				break;
			case ConfigureNotify:
				/* reconfigure size of window here */
				break;
		}
	}
	
	/* Unload the mesh and release the X display */
	mesh3d_delete(mesh);
	scene3d_delete(scene_1st);
	scene3d_delete(scene_3rd);
	/*XUnloadFont(dpy, fontinfo->fid);*/
	XFreeGC(dpy, app_win_gc);
	XCloseDisplay(dpy);	
	
	return 0;
}
