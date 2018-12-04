/***************************************************************************
 *            stack.h
 *
 *  Fri Mar 21 14:14:38 2008
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
 
 #ifndef _STACK_H_
 #define _STACK_H_
 
struct stack_node {
	void *data;
	struct stack_node *next;
};

typedef struct {
	struct stack_node *top;
	unsigned elem_num;
	size_t elem_size;
} stack_t;


#define stack_init(s, el_size) { \
	(s).top = 0; \
	(s).elem_num = 0; \
	(s).elem_size = (el_size); \
}

#define stack_push(s, dat) { \
	if(!(s).top) { \
		(s).top = (struct stack_node*) malloc(sizeof(struct stack_node)); \
		(s).top->next = 0; \
	} \
	else { \
		struct stack_node *t = (s).top; \
		(s).top = (struct stack_node*) malloc(sizeof(struct stack_node)); \
		(s).top->next = t; \
	} \
	(s).top->data = malloc((s).elem_size); \
	memcpy((s).top->data, (dat), (s).elem_size); \
	(s).elem_num++; \
}

#define stack_peek(s) ((s).top ? (s).top->data : 0)

#define stack_pop(s) { \
	if((s).elem_num > 0) { \
		struct stack_node *t = (s).top->next; \
		if((s).top) { \
			if((s).top->data) \
				free((s).top->data); \
			free((s).top); \
		} \
		(s).top = t; \
		(s).elem_num--; \
	} \
}

#define stack_delete(s) { \
	struct stack_node *t; \
	while((s).top) { \
		t = (s).top->next; \
		free((s).top->data); \
		free((s).top); \
		(s).top = t; \
	} \
	(s).elem_num = 0; \
}

#define stack_to_vect(s, vect, vect_size, vect_type) { \
	if(((vect_size) = (s).elem_num)) { \
		(vect) = (vect_type*) malloc((s).elem_num * sizeof(vect_type)); \
		while((s).top) { \
			memcpy((vect) + (s).elem_num - 1, (s).top->data, sizeof(vect_type)); \
			stack_pop(s); \
		} \
	} \
	else \
		vect = 0; \
}

#endif
