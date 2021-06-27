#ifndef LXX_CONTROLLER_H
#define LXX_CONTROLLER_H

typedef struct {
    zval        *router;  /* 来自其他类的引用 */
    zval        *request; /* 来自其他类的引用 */
    zend_object std;
} lxx_controller_t;

extern zend_class_entry *lxx_controller_ce;

LXX_MINIT_FUNCTION(controller);

#endif