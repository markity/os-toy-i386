#include "tools/list.h"

void test() {

}

void list_init(list_t *list) {
    list->frist = list->last = (list_node_t*)0;
    list->count = 0;
}

void list_insert_first(list_t *l, list_node_t *n) {
    n->next = l->frist;
    n->pre = (list_node_t*)0;
    if (l->count == 0) {
        l->frist = l->last = n;
    } else {
        l->frist->pre = n;
        l->frist = n;
    }

    l->count ++;
}

void list_insert_last(list_t *l, list_node_t *n) {
    n->pre = l->last;
    n->next = (list_node_t*)0;
    if(l->count == 0) {
        l->frist = l->last = n;
    } else {
        l->last->next = n;
        l->last = n;
    }
    
    l->count ++;
}

list_node_t *list_pop_first(list_t *l) {
    if(l->count == 0) {
        return (list_node_t*)0;
    }

    list_node_t *remove_node = l->frist;
    l->frist = remove_node->next;

    if (l->frist == (list_node_t*)0) {
        l->last = (list_node_t*)0;
    } else {
        remove_node->next->pre = (list_node_t*)0;
    }

    remove_node->pre = remove_node->next = (list_node_t*)0;
    l->count --;

    return remove_node;
}

list_node_t *list_pop_last(list_t *l) {
    if (l->count == 0) {
        return (list_node_t*)0;
    }

    list_node_t *remove_node = l->last;
    l->last = remove_node->pre;

    // 说明仅有一个node
    if (l->last == (list_node_t*)0) {
       l->frist = (list_node_t*)0;
    // 说明有多个node
    } else {
        remove_node->pre->next = (list_node_t*)0;
    }

    remove_node->pre =remove_node->next = (list_node_t*)0;

    l->count --;

    return remove_node;
}

list_node_t *list_pop(list_t *l, list_node_t *n) {
    if (n == l->frist) {
        l->frist = n->next;
    }

    if (n == l->last) {
        l->last = n->pre;
    }

    if (n->pre) {
        n->pre->next = n->next;
    }

    if (n->next) {
        n->next->pre = n->pre;
    }
    
    n->pre = n->next = (list_node_t*)0;

    l->count--;

    return n;
}
