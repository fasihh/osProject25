#ifndef VECTOR_H
#define VECTOR_H

#include <stdlib.h>
#include <string.h>

#define VECTOR_INIT_CAPACITY 6
#define UNDEFINE -1
#define SUCCESS 0

#define VECTOR_DEFINE(name) struct s_vector name

typedef struct vector_item
{
  void *data;
  size_t size;
} vector_item;

typedef struct s_vector_list
{
  vector_item *items;
  size_t capacity;
  size_t total;
} s_vector_list;

typedef struct s_vector vector;

struct s_vector
{
  s_vector_list vector_list;
  size_t (*pf_vector_total)(vector *);
  int (*pf_vector_resize)(vector *, size_t);
  int (*pf_vector_add)(vector *, const void *, size_t);
  int (*pf_vector_set)(vector *, size_t, const void *, size_t);
  void *(*pf_vector_get)(vector *, size_t);
  size_t (*pf_vector_get_size)(vector *, size_t);
  int (*pf_vector_delete)(vector *, size_t);
  int (*pf_vector_free)(vector *);
};

void vector_init(vector *v);
size_t vector_total(vector *v);
int vector_resize(vector *v, size_t capacity);
int vector_push_back(vector *v, const void *item, size_t size);
int vector_set(vector *v, size_t index, const void *item, size_t size);
void *vector_get(vector *v, size_t index);
size_t vector_get_size(vector *v, size_t index);
int vector_delete(vector *v, size_t index);
int vector_free(vector *v);

#endif
