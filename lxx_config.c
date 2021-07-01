#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "php.h"
#include "ext/standard/info.h"
#include "ext/standard/php_var.h"

#include "php_lxx.h"
#include "lxx_config.h"

zend_class_entry *lxx_config_ce;
static zend_object_handlers lxx_config_handlers;


static zend_object *lxx_config_new(zend_class_entry *ce) {
    lxx_config_t *app = emalloc(sizeof(lxx_config_t) + zend_object_properties_size(ce));
    memset(app, 0, XtOffsetOf(lxx_config_t, std));


    zend_object_std_init(&app->std, ce);
    app->std.handlers = &lxx_config_handlers;

    return &app->std;
}

static lxx_config_t *lxx_config_fetch(zend_object *object) {
    return (lxx_config_t *) ((char *) object - lxx_config_handlers.offset);
}

static void lxx_config_free(zend_object *object) {
    lxx_config_t *app = lxx_config_fetch(object);

    zend_object_std_dtor(object);
}

void lxx_config_instance(zval *this_ptr) {
    object_init_ex(this_ptr, lxx_config_ce);
}

zend_function_entry lxx_config_methods[] = {

    ZEND_FE_END
};

LXX_MINIT_FUNCTION(config) {
    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, "Lxx\\Config", lxx_config_methods);
    lxx_config_ce = zend_register_internal_class(&ce);
    lxx_config_ce->ce_flags |= ZEND_ACC_FINAL;
    lxx_config_ce->create_object = lxx_config_new;

    memcpy(&lxx_config_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    lxx_config_handlers.offset = XtOffsetOf(lxx_config_t, std);
    lxx_config_handlers.free_obj = lxx_config_free;

    return SUCCESS;
}