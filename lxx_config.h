#ifndef LXX_CONFIG_H
#define LXX_CONFIG_H

#define LXX_CONFIG_DIR      "/conf/config.php"

typedef struct {
    HashTable   *config;
    zend_object std;
} lxx_config_t;

void lxx_config_instance(zval *this_ptr);

zval *lxx_config_get_key_value(zend_object *object, const char *key);

LXX_MINIT_FUNCTION(config);
#endif