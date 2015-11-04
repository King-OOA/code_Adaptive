#ifndef BST_H
#define BST_H
#include <stdio.h>


struct BST;
typedef struct BST BST_t;
typedef int node_t;

int BST_insert(BST_t *bst, node_t value);
void BST_pre_order_traverse(BST_t *bst, size_t current_node, void (*callback)(node_t value));
BST_t *BST_create(size_t capacity);
node_t *BST_find(BST_t *bst, node_t value);
void BST_test(void);
void print_node(node_t value);


#endif 
