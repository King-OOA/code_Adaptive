#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "BST.h"
#include "common.h"

struct BST {
    size_t capacity;
    node_t *tree;
};

static int left_child(int current_node)
{
    return 2 * current_node;
}

static int right_child(int current_node)
{
    return 2 * current_node + 1;
}

BST_t *BST_create(size_t capacity)
{
    BST_t *new_bst = MALLOC(1, BST_t);
    new_bst->tree = MALLOC(capacity, node_t);
    new_bst->capacity = capacity;
    
    return new_bst;
}

int BST_insert(BST_t *bst, node_t value)
{
    size_t current_node = 1;
    
    if (value == 0)
        fprintf(stderr, "error: value can not be zero!\n");
    
    while (current_node < bst->capacity && bst->tree[current_node]) {
        if (value < bst->tree[current_node])
            current_node = left_child(current_node);
        else if (value > bst->tree[current_node])
            current_node = right_child(current_node);
        else 
            return 0;
    }
    
    if (current_node < bst->capacity) {
        bst->tree[current_node] = value;
        return 1;
    } else
        return 0;
}

node_t *BST_find(BST_t *bst, node_t value)
{
    size_t current_node = 1;
    
    while (current_node < bst->capacity && bst->tree[current_node])
        if (value < bst->tree[current_node])
            current_node = left_child(current_node);
        else if (value > bst->tree[current_node])
            current_node = right_child(current_node);
        else 
            return bst->tree + current_node;
        
    return NULL;
}

void BST_pre_order_traverse(BST_t *bst, size_t current_node, void (*callback)(node_t value))
{
    if (current_node < bst->capacity && bst->tree[current_node]) {
        callback(bst->tree[current_node]);
        BST_pre_order_traverse(bst, left_child(current_node), callback);
        BST_pre_order_traverse(bst, right_child(current_node), callback);
    }
}

void BST_test(void)
{

    int i = 10;
    BST_t *new_bst = BST_create(1000);

    srand(time(NULL));
    
    for (i = 1; i <= 10; i++)
        BST_insert(new_bst, rand()%10000);

}

void print_node(node_t value)
{
    printf("%d\n", value);
}


/* int main(void) */
/* { */
/*     BST_test(); */
/* } */
