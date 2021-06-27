#ifndef LXX_APPLICATION_H
#define LXX_APPLICATION_H

typedef struct {
    zval        *router;
    zend_object std;
} lxx_application_t;

LXX_MINIT_FUNCTION(application);

#endif