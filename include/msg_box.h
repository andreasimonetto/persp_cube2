#ifndef _MSG_BOX_H_
#define _MSG_BOX_H_

#include "mesh3d.h"
#include <X11/Xlib.h>

struct _str {
	char *str;
	int str_len;
};

typedef struct {
	struct _str *msgs;
	int cur_row, col_max, lines;
	Window win;
	GC gc;
	Display *dpy;
} msg_box_t;

msg_box_t* msg_box_create(Display *dpy, Window win, GC gc, int lines);
int msg_box_printf(msg_box_t *msg_box, const char *fmt, ...);
void msg_box_draw(const msg_box_t *msg_box, int x, int y);
void msg_box_delete(msg_box_t *msg_box);

#endif
