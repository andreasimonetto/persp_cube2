/* Scanner for .m files */

%option noyywrap
%option outfile="lex.yy.c"
%option prefix="__mesh3d_scan_m"

%{
#include "stack.h"
#include "mesh3d.h"
#include <errno.h>

#define YY_DECL int __mesh3d_scan_m(stack_t *vert, stack_t *vert_id, stack_t *face, stack_t *edge)

static int sorted = 0;
%}

DIGIT   [0-9]
UINT    {DIGIT}+
INT     "-"?{UINT}
FLOAT   "-"?{UINT}("."{UINT})?(e[ \t]*{INT})?

%%

Vertex[ \t]+{UINT}[ \t]+{FLOAT}[ \t]+{FLOAT}[ \t]+{FLOAT}	{
	vertex_t v;
	int id;
	sscanf(yytext, "Vertex %d %f %f %f", &id, &(v.pos[0]), &(v.pos[1]), &(v.pos[2]));
	sorted = (id - 1 == sorted ? id : 0);
	stack_push(*vert_id, &id);
	stack_push(*vert, &v);
}

Face[ \t]+{UINT}[ \t]+{UINT}[ \t]+{UINT}[ \t]+{UINT}	{
	face_t f;
	int id;
	
	f.nverts = 3;
	f.verts = (int*)malloc(3 * sizeof(int));
	sscanf(yytext, "Face %d %d %d %d", &id, &(f.verts[0]), &(f.verts[1]), &(f.verts[2]));
	
	/* convert id range from [1~N] to [0~N-1]*/
	f.verts[0]--;
	f.verts[1]--;
	f.verts[2]--;
	stack_push(*face, &f);
}

Edge[ \t]+{UINT}[ \t]+{UINT}[ \t]+{UINT}	{
	edge_t e;
	int id;

	e.r = e.g = e.b = 0.0;
	sscanf(yytext, "Edge %d %d %d", &id, &(e.v1), &(e.v2));
	
	/* convert id range from [1~N] to [0~N-1]*/
	e.v1--;
	e.v2--;
	stack_push(*edge, &e);
}

Edge[ \t]+{UINT}[ \t]+{UINT}[ \t]+"{sharp}"	/* don't really know! */

[ \t\r\n]+	/* eat up whitespace */

"#".*"\r"?"\n"	/* eat up comments */

.	{
	fprintf(stderr, "Unrecognized character: %s\n", yytext);
}


%%


int mesh3d_load_m(mesh3d_t *mesh, FILE *fp)
{
stack_t vertex_stack, vertex_id_stack, face_stack, edge_stack;


	/* Set scanner file pointer */
	yyin = fp;

	/* Init stacks */
	stack_init(vertex_stack, sizeof(vertex_t));
	stack_init(face_stack, sizeof(face_t));
	stack_init(edge_stack, sizeof(edge_t));
	stack_init(vertex_id_stack, sizeof(int));

	/* Init stacks */	
	__mesh3d_scan_m(&vertex_stack, &vertex_id_stack, &face_stack, &edge_stack);
	
	/* Convert stacks into vectors */
	stack_to_vect(vertex_stack, mesh->vertex_data, mesh->vertex_num, vertex_t);
	stack_to_vect(face_stack, mesh->face_data, mesh->face_num, face_t);
	stack_to_vect(edge_stack, mesh->edge_data, mesh->edge_num, edge_t);

	/* Check if vertex wasn't already sorted */
	if(!sorted)
	{
	int i, j, k;
	int *vert_id, vert_num;
	int found;

		stack_to_vect(vertex_id_stack, vert_id, vert_num, int);	
		for(i = 0; i < mesh->face_num; i++) {
			for(j = 0; j < mesh->face_data[i].nverts; j++) {
				for(k = 0, found = 0; k < vert_num && !found; k++) {
					if(mesh->face_data[i].verts[j] == vert_id[k]) {
						mesh->face_data[i].verts[j] = k;
						found = 1;
					}
				}
			}
		}

		for(i = 0; i < mesh->edge_num; i++) {
			for(k = 0, found = 0; k < vert_num && found < 2; k++) {
				if(mesh->edge_data[i].v1 == vert_id[k]) {
					mesh->edge_data[i].v1 = k;
					found++;
				}
				if(mesh->edge_data[i].v2 == vert_id[k]) {
					mesh->edge_data[i].v2 = k;
					found++;
				}
			}
		}
	}
	else {
		stack_delete(vertex_id_stack);
	}

	fclose(fp);
	return 0;
}
