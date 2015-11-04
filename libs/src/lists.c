#include <stdio.h>
#include "lists.h"
#include <stdlib.h>

struct single_list_node {
    list_t value;
    struct single_list_node *next;
};

struct single_list {
    size_t num_of_nodes;
    single_list_node_t *head;
};

single_list_t *single_list_create(void)
{
    single_list_t *new_list;

    new_list = malloc(sizeof(single_list_t));
    new_list->num_of_nodes = 0;
    new_list->head = NULL;
  
    return new_list;
}

void single_list_insert(single_list_t *list, list_t value)
{
    register single_list_node_t **current_node_p = &list->head;
    register single_list_node_t *current_node, *new_node;
  
    for (current_node_p = &list->head;
         (current_node = *current_node_p) != NULL && current_node->value < value;
         current_node_p = &current_node->next)
        ;
    
    new_node = malloc(sizeof(single_list_node_t));
    new_node->value = value;
    new_node->next = current_node;
    *current_node_p = new_node;
  
    list->num_of_nodes++;
}

void single_list_delete(single_list_t *list, list_t value)
{
    single_list_node_t **current_node_p, *current_node;

    for (current_node_p = &list->head;
         (current_node = *current_node_p) != NULL && current_node->value != value;
         current_node_p = &current_node->next)
        ;

    if (current_node != NULL) {
        *current_node_p = current_node->next;
        free(current_node);
    }
}

single_list_node_t *single_list_find(single_list_t *list, list_t value)
{
    register single_list_node_t *current_node;

    for (current_node = list->head;
         current_node != NULL && current_node->value < value;
         current_node = current_node->next)
        ;

    return current_node;
}

void single_list_clear(single_list_t *list)
{
    single_list_node_t *node_to_delete;

    while (list->head != NULL) {
        node_to_delete = list->head;
        list->head = list->head->next;
        free(node_to_delete);
    }
}

void single_list_destroy(single_list_t *list)
{
    single_list_clear(list);
    free(list);
}

void single_list_print(single_list_t *list)
{
    single_list_node_t *current_node;

    for (current_node = list->head; current_node != NULL; current_node = current_node->next)
        printf("%d ", current_node->value);

    putchar('\n');
}

void list_test(void)
{
    int value;
    single_list_t *list;
    
    list = single_list_create();
    
    scanf("%d", &value);
    while (value) {
        single_list_insert(list, value);
        scanf("%d", &value);
    }

    single_list_print(list);

    scanf("%d", &value);
    while (value) {
        printf("%d\n", (single_list_find(list, value))->value);
        scanf("%d", &value);
    }
    
    scanf("%d", &value);
    while (value) {
        single_list_delete(list, value);
        scanf("%d", &value);
    }
        
    single_list_print(list);

    single_list_clear(list);
    single_list_print(list);

    single_list_destroy(list);
}

