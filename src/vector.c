#include <vector.h>

void vector_init(vector *v)
{
  v->pf_vector_total = vector_total;
  v->pf_vector_resize = vector_resize;
  v->pf_vector_add = vector_push_back;
  v->pf_vector_set = vector_set;
  v->pf_vector_get = vector_get;
  v->pf_vector_get_size = vector_get_size;
  v->pf_vector_free = vector_free;
  v->pf_vector_delete = vector_delete;

  v->vector_list.capacity = VECTOR_INIT_CAPACITY;
  v->vector_list.total = 0;
  v->vector_list.items = (vector_item *)malloc(sizeof(vector_item) * v->vector_list.capacity);
}

size_t vector_total(vector *v)
{
  int total_count = UNDEFINE;
  if (v)
  {
    total_count = v->vector_list.total;
  }
  return total_count;
}

int vector_resize(vector *v, size_t capacity)
{
  int status = UNDEFINE;
  if (v)
  {
    vector_item *items = (vector_item *)realloc(v->vector_list.items, sizeof(vector_item *) * capacity);
    if (items)
    {
      v->vector_list.items = items;
      v->vector_list.capacity = capacity;
      status = SUCCESS;
    }
  }
  return status;
}

int vector_push_back(vector *v, const void *item, size_t size)
{
  int status = UNDEFINE;
  if (v)
  {
    if (v->vector_list.capacity == v->vector_list.total)
    {
      status = vector_resize(v, v->vector_list.capacity * 2);
      if (status != UNDEFINE)
      {
        void *new_item = malloc(size);
        memcpy(new_item, item, size);
        v->vector_list.items[v->vector_list.total].data = new_item;
        v->vector_list.items[v->vector_list.total].size = size;
        v->vector_list.total++;
      }
    }
    else
    {
      void *new_item = malloc(size);
      memcpy(new_item, item, size);
      v->vector_list.items[v->vector_list.total].data = new_item;
      v->vector_list.items[v->vector_list.total].size = size;
      v->vector_list.total++;
      status = SUCCESS;
    }
  }
  return status;
}

int vector_set(vector *v, size_t index, const void *item, size_t size)
{
  int status = UNDEFINE;
  if (v)
  {
    if (index < v->vector_list.total)
    {
      free(v->vector_list.items[index].data);

      void *new_item = malloc(size);
      memcpy(new_item, item, size);

      v->vector_list.items[index].data = new_item;
      v->vector_list.items[index].size = size;
      status = SUCCESS;
    }
  }
  return status;
}

void *vector_get(vector *v, size_t index)
{
  void *read_data = NULL;
  if (v)
  {
    if (index < v->vector_list.total)
    {
      read_data = v->vector_list.items[index].data;
    }
  }
  return read_data;
}

size_t vector_get_size(vector *v, size_t index)
{
  size_t size = 0;
  if (v)
  {
    if (index < v->vector_list.total)
    {
      size = v->vector_list.items[index].size;
    }
  }
  return size;
}
int vector_delete(vector *v, size_t index)
{
  int status = UNDEFINE;
  size_t i = 0;
  if (v)
  {
    if (index >= v->vector_list.total)
      return status;

    free(v->vector_list.items[index].data);
    v->vector_list.items[index].data = NULL;
    v->vector_list.items[index].size = 0;

    for (i = index; i < v->vector_list.total - 1; ++i)
    {
      v->vector_list.items[i].data = v->vector_list.items[i + 1].data;
      v->vector_list.items[i].size = v->vector_list.items[i + 1].size;
    }

    v->vector_list.items[v->vector_list.total - 1].data = NULL;
    v->vector_list.items[v->vector_list.total - 1].size = 0;

    v->vector_list.total--;

    if ((v->vector_list.total > 0) && ((v->vector_list.total) == (v->vector_list.capacity / 4)))
    {
      vector_resize(v, v->vector_list.capacity / 2);
    }
    status = SUCCESS;
  }
  return status;
}

int vector_free(vector *v)
{
  int status = UNDEFINE;
  if (v)
  {
    free(v->vector_list.items);
    v->vector_list.items = NULL;
    status = SUCCESS;
  }
  return status;
}
