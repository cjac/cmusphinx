
#ifndef _LIBUTIL_PROPERTIES_H
#define _LIBUTIL_PROPERTIES_H

#define PTYPE_STRING		0x00
#define PTYPE_INT		0x01
#define PTYPE_FLOAT		0x02
#define PTYPE_DOUBLE		0x03
#define PTYPE_UNKNOWN		0x04
#define PTYPE_REQUIRED		0xA0

#define PARSE_NEW_PROPERTY	0x01
#define PARSE_REDEFINITION	0x02
#define PARSE_OVERWRITE		0x04
#define PARSE_STRICT		0x00

#define MAX_PSET_SIZE		1024
#define MAX_WORD_LENGTH		1024

typedef struct {
  /** name of the property */
  char *name;
  /** type of the property */
  int type;
  /** value of the property in string representation */
  char *val;
  /** description of the property */
  char *desc;
} prop_t;

typedef struct {
  hash_table_t *indices;
  prop_t *entries;
  int num_entries;
} propset_t;

int prop_new(propset_t **);
void prop_free(propset_t *);
int prop_parse(propset_t *, int, char **, int);
int prop_parse_file(propset_t *, char **, int);
int prop_set_prop(propset_t *, prop_t *);
int prop_get_prop(propset_t *, char *, prop_t **);
int prop_remove_prop(propset_t *, char *);
int prop_get_type(propset_t *, char *, int *);
int prop_get_str(propset_t *, char *, char **);
int prop_get_int(propset_t *, char *, int *);
int prop_get_float(propset_t *, char *, float *);
int prop_get_double(propset_t *, char *, double *);
void prop_print(propset_t *, FILE *);

#endif
