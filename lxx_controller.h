#ifndef LXX_CONTROLLER_H
#define LXX_CONTROLLER_H

typedef struct {

    zend_object std;
} lxx_controller_t;

LXX_MINIT_FUNCTION(controller);

#endif