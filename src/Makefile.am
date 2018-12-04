## Process this file with automake to produce Makefile.in

AM_CPPFLAGS = \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" $(PERSP_CUBE2_CFLAGS)

AM_CFLAGS = -Wall -g

bin_PROGRAMS = persp_cube2

persp_cube2_SOURCES =        \
	mesh3d.c             \
	mesh3d_load_m.l      \
	mesh3d_load_obj.l    \
	ply.c                \
	vtypes.c             \
	main.c               \
	../include/mesh3d.h  \
	../include/ply.h     \
	../include/stack.h   \
	../include/vtypes.h  \
	msg_box.c            \
	../include/msg_box.h \
	../include/scene3d.h \
	scene3d.c            \
	clip_line.c

persp_cube2_CPPFLAGS = -I../include

persp_cube2_CFLAGS = -Wall

persp_cube2_LDFLAGS =

persp_cube2_LDADD =             \
	$(PERSP_CUBE2_LIBS)     \
	-lm -lX11 -lXpm -lXext  \
	$(X_LIBS)               \
	$(X_EXTRA_LIBS)

