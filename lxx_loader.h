#ifndef LXX_LOADER_H
#define LXX_LOADER_H

void lxx_loader_instance();

int lxx_loader_include(zend_string *filename, zval *retval);

LXX_MINIT_FUNCTION(loader);

#endif