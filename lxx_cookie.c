#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "php.h"
#include "ext/standard/info.h"
#include "ext/standard/php_var.h"

#include "php_lxx.h"
#include "lxx_cookie.h"

zend_class_entry *lxx_cookie_ce;

zend_function_entry lxx_cookie_methods[] = {

    ZEND_FE_END
};

LXX_MINIT_FUNCTION(cookie) {

    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, "Lxx\\Cookie", lxx_cookie_methods);
    lxx_cookie_ce = zend_register_internal_class_ex(&ce, NULL);
    lxx_cookie_ce->ce_flags |= ZEND_ACC_FINAL;


    return SUCCESS;
}