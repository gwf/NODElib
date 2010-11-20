/* Copyright (c) 1995 by G. W. Flake. */

#include <string.h>
#include <memory.h>
#include "nodelib/array.h"

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void __array_insert_index(ARRAY *a, unsigned index)
{
  unsigned asz, l, m, r;

  asz = array_size(a);

  if(index > asz) {
    ulog(ULOG_ERROR, "array_insert_index: bad index (%d > %d).",
	 index, asz);
  }
  else if(index == 0) array_prepend_ptr(a, a->hole, 1);
  else if(index == asz) array_append_ptr(a, a->hole, 1);
  else {
    if(asz == a->alloced) __array_grow_internal(a);
    l = (a->alloced + a->origin - a->lindex) % a->alloced;
    m = (a->alloced + a->origin - a->lindex + index) % a->alloced;
    r = (a->alloced + a->origin - a->lindex + asz - 1) % a->alloced;
    if((l < r && m - l < r - m && l > 0) ||
       (l < r && m - l > r - m && r == a->alloced - 1) ||
       (l > r && m > r)) {
      memmove(a->data + (l - 1) * a->elemsz, a->data + l * a->elemsz,
	      (m - l) * a->elemsz);
      memmove(a->data + (m - 1) * a->elemsz, a->hole, a->elemsz);
      a->lindex++;
    }
    else {
      memmove(a->data + (m + 1) * a->elemsz, a->data + m * a->elemsz,
	      (r - m + 1) * a->elemsz);
      memmove(a->data + m * a->elemsz, a->hole, a->elemsz);
      a->rindex++;
    }
  }
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void array_destroy_index(ARRAY *a, unsigned index)
{
  unsigned asz, l, m, r;

  asz = array_size(a);

  if(index >= asz) {
    if(asz == 0)
      ulog(ULOG_ERROR, "array_destroy_index: array is empty.");
    else
      ulog(ULOG_ERROR, "array_destroy_index: bad index %d not in [0 .. %d].",
	   index, asz - 1);
  }
  /*
   * Special simple case: destroy first element.
   */
  else if(index == 0 && a->lindex > 0)
    a->lindex--;
  /*
   * Special simple case: destroy last element.
   */
  else if(index == asz - 1 && a->rindex > 0)
    a->rindex--;
  /*
   * If the ARRAY does not wrap, then copy the smallest possible segment.
   * Otherwise, copy the easiest wrapping segment.
   */
  else {
    l = (a->alloced + a->origin - a->lindex) % a->alloced;
    m = (a->alloced + a->origin - a->lindex + index) % a->alloced;
    r = (a->alloced + a->origin - a->lindex + asz - 1) % a->alloced;
    if((l < r && m - l < r - m) || (l > r && m > r)) {
      memmove(a->data + (l + 1) * a->elemsz, a->data + l * a->elemsz,
	      (m - l) * a->elemsz);
      if(a->lindex > 0)
	a->lindex--;
      else {
	a->rindex--;
	a->origin = (a->origin + 1) % a->alloced;
      }
    }
    else {
      memmove(a->data + m * a->elemsz, a->data + (m + 1) * a->elemsz,
	      (r - m) * a->elemsz);
      if(a->rindex > 0)
	a->rindex--;
      else {
	a->lindex--;
	a->origin = (a->origin - 1 + a->alloced) % a->alloced;
      }
    }
  } 
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* Simply create a new ARRAY instance. */

ARRAY *__array_create_internal(size_t sz, size_t typesize)
{
  ARRAY *array;
  
  array = xmalloc(sizeof(ARRAY));
  array->lindex = array->rindex = array->origin = 0;
  array->data = xmalloc(typesize * sz);
  array->hole = xmalloc(typesize);
  array->alloced = sz;
  array->elemsz = typesize;
  return(array);
}    

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void array_destroy(ARRAY *a)
{
  xfree(a->data);
  xfree(a->hole);
  xfree(a);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* Double the data size of the ARRAY as needed. */

void __array_grow_internal(ARRAY *a)
{
  size_t sz;
  char *src, *dest;
  
  a->data = xrealloc(a->data, a->alloced * a->elemsz * 2);

  /* If the left-hand side wraps around to the right-hand side
   * then we need to copy the data that is wrapped back to the new
   * right-most edge of the newly enlarged data.
   */
  if(a->origin < a->lindex) {
    sz = a->elemsz * (a->lindex - a->origin);
    src = a->data + a->elemsz * (a->alloced + a->origin - a->lindex);
    dest = src + a->elemsz * a->alloced;
    memcpy(dest, src, sz);
  }
  /* If the right-hand-side wraps around to the left-hand-side
   * then we need to copy the data that is wrapped back to the
   * right-hand-side so that it is no longer wrapping.
   */
  else if(a->origin + a->rindex > a->alloced) {
    sz = a->elemsz * (a->origin + a->rindex - a->alloced);
    src = a->data;
    dest = src + a->elemsz * a->alloced;
    memcpy(dest, src, sz);
  }

  a->alloced *= 2;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void *array_to_pointer(ARRAY *a)
{
  size_t sz;
  char *src, *dest, *result;

  dest = result = xcalloc(array_size(a) * a->elemsz + 1, 1);
  
  /* If we wrap from the left to the right copy the first
   * chunk that is wrapped to the right, then copy the last
   * chunk which is unwrapped.
   */
  if(a->origin < a->lindex) {
    sz = a->elemsz * (a->lindex - a->origin);
    src = a->data + a->elemsz * (a->alloced + a->origin - a->lindex);
    memcpy(dest, src, sz);
    dest += sz;
    src = a->data;
    sz = a->elemsz * (a->rindex + a->origin);
    memcpy(dest, src, sz);
  }
  /* If we wrap from the right to the left copy the first chunk
   * that is unwrapped, then copy the last chumnk that is wrapped.
   */
  else if(a->origin + a->rindex > a->alloced) {
    sz = a->elemsz * (a->alloced + a->lindex - a->origin);
    src = a->data + a->elemsz * (a->origin - a->lindex);
    memcpy(dest, src, sz);
    dest += sz;
    sz = a->elemsz * (a->origin + a->rindex - a->alloced);
    src = a->data;
    memcpy(dest, src, sz);
  }
  /* We aren't wrapped at all so just copy everything in one chunk.
   */
  else {
    sz = a->elemsz * (a->lindex + a->rindex);
    src = a->data + a->elemsz * (a->origin - a->lindex);
    memcpy(dest, src, sz);
  }
  return(result);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void array_append_ptr(ARRAY *a, char *ptr, size_t sz)
{
  unsigned r;
  size_t cnt;
  char *src, *dest;

  /* We do not have enough memory. */
  while(sz > (a->alloced - array_size(a)))
    __array_grow_internal(a);

  /* We have enough space. Compute the index of the last element. */
  r = (a->alloced + a->origin + a->rindex) % a->alloced;
  
  if(r < a->origin || (a->alloced - r >= sz)) {
    /* There must be a big enough chunk at a[r ..]. */
    dest = a->data + r * a->elemsz;
    memcpy(dest, ptr, sz * a->elemsz);
  }
  else {
    /* There is only enough space if we wrap around. */
    dest = a->data + a->elemsz * r;
    cnt = a->elemsz * (a->alloced - r);
    memcpy(dest, ptr, cnt);
    src = ptr + a->elemsz * (a->alloced - r);
    dest = a->data;
    cnt = a->elemsz * (sz - (a->alloced - r));
    memcpy(dest, src, cnt);
  }
  a->rindex += sz;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void array_prepend_ptr(ARRAY *a, char *ptr, size_t sz)
{
  unsigned l;
  size_t cnt;
  char *src, *dest;

  /* We do not have enough memory. */
  while(sz > (a->alloced - array_size(a)))
    __array_grow_internal(a);

  /* We have enough space. Compute the index of the first element. */
  l = (a->alloced + a->origin - a->lindex) % a->alloced;

  if(l > a->origin || l >= sz) {
    /* There must be a big enough chunk at a[.. l]. */
    dest = a->data + a->elemsz * (l - sz);
    memcpy(dest, ptr, sz * a->elemsz);
  }
  else {
    /* There is only enough space if we wrap around. */
    cnt = l * a->elemsz;
    memcpy(a->data, ptr, cnt);
    src = ptr + cnt;
    cnt = (sz - l) * a->elemsz;
    dest = a->data + a->elemsz * (a->alloced - cnt);
    memcpy(dest, src, cnt);
  }
  a->lindex += sz;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
