
#include "properties.h"
#include "stdio.h"

static int
prop_new_entry(char *_name, int _type, char * _val, char *_desc,
	       prop_t **_prop);

static void
prop_free_entry(prop_t *_prop);

int
prop_new(propset_t **_pset)
{
  propset_t *pset = NULL;
  hash_table_t *ht = NULL;
  prop_t *parray = NULL;
  int i;

  assert(_pset != NULL);

  *_pset = NULL;

  /* allocating property set structure */
  if ((pset = (propset_t *)ckd_malloc(sizeof(propset_t))) == NULL) {
    goto prop_create_cleanup;
  }

  /* allocating index hash */
  if ((ht = hash_new(MAX_PSET_SIZE, HASH_CASE_YES)) == NULL) {
    goto prop_create_cleanup;
  }

  /* allocating entry array */
  parray = (prop_t **)ckd_calloc(MAX_PSET_SIZE, sizeof(prop_t *));
  if (parray == NULL) {
    goto prop_create_cleanup;
  }
  for (i = MAX_PSET_SIZE - 1; i >= 0; i--) {
    parray[i] = NULL;
  }

  pset->indices = ht;
  pset->entries = parray;
  pset->num_entries = 0;
  *_pset = pset;

  return 0;

 prop_create_cleanup:
  if (pset != NULL) {
    ckd_free(pset);
  }

  if (ht != NULL) {
    hash_free(ht);
  }
  
  if (parray != NULL) {
    ckd_free(parray);
  }

  return -1;
}

void
prop_free(propset_t *_pset)
{
  int i;

  assert(_pset != NULL);

  if (_pset->indices != NULL) {
    hash_free(_pset->indices);
  }
  if (_pset->entries != NULL && _pset->num_entries > 0) {
    for (i = _pset->num_entries - 1; i >= 0; i--) {
      prop_free_entry(_pset->entries[i]);
    }
    ckd_free(_pset->entries);
  }
  ckd_free(_pset);
}

int
prop_parse(propset_t *_pset, int _argc, char **_argv, int _flags)
{
  int i;
  hash_table_t *parsed_ht = NULL;
  glist_t parsed_list = NULL;
  gnode_t *itr;
  int num_parsed;
  prop_t *orig;
  prop_t *repl;
  char *val;
  int some_number;

  assert(_pset != NULL);
  assert(_argc % 2 == 0);

  if ((parsed_ht = hash_new(DEFAULT_HASH_SIZE, HASH_CASE_YES)) == NULL) {
    goto prop_parse_cleanup;
  }

  /* initial run through the new argument list to check for errors */
  for (i = 0; i < _argc; i += 2) {
    /* if the PARSE_REDEFINITION flag is NOT on, and we've encountered this
     * property before (in this parse session), then it is an error */
    if ((_flags & PARSE_REDEFINITION) == 0) {
      if (hash_lookup(parsed_ht, _argv[i], &some_number) == 0) {
	goto prop_parse_cleanup;
      }
    }

    if (prop_get_prop(_pset, _argv[i], &orig) == 0) {
      /* if the PARSE_OVERWRITE flag is NOT on, the property exists in prior
       * definition, and the property has a value, then it is an error. */
      if (orig->val != NULL && (_flags & PARSE_OVERWRITE) == 0) {
	goto prop_parse_cleanup;
      }
    }
    else {
      /* if the PARSE_NEW_PROPERTY flag is NOT on, and the property does NOT
       * exist in prior definition, then it is an error. */
      if ((_flags & PARSE_NEW_PROPERTY) ==  0) {
	goto prop_parse_cleanup;
      }
    }

    if (orig != NULL) {
      prop_new_entry(_argv[i], orig->type, _argv[i + 1], orig->desc, &repl);
    }
    else {
      prop_new_entry(_argv[i], PTYPE_UNKNOWN, _argv[i + 1], NULL, &repl);
    }
    if (repl == NULL) {
      goto prop_parse_cleanup;
    }
    glist_add_ptr(parsed_list, repl);
    hash_enter(parsed_ht, _argv[i], /* some meaningless number */ 12345);
  }

  /* insert/replace all the gathered properties.  failure to insert/replace
   * gathered property can cause inconsistent property sets!!! */
  for (itr = parsed_list; itr != NULL; itr = gnode_next(itr)) {
    if (prop_set_prop(_pset, (prop_t *)gnode_ptr(itr)) == -1) {
      goto prop_parse_cleanup;
    }
  }

  if (parsed_ht != NULL) {
    hash_free(parsed_ht);
    parsed_ht = NULL;
  }

  if (parsed_list != NULL) {
    glist_free(parsed_list);
    parsed_list = NULL;
  }

  return 0;

 prop_parse_cleanup:
  if (parsed_ht != NULL) {
    hash_free(parsed_ht);
  }

  if (parsed_list != NULL) {
    for (itr = parsed_list; itr != NULL; itr = gnode_next(itr)) {
      prop_free_entry((prop_t *)gnode_ptr(itr));
    }
    glist_free(parsed_list);
  }

  return -1;
}

#define WHITE_SPACE " \t\r\n"

int
prop_parse_file(propset_t *_pset, char **_filename, int _flags)
{
  char word[MAX_WORD_LENGTH];
  int wordlen;
  char **argv = NULL;
  int argc = 0;
  int index = 0;
  FILE *fd = NULL;
  int ch;
  
  assert(_pset != NULL);
  assert(_filename != NULL);
  
  if ((fd = fopen(_filename, "r")) == NULL) {
    return -1;
  }

  /* count the number of words in the file (and make sure we have an even
   * number) */
  do {
    ch = fgetc(fd);
    if (strchr(ch, WHITE_SPACE) != NULL) {
      argc++;
    }
  } while (ch != EOF);
  if ((argc % 2) != 0) {
    goto prop_parse_file_cleanup;
  }

  /* allocate space for the argv array */
  if ((argv = (char **)ckd_calloc(argc, sizeof(char *))) == NULL) {
    goto prop_parse_file_cleanup;
  }

  /* insert each word into the argument array */
  index = 0;
  wordlen = 0;
  rewind(fd);
  do {
    ch = fgetc(fd);
    if (strchr(ch, WHITE_SPACE) != NULL && index < argc) {
      if ((argv[index] = (char *)ckd_malloc(wordlen + 1)) == NULL) {
	goto prop_parse_file_cleanup;
      }
      strcpy(argv[index++], word);
      wordlen = 0;
    }
    else if (wordlen < MAX_WORD_LENGTH) {
      word[wordlen++] = (char)ch;
    }
  } while (ch != EOF);

  /* use prop_parse() to enter the argument array into _pset */
  if (prop_parse(_pset, index, argv, _flags) == -1) {
    goto prop_parse_file_cleanup;
  }

  /* free the argument array and close fd */
  if (argv != NULL) {
    for (index = argc - 1; index >= 0; index--) {
      if (argv[index] != NULL) {
	ckd_free(argv[index]);
      }
    }
    ckd_free(argv);
  }
  fclose(fd);

  return 0;

 prop_parse_file_cleanup:
  if (argv != NULL) {
    for (index = argc - 1; index >= 0; index--) {
      if (argv[index] != NULL) {
	ckd_free(argv[index]);
      }
    }
    ckd_free(argv);
  }
  fclose(fd);

  return -1;
}

#undef WHITE_SPACE

int
prop_set_prop(propset_t *_pset, prop_t *_prop)
{
  int index;
  prop_t *orig = NULL;

  assert(_pset != NULL);
  assert(_prop != NULL);

  if (hash_lookup(_pset->indices, _prop->name, &index) == 0) {
    orig = _pset->entries[index];
    _pset->entries[index] = _prop;
    prop_free_entry(orig);
  }
  else if (_pset->num_entries < MAX_PSET_SIZE) {
    _pset->entries[_pset->num_entries++] = _prop;
  }
  else {
    return -1;
  }

  return 0;
}

int
prop_get_prop(propset_t *_pset, char *_name, prop_t **_prop)
{
  int index;

  assert(_pset != NULL);
  assert(_name != NULL);
  assert(_prop != NULL);

  if (hash_lookup(_pset->indices, _name, &index) == 0) {
    *_prop = _pset->entries[index];
  }
  else {
    *_prop = NULL;
    return -1;
  }

  return 0;
}

int
prop_remove_prop(propset_t *_pset, char *_name)
{
  int index;
  prop_t *prop = NULL;
  prop_t *shift;

  assert(_pset != NULL);
  assert(_name != NULL);

  if (hash_lookup(_pset->indices, _name, &index) == 0) {
    if (index == _pset->num_entries) {
      prop = _pset->entries[_pset->num_entries--];
    }
    else {
      prop = _pset->entries[index];
      shift = _pset->entries[_pset->num_entries--];
      hash_enter(_pset->indices, shift->name, index);
      _pset->entries[index] = shift;
    }
  }

  if (prop != NULL) {
    prop_free_entry(prop);
    return 0;
  }

  return -1;
}

int
prop_get_type(propset_t *_pset, char *_name, int *_type)
{
  int index;
  prop_t *prop = NULL;

  assert(_pset != NULL);
  assert(_name != NULL);
  assert(_type != NULL);

  if (hash_lookup(_pset->indices, _name, &index) == 0) {
    prop = _pset->entries[index];
    if (prop != NULL) {
      _type = prop->type;
      return 0;
    }
  }

  return -1;
}

int
prop_get_str(propset_t *_pset, char *_name, char **_val)
{
  int index;
  prop_t *prop = NULL;

  assert(_pset != NULL);
  assert(_name != NULL);
  assert(_val != NULL);

  if (hash_lookup(_pset->indices, _name, &index) == 0) {
    prop = _pset->entries[index];
    if (prop != NULL) {
      *_val = prop->val;
      return 0;
    }
  }

  return -1;
}

int
prop_get_int(propset_t *_pset, char *_name, int *_val)
{
  int index;
  prop_t *prop = NULL;
  int val;

  assert(_pset != NULL);
  assert(_name != NULL);
  assert(_val != NULL);

  if (hash_lookup(_pset->indices, _name, &index) == 0) {
    prop = _pset->entries[index];
    if (prop != NULL) {
      if (prop->type == PTYPE_INT) {
	if (sscanf(prop->val, "%d", &val) != 0) {
	  *_val = val;
	  return 0;
	}
      }
    }
  }

  return -1;
}

int
prop_get_float(propset_t *_pset, char *_name, float *_val)
{
  int index;
  prop_t *prop = NULL;
  float val;

  assert(_pset != NULL);
  assert(_name != NULL);
  assert(_val != NULL);

  if (hash_lookup(_pset->indices, _name, &index) == 0) {
    prop = _pset->entries[index];
    if (prop != NULL) {
      if (prop->type == PTYPE_FLOAT) {
	if (sscanf(prop->val, "%f", &val) != 0) {
	  *_val = val;
	  return 0;
	}
      }
    }
  }

  return -1;
}

int
prop_get_double(propset_t *_pset, char *_name, double *_val)
{
  int index;
  prop_t *prop = NULL;
  double val;

  assert(_pset != NULL);
  assert(_name != NULL);
  assert(_val != NULL);

  if (hash_lookup(_pset->indices, _name, &index) == 0) {
    prop = _pset->entries[index];
    if (prop != NULL) {
      if (prop->type == PTYPE_DOUBLE) {
	if (sscanf(prop->val, "%lf", &val) != 0) {
	  *_val = val;
	  return 0;
	}
      }
    }
  }

  return -1;
}

void
prop_print(propset_t *_pset, FILE *_file)
{
}

static int
prop_new_entry(char *_name, int _type, char *_val, char *_desc, prop_t **_prop)
{
  int type_bits;
  prop_t *prop = NULL;

  assert(_name != NULL);
  assert(_prop != NULL);
  
  type_bits = _type & ~PTYPE_REQUIRED;
  if (type_bits != PTYPE_STRING &&
      type_bits != PTYPE_INT &&
      type_bits != PTYPE_FLOAT &&
      type_bits != PTYPE_DOUBLE &&
      type_bits != PTYPE_UNKNOWN) {
    return -1;
  }

  if ((prop = (prop_t *)ckd_malloc(sizeof(prop_t))) == NULL ||
      (prop->name = (char *)ckd_malloc(strlen(_name) + 1)) == NULL ||
      (prop->val = (char *)ckd_malloc(strlen(_val) + 1)) == NULL ||
      (prop->desc = (char *)ckd_malloc(strlen(_desc) + 1)) == NULL) {
    goto prop_new_entry_cleanup;
  }
  strcpy(prop->name, _name);
  strcpy(prop->val, _val);
  strcpy(prop->desc, _desc);
  prop->type = _type;
  *_prop = prop;
  
  return 0;

 prop_new_entry_cleanup:
  if (prop != NULL) {
    if (prop->name != NULL) {
      ckd_free(prop->name);
    }
    if (prop->val != NULL) {
      ckd_free(prop->val);
    }
    if (prop->desc != NULL) {
      ckd_free(prop->desc);
    }
    ckd_free(prop);
  }

  return -1;
}

static void
prop_free_entry(prop_t *_prop)
{
  assert(_prop != NULL);

  if (_prop->name != NULL) {
    ckd_free(_prop->name);
  }
  if (_prop->val != NULL) {
    ckd_free(_prop->val);
  }
  if (_prop->desc != NULL) {
    ckd_free(_prop->desc);
  }
  ckd_free(_prop);
}
