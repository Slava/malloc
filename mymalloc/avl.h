#ifndef AVL_H
#define AVL_H

/* creates a mask that can be applied each time we need to invert
 * a value depending on the boolean that was provided
 * to new_bool_inv_mask(bool) aka bool?value:-value, this
 * is done to prevent forking. Am i right to do it like so? */
#include <stdbool.h>
__inline__
static signed char
new_bool_inv_mask(bool value) {
		return (signed char) ~( ((int)value) * 0xff );
}

/* applies a mask created with new_bool_inv_mask() */
__inline__
static signed char
apply_mask(register signed char balance, register signed char mask)
{
	return (signed char)
		( ((int)(balance^mask)) + (mask&1) );
}

#endif

#ifndef LOGFILE
#define LOGFILE stderr
#endif

#ifdef LOG
#include <stdio.h>
#endif

#define AVL_NODE struct AVL_NODE_##TYPE

AVL_NODE {
	signed char balance;
	AVL_NODE *left, *right, *parent;
	TYPE data;
};

/* We use this to measure the height of the subtree rooted at c. See:
 *
 * https://upload.wikimedia.org/wikipedia/commons/c/c4/The_Rebalancing.gif
 *
 * We then infer the heights of the children d, b and a, respectively,
 * in order to calculate node balances afterwards, something implemented
 * for the performance benefits of it vs calculating or storing heights. */
#define GET_HEIGHT GET_HEIGHT_##TYPE
#include <stddef.h>
__inline__ static size_t
GET_HEIGHT(register AVL_NODE *curr) {
	register size_t h = 0;

	while (curr) {
		h++;
		curr = *( &curr->left + (curr->balance > 0) );
	}

	return h;
}

/* refer to:
 * https://upload.wikimedia.org/wikipedia/commons/c/c4/The_Rebalancing.gif */
#define REBALANCE REBALANCE_##TYPE
static AVL_NODE* REBALANCE(AVL_NODE *n, bool n_rheavy, signed char n_balance) {
	AVL_NODE **ns_heaviest_ref = &n->left + n_rheavy,
					 *f = *ns_heaviest_ref;

	signed char f_balance = f->balance;

	/* FIXME !f_balance is here just for some delete situations.. how to optimize? */
	bool same_dir = n_balance == (f_balance<<1) || !f_balance,
			 /* is f's tallest subtree to the right? */
			 f_rheavy = same_dir == n_rheavy;

	AVL_NODE **fs_left_child_ref = &f->left,
					 **fs_heaviest_ref = fs_left_child_ref + f_rheavy,
					 *l = *fs_heaviest_ref,
					 **ls_left_child_ref = &l->left,
					 **ls_c_ref = ls_left_child_ref + !f_rheavy,
					 *c = *ls_c_ref,
					 **ns_parent_ref = &n->parent,
					 *ns_parent = *ns_parent_ref;

	signed char imask_n_rheavy = new_bool_inv_mask(n_rheavy),
							imask_f_rheavy = new_bool_inv_mask(f_rheavy),
							l_balance = l->balance;

	size_t c_height = GET_HEIGHT(c),
				 d_height = ( size_t ) ( (long long int)c_height +
						 apply_mask(l_balance, imask_f_rheavy) ),
				 l_height = (c_height>d_height?c_height:d_height) + 1,
				 b_height = ( size_t ) ( (long long int)l_height -
						 apply_mask(f_balance, imask_f_rheavy) ),
				 a_height = ( size_t ) ( (long long int)l_height + 1 -
						 apply_mask(n_balance, imask_n_rheavy) );
	/* fprintf(LOGFILE, "{dh:%zu, lh:%zu, bh:%zu, ah:%zu}", */
	/* 		d_height, l_height, b_height, a_height); */

#ifdef LOG
	fputs(n_rheavy?">":"<", LOGFILE);
	fputs((same_dir == n_rheavy)?">":"<", LOGFILE);
#endif

	if (same_dir) { /* one rotation */
		n->balance = apply_mask((signed char)
				b_height - a_height, imask_n_rheavy);
		f->balance = apply_mask((signed char) /* FIXME problem is here */
				l_height - ((b_height>a_height?b_height:a_height) + 1),
				imask_n_rheavy);
		/* fprintf(LOGFILE, "{nb:%d, fb:%d}", n->balance, f->balance); */

		*ns_parent_ref = f;
		{
			AVL_NODE **fs_lightest_ref = fs_left_child_ref + !f_rheavy,
							 *b = *fs_lightest_ref;

			if (b) b->parent = n;
			f->parent = ns_parent;
			if (ns_parent) *(&ns_parent->left + (n == ns_parent->right)) = f;

			*fs_lightest_ref = n;
			*ns_heaviest_ref = b;
		}

		return f;
	}

	/* two */
	n->balance = apply_mask((signed char) 
			(d_height - a_height), imask_n_rheavy);
	f->balance = apply_mask((signed char)
			(b_height - c_height), imask_n_rheavy);
	l->balance = apply_mask((signed char)
			( (b_height>c_height?b_height:c_height) -
				(d_height>a_height?d_height:a_height)
			), imask_n_rheavy);

	*ns_parent_ref = l;
	f->parent = l;

	{
		register AVL_NODE **ls_d_ref = ls_left_child_ref + f_rheavy,
						 *d = *ls_d_ref;

		if (d) d->parent = n;
		if (c) c->parent = f;
		l->parent = ns_parent;
		if (ns_parent) *(&ns_parent->left + (n == ns_parent->right)) = l;

		*fs_heaviest_ref = c;
		*ls_c_ref = f;
		*ns_heaviest_ref = d;
		*ls_d_ref = n;
	}
	return l;
}

#include <stdlib.h>
#define AVL_INSERT AVL_INSERT_##TYPE
AVL_NODE * AVL_INSERT(AVL_NODE *root, TYPE data) {
	AVL_NODE *l = (AVL_NODE*) malloc(sizeof(AVL_NODE));
	l->left = l->right = NULL;
	l->balance = 0;
	l->data = data;

	if (root) {
		register AVL_NODE **n_r = &root, *n;
		register int side;
		register bool bside;

		do {
			n = *n_r;
			side = CMP(n->data, data);
			bside = side>0;
			n_r = &n->left + bside;
#ifdef LOG
			fputs(bside?"r":"l", LOGFILE);
#endif
		} while (*n_r);

#ifdef LOG
	LOG(data);
#endif

		l->parent = n;
		*n_r = l;

		{
			register AVL_NODE *p;

avl_insert_go_up:
			{
				signed char *n_balance_r = &n->balance,
										n_balance = (*n_balance_r + (bside<<1) - 1);

				if (n_balance) {
					if ((n_balance&3) == 2) {
						n = REBALANCE(n, bside, n_balance);
					} else {
						*n_balance_r = n_balance;
						if ((p = n->parent)) {
							bside = n == p->right;
							n = p;
							goto avl_insert_go_up;
						}
						return root;
					}
				} else *n_balance_r = 0;
			}

			if (!(p = n->parent)) return n;
			return root;
		}
	}

#ifdef LOG
	LOG(data);
#endif

	l->parent = NULL;
	return l;
}

#define AVL_FIND AVL_FIND_##TYPE
AVL_NODE *AVL_FIND(AVL_NODE *root, TYPE data) {
	if (root) {
    register AVL_NODE **parent_ref = &root, *parent, *last_left_parent = NULL;
		register int side;

		do {
			parent = *parent_ref;
			side = CMP(parent->data, data);
			if (!side) return parent;
      parent_ref = &parent->left + (side>0);
      last_left_parent = (AVL_NODE *)((size_t)last_left_parent*(side<0) + (size_t)parent*(side>0));
    } while (*parent_ref);

    return last_left_parent;
	}

	return NULL;
}

#define REBALANCE_DEL REBALANCE_DEL_##TYPE
static AVL_NODE *
REBALANCE_DEL(AVL_NODE *root, register AVL_NODE *n, register bool lside) {
	do {
		signed char *n_balance_r = &n->balance;
		register signed char n_balance = *n_balance_r + 1 - (lside<<1);
		register AVL_NODE *p;

		if ((n_balance&3) == 2)
			n = REBALANCE(n, !lside, n_balance);
		else
			*n_balance_r = n_balance;

		p = n->parent;
		if (!p) return n;
		if (n->balance) return root;

		lside = n == p->right;
		n = p;
	} while (true);
}

#define _AVL_REMOVE _AVL_REMOVE_##TYPE
static AVL_NODE * _AVL_REMOVE(AVL_NODE* root, AVL_NODE* n) {

#ifdef LOG
  fputs("x", LOGFILE);
	LOG(n->data);
#endif

  if (n->left) {
    AVL_NODE *l = n->left, *nl = l, **lp_r, *lp;
    bool lside;

    while (l->right) l = l->right;

		lp_r = &l->parent;
    lp = *lp_r;
    lside = l == lp->right;
    l->balance = n->balance;

		{
			register AVL_NODE *np = n->parent;
			if ((*lp_r = np))
				*(&np->left + (n == np->right)) = l;
		}

    if (lp != n) {

			{
				register AVL_NODE *ll = l->left;
				if ((lp->right = ll))
					ll->parent = lp;
			}

			l->left = nl;
			nl->parent = l;
			{
				register AVL_NODE *nr = n->right;
				if ((l->right = nr))
					nr->parent = l;
			}

      return REBALANCE_DEL(n == root ? l : root, lp, lside);
    } else {
			register AVL_NODE *nr = n->right;
			l->right = nr;
			if (nr) nr->parent = l;
			return REBALANCE_DEL(n == root ? l : root, l, l == nr);
		}
  } else {
    AVL_NODE *p = n->parent, *nr = n->right;

    if (nr) nr->parent = p;

    if (p) {
      bool side = n == p->right;

			*(&p->left + side) = nr;
      return REBALANCE_DEL(root, p, side);
    }

    return nr;
  }
}

#define AVL_REMOVE AVL_REMOVE_##TYPE
AVL_NODE *AVL_REMOVE(AVL_NODE *root, AVL_NODE *n) {
	AVL_NODE *new_root = _AVL_REMOVE(root, n);
  free(n);
	return new_root;
}
