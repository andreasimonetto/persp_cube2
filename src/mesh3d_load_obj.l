/* Scanner for .obj (Wavefront) files */

%option noyywrap
%option outfile="lex.yy.c"
%option prefix="__mesh3d_scan_obj"

%{
#include "stack.h"
#include "mesh3d.h"
#include <errno.h>

#define YY_DECL int __mesh3d_scan_obj(stack_t *vert, stack_t *face, stack_t *edge)

%}

DIGIT   [0-9]
UINT    {DIGIT}+
INT     "-"?{UINT}
FLOAT   "-"?{UINT}("."{UINT})?(e[ \t]*{INT})?

%%

v[ \t]+{FLOAT}[ \t]+{FLOAT}[ \t]+{FLOAT}	{
	vertex_t v;
	sscanf(yytext, "v %f %f %f\n", &(v.pos[0]), &(v.pos[1]), &(v.pos[2]));
	stack_push(*vert, &v);
}

f[ \t]+{INT}([ \t]+{INT})+	{
	face_t f;
	int verts_buf[64];
	int id, i = 1;
	
	f.nverts = 0;
	while(yytext[i]) {
		/* scan a symbol */
		if(sscanf(yytext + i, " %d", &id) == 1) 
			verts_buf[f.nverts++] = id > 0 ? id - 1 : id;
		
		/* skip spaces */
		while(yytext[i] && (yytext[i] == ' ' || yytext[i] == '\t'))
			i++;
		
		/* skip the scanned symbol */		
		while(yytext[i] && yytext[i] != ' ' && yytext[i] != '\t')
			i++;
	}
	
	f.verts = malloc(f.nverts * sizeof(int));
	memcpy(f.verts, verts_buf, f.nverts * sizeof(int));
	stack_push(*face, &f);
}

e[ \t]+{INT}[ \t]+{INT}	{
	edge_t e;

	e.r = e.g = e.b = 0.0;	
	sscanf(yytext, "e %d %d\n", &(e.v1), &(e.v2));
	
	/* convert id range from [1~N] to [0~N-1]*/
	e.v1--;
	e.v2--;
	stack_push(*face, &e);
}

#.*"\r"?"\n"	/* eat up comments */

[ \t\r\n]+	/* eat up whitespace */

.	{
	fprintf(stderr, "Unrecognized character: %s\n", yytext);
}


%%


int mesh3d_load_obj(mesh3d_t *mesh, FILE *fp)
{
stack_t vertex_stack, face_stack, edge_stack;

	/* Set scanner file pointer */
	yyin = fp;

	/* Init stacks */
	stack_init(vertex_stack, sizeof(vertex_t));
	stack_init(face_stack, sizeof(face_t));
	stack_init(edge_stack, sizeof(edge_t));

	/* Init stacks */	
	__mesh3d_scan_obj(&vertex_stack, &face_stack, &edge_stack);
	
	/* Convert stacks into vectors */
	stack_to_vect(vertex_stack, mesh->vertex_data, mesh->vertex_num, vertex_t);
	stack_to_vect(face_stack, mesh->face_data, mesh->face_num, face_t);
	stack_to_vect(edge_stack, mesh->edge_data, mesh->edge_num, edge_t);
	
	fclose(fp);
	return 0;
}
