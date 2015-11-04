#ifndef  LISTS_H
#define  LISTS_H

typedef int list_t;
struct single_list;
struct single_list_node;
typedef struct single_list single_list_t;
typedef struct single_list_node single_list_node_t;

void list_test(void);
void single_list_print(single_list_t *list);
void single_list_destroy(single_list_t *list);
void single_list_clear(single_list_t *list);
void single_list_delete(single_list_t *list, list_t value);
void single_list_insert(single_list_t *list, list_t value);
single_list_node_t *single_list_find(single_list_t *list, list_t value);
single_list_t *single_list_create(void);

#endif 
