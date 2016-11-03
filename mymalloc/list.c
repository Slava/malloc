#include <stdlib.h>
#include "./util.h"

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
  list_t *p = NULL;
  list_t **prevpp = root;

  // Iterate the linked list until you find the range with a matching lo
  // payload and remove it.  Remember to properly handle the case where the
  // payload is in the first node, and to free the node after unlinking it.
  p = *prevpp;
  while (p) {
    if (p == node) {
      *prevpp = p->next;
      break;
    }
    prevpp = &p->next;
    p = p->next;
  }
}

D(
  size_t id_cnt = 0;
  int nxt_id() {
    return id_cnt++;
  })

void list_append(list_t **root, list_t *node) {
  node->next = *root;
  *root = node;
}
