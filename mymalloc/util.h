#ifdef DEBUG
#define D(x) x
#define dprintf printf
#else
#define D(...)
#define dprintf(...)
#endif

typedef struct list_t {
  size_t size;
  D(size_t id;)
  struct list_t* next;
} list_t;

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
    dprintf("-->%zu", cur->id);
    cur = cur->next;
  }
  if (cnt == 0) {
    dprintf("-->...");
  }
  dprintf("]\n");
}

void list_erase(list_t **root, list_t *node) {
  dprintf("\n");
  dprintf("erasing %zu\n", node->id);
  list_t **cur = root; // be careful: node might be the root
  while (*cur) {
    if (*cur == node) {
      *cur = node->next;
      return;
    } else {
      cur = &((*cur)->next);
    }
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

