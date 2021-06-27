#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "php.h"
#include "ext/standard/info.h"
#include "ext/standard/php_var.h"

#include "php_lxx.h"
#include "lxx_response.h"

zend_class_entry *lxx_response_ce;
static zend_object_handlers lxx_response_handlers;


static zend_object *lxx_response_new(zend_class_entry *ce) {
    lxx_response_t *app = emalloc(sizeof(lxx_response_t) + zend_object_properties_size(ce));
    memset(app, 0, XtOffsetOf(lxx_response_t, std));


    zend_object_std_init(&app->std, ce);
    app->std.handlers = &lxx_response_handlers;

    return &app->std;
}

static lxx_response_t *lxx_response_fetch(zend_object *object) {
    return (lxx_response_t *) ((char *) object - lxx_response_handlers.offset);
}

static void lxx_response_free(zend_object *object) {
    lxx_response_t *app = lxx_response_fetch(object);

    zend_object_std_dtor(object);
}

zend_function_entry lxx_response_methods[] = {

    ZEND_FE_END
};

LXX_MINIT_FUNCTION(response) {
    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, "Lxx\\Response", lxx_response_methods);
    lxx_response_ce = zend_register_internal_class(&ce);
    lxx_response_ce->ce_flags |= ZEND_ACC_FINAL;
    lxx_response_ce->create_object = lxx_response_new;

    memcpy(&lxx_response_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    lxx_response_handlers.offset = XtOffsetOf(lxx_response_t, std);
    lxx_response_handlers.free_obj = lxx_response_free;

    return SUCCESS;
}