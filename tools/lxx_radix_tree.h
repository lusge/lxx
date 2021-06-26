#ifndef LXX_RADIX_TREE_H
#define LXX_RADIX_TREE_H


#include "rax/rax.h"

typedef struct {
    raxIterator it;
    rax         *radix;
    zend_object std;
} rax_tree_t;

void lxx_rax_tree_instance(zval *rax_tree);

void lxx_rax_tree_show(zend_object *object);
zend_long lxx_rax_tree_find(zend_object *object, zend_string *key);
int lxx_rax_tree_insert(zend_object *object, zend_string *key, zend_long val);
int lxx_rax_tree_seek(zend_object *object, zend_string *op, zend_string *ele);
zend_long lxx_rax_tree_pre(zend_object *object);
zend_long lxx_rax_tree_next(zend_object *object);

zend_long lxx_rax_tree_pre_seach(zend_object *object, const char *op, zend_string *key);
zend_long lxx_rax_tree_nex_seach(zend_object *object, const char *op, zend_string *key);


LXX_MINIT_FUNCTION(raxTree);

#endif