#ifndef LXX_CONTROLLER_H
#define LXX_CONTROLLER_H

typedef struct {
    zend_object std;
} lxx_controller_t;

extern zend_class_entry *lxx_controller_ce;

LXX_MINIT_FUNCTION(controller);

#endif