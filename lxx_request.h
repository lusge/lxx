#ifndef LXX_REQUEST_H
#define LXX_REQUEST_H

typedef struct {
    zend_string *base_uri;
    zend_string *request_uri;
    zend_string *method;
    HashTable   *params; 
    zend_object std;
} lxx_request_t;

void lxx_request_instance(zval *this_ptr);
zval *lxx_request_get_server(char *key, size_t len);
zend_string *lxx_request_get_method(zend_object *object);
zend_string *lxx_request_get_base_uri(zend_object *object);
zend_string *lxx_request_get_request_uri(zend_object *object);

void lxx_request_set_params(zend_object *object, zval *params);

LXX_MINIT_FUNCTION(request);

#endif