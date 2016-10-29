# test case that simulates a behavior of a recursive algorithm
# that has some BRANCHING_FACTOR and OBJS_ON_FRAME allocations
# on every step of recursion. Objects are only freed when the
# frame responsible for allocation is destroyed.
# no reallocs.

import random

# sizes of different object allocations in bytes
sizes = [4, 8, 48, 56, 98, 156, 454, 500]
# priorities a specific size will be chosen, more is better
prior = [30, 45, 60, 60, 60, 20, 10, 5]

BRANCHING_FACTOR = 3
OBJS_ON_FRAME = 7
MAX_DEPTH = 10

all_prior = sum(prior)
def rand_size():
    r = random.randint(0, all_prior - 1)
    for ind, i in enumerate(prior):
        r = r - i
        if r < 0:
            return sizes[ind]
    return sizes[-1]


actions = []
obj_counter = 0

def alloc():
    global obj_counter
    size = rand_size()
    actions.append(['a', obj_counter, size])
    obj_counter += 1
    return obj_counter - 1

def free(id):
    actions.append(['f', id])

def rec(depth):
    if depth > MAX_DEPTH:
        return
    allocs = [alloc() for i in range(OBJS_ON_FRAME)]
    for i in range(BRANCHING_FACTOR):
        rec(depth + 1)
    for i in allocs:
        free(i)

rec(1)

# suggested heap size
print OBJS_ON_FRAME * BRANCHING_FACTOR ** MAX_DEPTH * 200
# num of ids
print obj_counter
# num of ops
print len(actions)
# weight of test, not used
print 1

for action in actions:
    print ' '.join([str(i) for i in action])
