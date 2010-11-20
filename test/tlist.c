
/* Copyright (c) 1998 by G. W. Flake. */

/* A test for the linked list routines */

#include <nodelib.h>
#include <stdio.h>

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

int main(int argc, char **argv)
{
  int seed = 0, n = 100, count = 1000, verbose = 0;
  double p = 0.7;
  OPTION opts[] = {
    { "-seed",    OPT_INT,    &seed,    "random number seed" },
    { "-p",       OPT_DOUBLE, &p,       "probability of insertion" },
    { "-n",       OPT_INT,    &n,       "number of integers" },
    { "-count",   OPT_INT,    &count,   "number of operations to test" },
    { "-verbose", OPT_SWITCH, &verbose, "verbose output?" },
    { NULL,       OPT_NULL,   NULL,     NULL }
  };
  int i, j, k, act1, act2, *nums, *ip;
  ARRAY *a;
  LIST *l;
  LIST_NODE *node;
  
  get_options(argc, argv, opts, NULL, NULL, 0);
  srandom(seed);
  
  nums = xmalloc(sizeof(int) * n);
  for (i = 0; i < n; i++) nums[i] = i;
  
  a = array_create(10, int *);
  l = list_create();

  for (i = 0; i < count; i++) {
    act1 = 0;
    if (random_range(0, 1) < p) act1 = 1;
    if (act1 == 0 && l->count == 0) act1 = 1;
    
    if (act1 == 1) { /* insert */
      ip = &nums[random() % n];
      act2 = random() % 4;      
      if (act2 == 0) { /* front */
	if (verbose) fprintf(stderr, "insert front: %d\n", *ip);
	list_insert_head(l, list_node_create(ip));
	array_prepend(a, ip, int *);
      }
      else if (act2 == 1) { /* back */
	if (verbose) fprintf(stderr, "insert back: %d\n", *ip);
	list_insert_tail(l, list_node_create(ip));
	array_append(a, ip, int *);
      }
      else if (act2 == 2) { /* before */
	j = (l->count == 0) ? 0 : random() % l->count;
	if (verbose) fprintf(stderr, "insert before: %d, %d\n", *ip, j);
	for (node = l->head, k = 0; k < j; node = node->next, k++)
	  ;
	list_insert_before(l, list_node_create(ip), node);
	array_insert_index(a, ip, int *, j);
      }
      else { /* after */
	j = (l->count == 0) ? 0 : random() % l->count;
	if (verbose) fprintf(stderr, "insert after: %d, %d\n", *ip, j);
	for (node = l->head, k = 0; k < j; node = node->next, k++)
	  ;
	if(node)
	  list_insert_after(l, list_node_create(ip), node);
	else
	  list_insert_head(l, list_node_create(ip));
	if(node)
	  array_insert_index(a, ip, int *, j + 1);
	else
	  array_insert_index(a, ip, int *, j);
      }
    }
    else { /* delete */
      act2 = random() % 3;
      if (act2 == 0) { /* front */
	if (verbose) fprintf(stderr, "delete front: \n");
	list_node_destroy(list_remove_head(l));
	array_remove_front(a, int *);
      }
      else if (act2 == 1) { /* back */
	if (verbose) fprintf(stderr, "delete back: \n");
	list_node_destroy(list_remove_tail(l));
	array_remove_back(a, int *);
      }
      else { /* middle */
	j = (l->count == 0) ? 0 : random() % l->count;
	if (verbose) fprintf(stderr, "delete middle: %d\n", j);
	for (node = l->head, k = 0; k < j; node = node->next, k++)
	  ;
	list_remove_node(l, node);
	list_node_destroy(node);
	(void)array_remove_index(a, int *, j);
      }
      
    }
  }

  for (node = l->head; node != NULL; node = node->next)
    printf("l %d\n", *(int *)node->data);
  k = array_size(a);
  for (i = 0; i < k; i++) {
    printf("a %d\n", *array_fast_access(a, i, int *));
  }

  list_destroy_all(l);
  array_destroy(a);

  exit(0);    
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

