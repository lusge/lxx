#ifndef LXX_ROUTER_H
#define LXX_ROUTER_H

typedef struct {
    HashTable   *routes;
    zend_long   idx;
    zval        radix_tree;
    zval        request;
    zend_object std;
} lxx_router_t;

LXX_MINIT_FUNCTION(router);

#endif