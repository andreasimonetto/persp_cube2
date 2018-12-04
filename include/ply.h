/*

Header for PLY polygon files.

- Greg Turk

A PLY file contains a single polygonal _object_.

An object is composed of lists of _elements_.  Typical elements are
vertices, faces, edges and materials.

Each type of element for a given object has one or more _properties_
associated with the element type.  For instance, a vertex element may
have as properties three floating-point values x,y,z and three unsigned
chars for red, green and blue.

-----------------------------------------------------------------------

Copyright (c) 1998 Georgia Institute of Technology.  All rights reserved.   
  
Permission to use, copy, modify and distribute this software and its   
documentation for any purpose is hereby granted without fee, provided   
that the above copyright notice and this permission notice appear in   
all copies of this software and that you do not sell the software.   
  
THE SOFTWARE IS PROVIDED "AS IS" AND WITHOUT WARRANTY OF ANY KIND,   
EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY   
WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.   

*/

#ifndef _PLY_H_
#define _PLY_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stddef.h>

#define PLY_ASCII      1        /* ascii PLY file */
#define PLY_BINARY_BE  2        /* binary PLY file, big endian */
#define PLY_BINARY_LE  3        /* binary PLY file, little endian */

#define PLY_OKAY    0           /* ply routine worked okay */
#define PLY_ERROR  -1           /* error in ply routine */

/* scalar data types supported by PLY format */

#define StartType  0
#define Int8       1
#define Int16      2
#define Int32      3
#define Uint8      4
#define Uint16     5
#define Uint32     6
#define Float32    7
#define Float64    8
#define EndType    9

#define  PLY_SCALAR  0
#define  PLY_LIST    1
#define  PLY_STRING  2


typedef struct {    /* description of a property */

  char *name;                   /* property name */
  int external_type;            /* file's data type */
  int internal_type;            /* program's data type */
  int offset;                   /* offset bytes of prop in a struct */

  int is_list;                  /* 0 = scalar, 1 = list, 2 = char string */
  int count_external;           /* file's count type */
  int count_internal;           /* program's count type */
  int count_offset;             /* offset byte for list count */

} ply_property_t;

typedef struct {     /* description of an element */
  char *name;                   /* element name */
  int num;                      /* number of elements in this object */
  int size;                     /* size of element (bytes) or -1 if variable */
  int nprops;                   /* number of properties for this element */
  ply_property_t **props;          /* list of properties in the file */
  char *store_prop;             /* flags: property wanted by user? */
  int other_offset;             /* offset to un-asked-for props, or -1 if none*/
  int other_size;               /* size of other_props structure */
} ply_element_t;

typedef struct {   /* describes other properties in an element */
  char *name;                   /* element name */
  int size;                     /* size of other_props */
  int nprops;                   /* number of properties in other_props */
  ply_property_t **props;          /* list of properties in other_props */
} ply_other_prop_t;

typedef struct { /* for storing other_props for an other element */
  void *other_props;
} ply_other_data_t;

typedef struct {     /* data for one "other" element */
  char *elem_name;             /* names of other elements */
  int elem_count;              /* count of instances of each element */
  ply_other_data_t **other_data;      /* actual property data for the elements */
  ply_other_prop_t *other_props;   /* description of the property data */
} ply_other_elem_t;

typedef struct {  /* "other" elements, not interpreted by user */
  int num_elems;                /* number of other elements */
  ply_other_elem_t *other_list;        /* list of data for other elements */
} ply_other_elems_t;

#define AVERAGE_RULE  1
#define MAJORITY_RULE 2
#define MINIMUM_RULE  3
#define MAXIMUM_RULE  4
#define SAME_RULE     5
#define RANDOM_RULE   6

typedef struct {   /* rules for combining "other" properties */
  ply_element_t *elem;      /* element whose rules we are making */
  int *rule_list;        /* types of rules (AVERAGE_PLY, MAJORITY_PLY, etc.) */
  int nprops;            /* number of properties we're combining so far */
  int max_props;         /* maximum number of properties we have room for now */
  void **props;          /* list of properties we're combining */
  float *weights;        /* list of weights of the properties */
} ply_prop_rules_t;

typedef struct ply_rule_node {
  char *name;                  /* name of the rule */
  char *element;               /* name of element that rule applies to */
  char *property;              /* name of property that rule applies to */
  struct ply_rule_node *next;    /* pointer for linked list of rules */
} ply_rule_list_t;

typedef struct {        /* description of PLY file */
  FILE *fp;                     /* file pointer */
  int file_type;                /* ascii or binary */
  float version;                /* version number of file */
  int num_elem_types;           /* number of element types of object */
  ply_element_t **elems;           /* list of elements */
  int num_comments;             /* number of comments */
  char **comments;              /* list of comments */
  int num_obj_info;             /* number of items of object information */
  char **obj_info;              /* list of object info items */
  ply_element_t *which_elem;       /* element we're currently reading or writing */
  ply_other_elems_t *other_elems;   /* "other" elements from a PLY file */
  ply_prop_rules_t *current_rules;  /* current propagation rules */
  ply_rule_list_t *rule_list;       /* rule list from user */
} ply_file_t;

/*** delcaration of routines ***/

ply_other_elems_t *ply_get_other_element (ply_file_t *);

ply_file_t *ply_read(FILE *);
ply_file_t *ply_write(FILE *, int, char **, int);
extern ply_file_t *ply_open_for_writing(char *, int, char **, int);
void ply_close(ply_file_t *);
void ply_free(ply_file_t *);

void ply_get_info(ply_file_t *, float *, int *);
void ply_free_other_elements (ply_other_elems_t *);

void ply_append_comment(ply_file_t *, char *);
void ply_append_obj_info(ply_file_t *, char *);
void ply_copy_comments(ply_file_t *, ply_file_t *);
void ply_copy_obj_info(ply_file_t *, ply_file_t *);
char **ply_get_comments(ply_file_t *, int *);
char **ply_get_obj_info(ply_file_t *, int *);

char **ply_get_element_list(ply_file_t *, int *);
void ply_setup_property(ply_file_t *, const ply_property_t *);
void ply_get_element (ply_file_t *, void *);
char *ply_setup_element_read (ply_file_t *, int, int *);
ply_other_prop_t *ply_get_other_properties(ply_file_t *, int);

void ply_element_count(ply_file_t *, char *, int);
void ply_describe_element(ply_file_t *, char *, int);
void ply_describe_property(ply_file_t *, ply_property_t *);
void ply_describe_other_properties(ply_file_t *, ply_other_prop_t *, int);
void ply_describe_other_elements ( ply_file_t *, ply_other_elems_t *);
void ply_get_element_setup(ply_file_t *, char *, int, ply_property_t *);
ply_property_t **ply_get_element_description(ply_file_t *, char *, int*, int*);
void ply_element_layout(ply_file_t *, char *, int, int, ply_property_t *);

void ply_header_complete(ply_file_t *);
void ply_element_setup(ply_file_t *, char *);
void ply_put_element(ply_file_t *, void *);
void ply_put_other_elements(ply_file_t *);

ply_prop_rules_t *ply_init_rule (ply_file_t *, char *);
void ply_modify_rule (ply_prop_rules_t *, char *, int);
void ply_start_props (ply_file_t *, ply_prop_rules_t *);
void ply_weight_props (ply_file_t *, float, void *);
void *ply_get_new_props(ply_file_t *);
void ply_set_prop_rules (ply_file_t *, ply_rule_list_t *);
ply_rule_list_t *ply_append_prop_rule (ply_rule_list_t *, char *, char *);
int ply_matches_rule_name (char *);

#ifdef __cplusplus
}
#endif
#endif /* !_PLY_H_ */
