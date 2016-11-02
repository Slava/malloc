#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "./allocator_interface.h"
#include "./memlib.h"
#include "./utilfast.h"
#include "./sampler.c"

#define malloc(...) (USE_MY_MALLOC)
#define free(...) (USE_MY_FREE)
#define realloc(...) (USE_MY_REALLOC)

#ifndef NUM_FREE_LISTS
#define NUM_FREE_LISTS 10
#endif

list_t *free_lists[NUM_FREE_LISTS];

const int NUM_THRES = 5;
const int NUM_BKT_DIV = NUM_FREE_LISTS / NUM_THRES;
const size_t THRES[] = {250, 1000, 4000, 16000, 1 << 30};

void *heap_lo, *heap_hi;

static inline int get_bkt_idx(list_t *p) {
  size_t a = p->size;
  size_t b = (size_t) p;
  b = (b >> 6) ^ (b >> 3) ^ b; // "uniformly" hash the address
  int idx = 0;
  while (idx + 1 < NUM_THRES && a >= THRES[idx]) {
    ++idx;
  }

  return idx * NUM_BKT_DIV + (b % NUM_BKT_DIV);
}

int my_check() {
  char *p;
  char *lo = (char*)mem_heap_lo();
  char *hi = (char*)mem_heap_hi() + 1;
  size_t size = 0;

  p = lo;
  while (lo <= p && p < hi) {
    size = ALIGN(*(size_t*)p + SIZE_T_SIZE);
    p += size;
  }

  if (p != hi) {
    printf("Bad headers did not end at heap_hi!\n");
    printf("heap_lo: %p, heap_hi: %p, size: %lu, p: %p\n", lo, hi, size, p);
    return -1;
  }

  return 0;
}

void sample_finished(int *bins, int size) {
  // TODO do something smart;
  printf("sampling finished %d\n", size);
}

int my_init() {
  heap_lo = mem_heap_lo();
  heap_hi = mem_heap_hi() + 1;
  D(id_cnt=0);
  dprintf("\n\n--------------------\nINIT\n");
  for (size_t i = 0; i < NUM_FREE_LISTS; ++i) {
    free_lists[i] = NULL;
  }

  init_samples();
  register_sampling_cb(&sample_finished);
  return 0;
}

static inline void * my_brk(size_t size) {
  dprintf("\nBRK %zu\n", size);
  list_t *cur, *ret;

  ret = NULL;
  if (size != 456)
  for (size_t _idx = 0; _idx < NUM_THRES; ++_idx) {
    if (THRES[_idx] < size) {
      continue;
    }
    for(size_t idx = _idx * NUM_BKT_DIV; idx < _idx * NUM_BKT_DIV + NUM_BKT_DIV; ++idx) {
      cur = free_lists[idx];
      while (cur) {
        if (cur->size >= size) {
          if (!ret || cur->size < ret->size) {
            ret = cur;
            break;
          }
        }
        cur = cur->next;
      }
      if (ret) {
        break;
      }
    }
    if (ret) break;
  }

  void *p;
  if (!ret) {
    dprintf("mem_sbrking\n");
    p = mem_sbrk(size);
    if (p == (void *) -1) {
      return p;
    }
    heap_hi += size;
    size_t *sz = (size_t *)p;
    *sz = size;
  } else {
    p = (void *)ret;
      size_t *sz = (size_t *)p;
    *sz = ret->size;
    list_erase(&(free_lists[get_bkt_idx(ret)]), ret);
    list_t *rem = slice(ret, size);
    if (rem) {
      *sz -= rem->size;
      list_append(&(free_lists[get_bkt_idx(rem)]), rem);
    }
  }

  return p;
}

void * my_malloc(size_t size) {
  // we need to be able to store SIZE_T + the current block.
  // in addition, because a list_t needs to have its end marked the size also needs to be LIST_T_SIZE + SIZE_T_SIZE for us to be able to eventually free and reuse the space.
  add_sample(size);
  size_t size_needed = max(size + SIZE_T_SIZE, LIST_T_SIZE + SIZE_T_SIZE);
  size_t aligned_size = ALIGN(size_needed);

  void *p = my_brk(aligned_size);
  if (p == (void *)-1) {
    return NULL;
  } else {
    return p + SIZE_T_SIZE;
  }
}

static inline bool in_free_list(list_t *p) {
  list_t *cur = free_lists[get_bkt_idx(p)];
  while (cur) {
    if (cur == p) {
      return true;
    }
    cur = cur->next;
  }
  return false;
}

/*
 * check if the free list contains a block that ends at p_e
 */
static inline bool valid_node_end(void *p_e) {
  if (p_e - SIZE_T_SIZE < heap_lo || p_e > heap_hi) {
    return false;
  }
  size_t size_p = (*(size_t *)(p_e - SIZE_T_SIZE)) ^ SECRET;
  void *p = p_e - size_p;
  if (p < heap_lo || p + LIST_T_SIZE > heap_hi) {
    return false;
  }

  list_t *p_node = (list_t *)p;
  if (p_node->size != size_p) {
    return false;
  }
  if (A) return true; // experiment with probabalistic correctness
  return in_free_list(p_node);
}

/*
 * check if the free list contains p_e
 */
static inline bool valid_node_start(void *p) {
  if (p < heap_lo || p + LIST_T_SIZE > heap_hi) {
    return false;
  }
  list_t *p_node = (list_t *)p;
  size_t size_p = p_node->size;
  void *p_e = p + size_p;

  if (p_e < heap_lo || p_e > heap_hi) {
    return false;
  }
  size_t size_p2 = *(size_t *)(p_e - SIZE_T_SIZE) ^ SECRET;
  if (size_p != size_p2) {
    return false;
  }
  if (A) return true; // experiment with probabalistic correctness
  return in_free_list(p_node);
}

/* before adding a node q to the free list,
 * check if there is a p such that p + q is a single contiguous free block.
 * If there exists such a q, update its size and return true.
 * [.........][......]
 *  p     p_e  q
 * ----> (mark the new size at the new p_e)
 * [.................]
 *  p             p_e
 */
static inline bool coalesce_back(list_t *q_node) {
  void *p, *q;

  q = (void *)q_node;

  if (valid_node_end(q)) { // can we safely coalesce?
    p = q - (*(size_t*)(q - SIZE_T_SIZE) ^ SECRET);
    list_t *p_node = (list_t*)(p);
    list_erase(&(free_lists[get_bkt_idx(p_node)]), p_node);
    grow(p_node, q_node->size);
    list_append(&(free_lists[get_bkt_idx(p_node)]), p_node);
    return true;
  } else {
    return false;
  }
}

/* before adding a node p to the free list,
 * check if there is a q such that p + q is a single contiguous free block.
 * If there exists such a q, update its size and return true.
 * [.........][......]
 *  p     p_e  q
 * ----> (mark the new size at the new p_e)
 * [.................]
 *  p             p_e
 */
static inline bool coalesce_forward(list_t *p_node) {
  void *p, *q;

  p = (void *)p_node;
  q = p + p_node->size;
  if (valid_node_start(q)) { // can we safely coalesce?
    list_t *q_node = (list_t *)q;
    unmark_end(q_node);
    list_erase(&(free_lists[get_bkt_idx(q_node)]), q_node);
    grow(p_node, q_node->size);
    return true;
  } else {
    return false;
  }
}

// free - Freeing a block does nothing.
void my_free(void *p) {
  dprintf("\nFREE\n");
  p -= SIZE_T_SIZE;
  size_t size = *(size_t *)p;

  list_t *p_node = mknode(p, size);

  // try to absorb the one in front
  if (coalesce_forward(p_node)) {
    dprintf("successful coalesce_forward\n");
  } else {
    dprintf("unsuccessful coalesce_forward\n");
  }

  if (coalesce_back(p_node)) {
    dprintf("successful coalesce_back\n");
    return;
  }

  dprintf("unsuccessful coalesce_back\n");
  mark_end(p_node);
  list_append(&(free_lists[get_bkt_idx(p_node)]), p_node);
}

static inline bool fast_realloc(void *p, size_t new_size) {
  p -= SIZE_T_SIZE;
  new_size += SIZE_T_SIZE;
  size_t *p_sz = (size_t *)p;
  size_t old_size = *p_sz;
  size_t size = new_size - old_size;
  dprintf("(%zu) --> (%zu): +%zu\n", old_size, new_size, size);
  if (new_size < old_size) {
    dprintf("Realloc case 0\n");
    return true;
  }

  void *q = p + old_size;
  if (q == heap_hi) {
    dprintf("Realloc case 1\nmem_sbrk %zu\n", size);
    mem_sbrk(size);
    heap_hi += size;
    *p_sz += size;
    return true;
  }

  if (!valid_node_start(q)) {
    return false;
  }

  list_t *q_node = (list_t *)q;
  if (q_node->size >= size) {
    dprintf("Realloc case 2\n");
    *p_sz += q_node->size;
    list_erase(&(free_lists[get_bkt_idx(q_node)]), q_node);
    list_t *rem = slice(q_node, size);
    if (rem) {
      *p_sz -= rem->size;
      list_append(&(free_lists[get_bkt_idx(rem)]), rem);
    }

    return true;
  }

  return false;
}


void * my_realloc(void *ptr, size_t size) {
  dprintf("\nREALLOC\n");
  if (fast_realloc(ptr, size)) {
    return ptr;
  }
  void *newptr;
  size_t copy_size;

  newptr = my_malloc(size);
  if (NULL == newptr)
    return NULL;

  copy_size = *(size_t*)((uint8_t*)ptr - SIZE_T_SIZE);

  if (size < copy_size)
    copy_size = size;

  memcpy(newptr, ptr, copy_size);

  my_free(ptr);

  return newptr;
}

void my_reset_brk() {
  mem_reset_brk();
}

void * my_heap_lo() {
  return mem_heap_lo();
}

void * my_heap_hi() {
  return mem_heap_hi();
}

void my_dump_state() {
  printf("// Printing state\n");
  for (int i = 0; i < NUM_FREE_LISTS; i++) {
    list_t *l = free_lists[i];
    while (l) {
      printf("%zu %zu\n", (size_t)l, l->size);
      l = l->next;
    }
  }
  printf("\n");
}
