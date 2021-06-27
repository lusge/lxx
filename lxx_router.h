#ifndef LXX_ROUTER_H
#define LXX_ROUTER_H

typedef struct {
    HashTable   *routes;
    zend_long   idx;
    zval        radix_tree;
    zval        request;
    zend_object std;
} lxx_router_t;

void lxx_router_instance(zval *this_ptr);

zval *lxx_router_match_router(zend_object *object);

LXX_MINIT_FUNCTION(router);

#endif