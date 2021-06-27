#ifndef LXX_LOADER_H
#define LXX_LOADER_H

typedef struct {

    zend_object std;
} lxx_loader_t;

LXX_MINIT_FUNCTION(loader);
#endif