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

  dprintf("\n");
  dprintf("erasing %zu\n", node->id);
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

  D(list_print(root));
}

D(size_t id = 0);
void list_append(list_t **root, list_t *node) {
  D(node->id = id++);
  dprintf("\n");
  dprintf("appending %zu\n", node->id);
  node->next = *root;
  *root = node;
  D(list_print(root));
}

