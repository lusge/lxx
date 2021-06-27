#ifndef LXX_CONFIG_H
#define LXX_CONFIG_H


typedef struct {
    zend_object std;
} lxx_config_t;

LXX_MINIT_FUNCTION(config);
#endif