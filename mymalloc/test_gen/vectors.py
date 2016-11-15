# a test case that simulates a behavior of the program that manages a lot of
# vectors that expand in size over time.
# sometimes it allocates objects, but mostly expands vectors with realloc

# alloc some vectors
# push a lot of things
# free some things from the back
# push a lot of things
# pop and free everything from the back

import random

VECTOR_ELEMENT_SIZE = 8
STARTING_VECTOR_SIZE = 10
NUMBER_OF_VECTORS = 15
VECTOR_EXPAND_CONST = 1.61803398875 # golden ratio like GNU STL
ITERATIONS = 3000

# sizes of different object allocations in bytes
sizes = [4, 8, 48, 56, 98, 156, 454, 500]
# priorities a specific size will be chosen, more is better
prior = [30, 45, 60, 60, 60, 20, 10, 5]

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

def alloc(alloc_size=None):
    global obj_counter
    size = alloc_size
    if alloc_size is None:
        size = rand_size()
    actions.append(['a', obj_counter, size])
    obj_counter += 1
    return obj_counter - 1

def free(id):
    actions.append(['f', id])

def realloc(id, size):
    actions.append(['r', id, size])

vectors = [alloc(VECTOR_ELEMENT_SIZE * STARTING_VECTOR_SIZE) for i in range(NUMBER_OF_VECTORS)]
vector_size = [STARTING_VECTOR_SIZE for i in range(NUMBER_OF_VECTORS)]
vector_els = [[] for i in range(NUMBER_OF_VECTORS)]

def choose_vector():
    return random.randint(0, NUMBER_OF_VECTORS - 1)

if __name__ == '__main__':
    for iter in range(ITERATIONS):
        obj = alloc()
        vec = choose_vector()
        if len(vector_els[vec]) == vector_size[vec]:
                vector_size[vec] = int(vector_size[vec] * VECTOR_EXPAND_CONST)
                realloc(vectors[vec], vector_size[vec] * VECTOR_ELEMENT_SIZE)
        vector_els[vec].append(obj)

    for iter in range(ITERATIONS // 12):
        vec = choose_vector()
        obj = vector_els[vec].pop()
        free(obj)
        if len(vector_els[vec]) <= vector_size[vec] // VECTOR_EXPAND_CONST:
            vector_size[vec] = int(vector_size[vec] // VECTOR_EXPAND_CONST)
            realloc(vectors[vec], vector_size[vec] * VECTOR_ELEMENT_SIZE)

    for iter in range(ITERATIONS // 55):
        obj = alloc()
        vec = choose_vector()
        if len(vector_els[vec]) == vector_size[vec]:
                vector_size[vec] = int(vector_size[vec] * VECTOR_EXPAND_CONST)
                realloc(vectors[vec], vector_size[vec] * VECTOR_ELEMENT_SIZE)
        vector_els[vec].append(obj)


    # suggested heap size
    print ITERATIONS * 4 * 200
    # num of ids
    print obj_counter
    # num of ops
    print len(actions)
    # weight of test, not used
    print 1

    for action in actions:
        print ' '.join([str(i) for i in action])
