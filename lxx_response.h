#ifndef LXX_RESPONSE_H
#define LXX_RESPONSE_H

typedef struct {
    HashTable       *header;
    unsigned int    code;
    zend_object     std;
} lxx_response_t;

void lxx_response_set_hader(zend_object *object, zend_string *key, zend_string *value);
void lxx_response_send(zend_object *object, char *body, size_t len);

void lxx_response_instance(zval *this_ptr);

LXX_MINIT_FUNCTION(response);

#endif