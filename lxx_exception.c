#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "php.h"
#include "ext/standard/info.h"
#include "ext/standard/php_var.h"
#include "Zend/zend_exceptions.h"

#include "php_lxx.h"
#include "lxx_exception.h"

zend_class_entry *lxx_exception_ce;

void lxx_throw_exception(int type, const char *format, ...) {
    va_list args;
	char *message;

	va_start(args, format);
	vspprintf(&message, 0, format, args);
	va_end(args);
    
    zend_throw_exception(lxx_exception_ce, message, type);
}

LXX_MINIT_FUNCTION(exception) {
    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, "Lxx\\Exception", NULL);
    lxx_exception_ce = zend_register_internal_class_ex(&ce, zend_ce_exception);
    lxx_exception_ce->ce_flags |= ZEND_ACC_FINAL;

    return SUCCESS;
}
