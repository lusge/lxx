#ifndef LXX_CONFIG_H
#define LXX_CONFIG_H


typedef struct {
    HashTable   *config;
    zend_object std;
} lxx_config_t;

void lxx_config_instance(zval *this_ptr);

LXX_MINIT_FUNCTION(config);
#endif