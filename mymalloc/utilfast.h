#ifndef UTILFAST_H
#define UTILFAST_H

#ifdef VERBOSE
#define D(x) x
#define dprintf printf
#else
#define D(...)
#define dprintf(...)
#endif

#define max(a, b) (((a) > (b)) ? (a) : (b))
#define min(a, b) (((a) < (b)) ? (a) : (b))

// A -- experiment flag
#ifndef EXP
//#define A 0
#define A 1 // turn on experimental for handin (Slava)
#else
#define A 1
#endif

#ifndef ALIGNMENT
#define ALIGNMENT 8
#endif


#define SECRET 0x123456789ABCDEF0
#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~(ALIGNMENT-1))
#if (__WORDSIZE == 64 )
#define IS_ALIGNED(p)  ((((uint64_t)(p)) % ALIGNMENT) == 0)
#else
#define IS_ALIGNED(p)  ((((uint32_t)(p)) % ALIGNMENT) == 0)
#endif


typedef struct list_t {
  size_t size;
  D(size_t id;)
  struct list_t* next;
  struct list_t* prev;
} list_t;

#define LIST_T_SIZE (sizeof(list_t))


void list_print(list_t **root);
void list_erase(list_t **root, list_t *node);
void list_append(list_t **root, list_t *node);
list_t* mknode(void *p, size_t size);
list_t * slice(list_t *node, size_t size);
void grow(list_t *node, size_t size);
void mark_end(list_t *p);
void unmark_end(list_t *p);

#endif
