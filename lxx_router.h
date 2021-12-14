#ifndef LXX_ROUTER_H
#define LXX_ROUTER_H

typedef struct {
    HashTable   *routes;
    HashTable   *paths;
    zend_long   idx;
    zval        radix_tree;
    zend_string *controller;
    zend_string *action;
    zend_object std;
} lxx_router_t;

void lxx_router_instance(zval *this_ptr);

zval *lxx_router_match_router();

void lxx_router_set_controller(zend_object *object, zend_string *controller);

void lxx_router_set_action(zend_object *object, zend_string *action);

zval *lxx_router_get_request(zend_object *object);

LXX_MINIT_FUNCTION(router);

#endif