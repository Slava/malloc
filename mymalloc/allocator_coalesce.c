#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "./allocator_interface.h"
#include "./memlib.h"
#include "./util.h"

#define malloc(...) (USE_MY_MALLOC)
#define free(...) (USE_MY_FREE)
#define realloc(...) (USE_MY_REALLOC)

#ifndef ALIGNMENT
#define ALIGNMENT 8
#endif

#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~(ALIGNMENT-1))

#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))
list_t *free_list;

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

int my_init() {
  D(id_cnt=0);
  dprintf("init.\n--------------------\n\n");
  free_list = NULL;
  return 0;
}

/*
 * update the last SIZE_T_SIZE bytes of p's free block to hold the size
 * requires that p is at least 2 * SIZE_T_SIZE wide.
 */
void mark_end(list_t *p) {
  *(size_t *)((void *)p + p->size - SIZE_T_SIZE) = p->size;
}

void * my_brk(size_t size) {
  dprintf("my_brk %zu\n", size);
  list_t *cur, *ret;
  cur = free_list;
  ret = NULL;
  while (cur) {
    //printf("size: %zu\n", cur->size);
    if (cur->size >= size) {
      if (!ret || cur->size < ret->size) {
        ret = cur;
      }
    }
    cur = cur->next;
  }

  void *p;
  if (!ret) {
    dprintf("mem_sbrking\n");
    p = mem_sbrk(size);
  } else {
    dprintf("reusing %zu\n", ret->size);
    p = (void*) ret;
    list_erase(&free_list, ret);
    if (ret->size >= size + LIST_T_SIZE + SIZE_T_SIZE) {
      list_t *node = (list_t *)(p + size);
      node->size = ret->size - size;
      mark_end(node);
      list_append(&free_list, node);
    }
  }
  return p;
}

void * my_malloc(size_t size) {
  // we need to be able to store SIZE_T + the current block.
  // in addition, because a list_t needs to have its end marked the size also needs to be LIST_T_SIZE + SIZE_T_SIZE for us to be able to eventually free and reuse the space.
  size_t size_needed = max(size + SIZE_T_SIZE, LIST_T_SIZE + SIZE_T_SIZE);
  size_t aligned_size = ALIGN(size_needed);

  void *p = my_brk(aligned_size);
  if (p == (void *)-1) {
    return NULL;
  } else {
    *(size_t*)p = aligned_size;
    return p + SIZE_T_SIZE;
  }
}

bool in_free_list(list_t *p) {
  list_t *cur = free_list;
  while (cur) {
    if (cur == p) {
      return true;
    }
    cur = cur->next;
  }
  return false;
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
bool coalesce_back(list_t *q_node) {
  void *p, *q, *p_e;
  size_t size_p, size_q;
  list_t *p_node;

  q = (void *)q_node;
  p_e = q - SIZE_T_SIZE;
  if (p_e < mem_heap_lo()) {
    return false;
  }
  size_p = *(size_t *)p_e;
  p = p_e - size_p + SIZE_T_SIZE;
  if (p < mem_heap_lo() || p + LIST_T_SIZE > mem_heap_hi() + 1) {
    return false;
  }

  p_node = (list_t *)p;
  q_node = (list_t *)q;
  if (p_node->size != size_p) {
    return false;
  }
  if (!in_free_list(p_node)) {
    return false;
  }


  // finally can safely coalesce
  size_q = q_node->size;
  p_node->size += size_q;
  mark_end(p_node);
  return true;
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
bool coalesce_forward(list_t *p_node) {
  void *p, *q, *q_e;
  size_t size_p, size_q, size_q2;
  list_t *q_node;

  p = (void *)p_node;
  size_p = p_node->size;
  q = p + size_p;
  if (q + LIST_T_SIZE > mem_heap_hi() + 1) {
    return false;
  }
  q_node = (list_t *)q;
  size_q = q_node->size;
  q_e = q + size_q;

  if (q_e < mem_heap_lo() || q_e > mem_heap_hi() + 1) {
    return false;
  }
  size_q2 = *(size_t *)(q_e - SIZE_T_SIZE);
  if (size_q != size_q2) {
    return false;
  }
  if (!in_free_list(q_node)) {
    return false;
  }

  // finally can safely coalesce
  p_node->size += size_q;
  list_erase(&free_list, q_node);
  mark_end(p_node);
  return true;
}

// free - Freeing a block does nothing.
void my_free(void *p) {
  p -= SIZE_T_SIZE;
  size_t size = *(size_t *)p;

  list_t *p_node = (list_t *)(p);
  D(p_node->id = nxt_id());
  dprintf("my_free(%zu)\n", p_node->id);
  p_node->size = size;

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
  list_append(&free_list, p_node);
}

void * my_realloc(void *ptr, size_t size) {
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
