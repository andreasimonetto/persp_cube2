#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "msg_box.h"

msg_box_t* msg_box_create(Display *dpy, Window win, GC gc, int lines)
{
msg_box_t *msg_box = (msg_box_t*) malloc(sizeof(msg_box_t));
	
	msg_box->col_max = msg_box->cur_row = 0;
	msg_box->dpy = dpy;
	msg_box->win = win;
	msg_box->gc = gc;
	msg_box->lines = lines;
	msg_box->msgs = (struct _str*) malloc(lines * sizeof(struct _str));
	return msg_box;
}

void msg_box_delete(msg_box_t *msg_box)
{
int i;
	
	for(i = 0; i < msg_box->cur_row; i++) {
		if(msg_box->msgs[i].str)
			free(msg_box->msgs[i].str);
	}
	
	free(msg_box->msgs);
	free(msg_box);
}

int msg_box_printf(msg_box_t *msg_box, const char *fmt, ...)
{
char buf[257];
va_list ap;
	
	if(msg_box->cur_row > msg_box->lines)
	   return -1;
	
	va_start(ap, fmt);
	msg_box->msgs[msg_box->cur_row].str_len = vsnprintf(buf, 256, fmt, ap);
	msg_box->msgs[msg_box->cur_row].str = (char*) malloc((msg_box->msgs[msg_box->cur_row].str_len + 1) * sizeof(char));
	strcpy(msg_box->msgs[msg_box->cur_row].str, buf);

	if(msg_box->msgs[msg_box->cur_row].str_len > msg_box->col_max)
	   msg_box->col_max = msg_box->msgs[msg_box->cur_row].str_len;
	msg_box->cur_row++;	
	
	return msg_box->msgs[msg_box->cur_row].str_len;
}

void msg_box_draw(const msg_box_t *msg_box, int x, int y)
{
int i;
	
	XSetForeground(msg_box->dpy, msg_box->gc, 0xFFFFFF);
	XFillRectangle(msg_box->dpy, msg_box->win, msg_box->gc, x, y, (msg_box->col_max + 1) * 6, msg_box->lines * 12 + 7);
	XSetForeground(msg_box->dpy, msg_box->gc, 0);
	
	for(i = 0; i < msg_box->cur_row; i++) 
		XDrawString(msg_box->dpy, msg_box->win, msg_box->gc, x + 3, y + (i + 1) * 12 + 1, msg_box->msgs[i].str, msg_box->msgs[i].str_len);

	XDrawRectangle(msg_box->dpy, msg_box->win, msg_box->gc, x, y, (msg_box->col_max + 1) * 6, msg_box->lines * 12 + 7);
}
