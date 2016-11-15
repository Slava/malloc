#ifndef UTIL_H
#define UTIL_H

#ifdef VERBOSE
#define D(x) x
#define dprintf printf
#else
#define D(...)
#define dprintf(...)
#endif

#define max(a, b) (((a) > (b)) ? (a) : (b))
#define min(a, b) (((a) < (b)) ? (a) : (b))

typedef struct list_t {
  size_t size;
  D(size_t id;)
  struct list_t* next;
} list_t;

#define LIST_T_SIZE (sizeof(list_t))


void list_print(list_t **root);
void list_erase(list_t **root, list_t *node);
void list_add(list_t **root, list_t *node);

#endif
