#include <stdlib.h>
#include "./utilfast.h"

void list_print(list_t **root) {
  list_t *cur = *root;
  dprintf("[");
  if (cur) {
    dprintf("%zu", cur->id);
    cur = cur->next;
  } else {
    dprintf("_");
  }

  int cnt = 10;
  while (cur && cnt > 0) {
    cnt--;
    dprintf("-->%zu (%p)", cur->id, cur);
    cur = cur->next;
  }
  if (cnt == 0) {
    dprintf("-->...");
  }
  dprintf("]\n");
}

void list_erase(list_t **root, list_t *node) {
  if (node->prev) {
    node->prev->next = node->next;
  } else {
    *root = node->next;
  }
  if (node->next) {
    node->next->prev = node->prev;
  }
}

D(
size_t id_cnt = 0;
int nxt_id() {
  return id_cnt++;
})

void list_add(list_t **root, list_t *node) {
  node->prev = NULL;
  node->next = *root;
  if (*root) {
    (*root)->prev = node;
  }
  *root = node;
}


/*
 * update the last SIZE_T_SIZE bytes of p's free block to hold the size ^ SECRET
 * requires that p is at least LIST_T_SIZE + SIZE_T_SIZE wide.
 */

void mark_end(list_t *p) {
  *(size_t *)((void *)p + p->size - SIZE_T_SIZE) = p->size ^ SECRET;
}

void unmark_end(list_t *p) {
  *(size_t *)((void *)p + p->size - SIZE_T_SIZE) = 0;
}

list_t* mknode(void *p, size_t size) {
  list_t *node = (list_t *)p;
  D(node->id = nxt_id());
  node->size = size;
  mark_end(node);
  dprintf("created n%zu (%zu)\n", node->id, node->size);
  return p;
}

/*
 * unmark the current node
 * return the list_t * ret, such that:
 *  ret starts at (void *)node + size
 *  ret->size == node->size - size
 */
list_t * slice(list_t *node, size_t size) {
  dprintf("slice n%zu (%zu)\n", node->id, node->size);
  unmark_end(node);
  if (node->size >= size + LIST_T_SIZE + SIZE_T_SIZE) {
    list_t *rem = (list_t *)((void *)node + size);
    D(rem->id = node->id);
    rem->size = node->size - size;
    mark_end(rem);
    dprintf("ret n%zu (%zu)\n", rem->id, rem->size);
    return rem;
  } else {
    dprintf("del n%zu (0)\n", node->id);
    return NULL;
  }
}

/*
 * unmark the current node
 * return the list_t * ret, such that:
 *  ret starts at node
 *  ret->size == node->size + size
 */
void grow(list_t *node, size_t size) {
  dprintf("grow n%zu (%zu)\n", node->id, node->size);
  unmark_end(node);
  node->size += size;
  mark_end(node);
  dprintf("ret n%zu (%zu)\n", node->id, node->size);
}
