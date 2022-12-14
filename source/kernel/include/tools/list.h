#ifndef LIST_H
#define LIST_H

// 提供一个双向链表, 支持头部插入和尾部插入, 头部删除, 尾部删除, 任意位置删除

typedef struct _list_node_t {
    struct _list_node_t *pre;
    struct _list_node_t *next;
} list_node_t;

static inline void list_node_init(list_node_t *node) {
    node->pre = node->next = (list_node_t*)0;
}

static inline list_node_t *list_node_pre(list_node_t *node) {
    return node->pre;
}

static inline list_node_t *list_node_next(list_node_t *node) {
    return node->next;
}

typedef struct _list_t{
    list_node_t *frist;
    list_node_t *last;
    int count;
} list_t;

extern void list_init(list_t *);

static inline int list_is_empty(list_t *l) {
    return l->count == 0;
}

static inline int list_count(list_t *l) {
    return l->count;
}

static inline list_node_t *list_first(list_t *l) {
    return l->frist;
}

static inline list_node_t *list_last(list_t *l) {
    return l->last;
}

extern void list_insert_first(list_t *l, list_node_t *n);

extern void list_insert_last(list_t *l, list_node_t *n);

extern list_node_t *list_pop_first(list_t *l);
extern list_node_t *list_pop_last(list_t *l);

extern list_node_t *list_pop(list_t *l, list_node_t *n);

// 找到一个结构体某字段的偏移量
#define OFFSET_OF(type, name) \
    ((uint32_t)(&((type*)0)->name))


// 找到一个NODE
#define CONTAINER_OF(node_ptr, parent_type, name) \
    (parent_type*)((uint32_t)node_ptr - OFFSET_OF(parent_type, name))

#endif